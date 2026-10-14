// CP3a: V5-style 4x4 outer-product kernel, using 256-bit doubles.
// SIMD lanes hold the SAME column of FOUR different input rows.
// Adds B=48 output tiling, K=256 original-column cache slices,
// and two independent accumulation streams in the inner k loop.
// Based on https://ppc.cs.aalto.fi/ch2/v5/ and the cache-slicing idea in ch2/v7/.
// For GCC/Clang; compile with -std=c++20 -O3 -march=native -fopenmp.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

typedef double double4_t
    __attribute__((vector_size(4 * sizeof(double))));


static inline double4_t swap1(double4_t x) {
    // [x1, x0, x3, x2]. The compiler lowers this to a vector shuffle.
    double4_t y = {x[1], x[0], x[3], x[2]};
    return y;
}

static inline double4_t swap2(double4_t x) {
    // [x2, x3, x0, x1]. The compiler lowers this to a vector shuffle.
    double4_t y = {x[2], x[3], x[0], x[1]};
    return y;
}

static void prepare_columns(
    int ny, int nx, const float* data,
    std::vector<double4_t>& packed
) {
    const int row_blocks = (ny + 3) / 4;
    #pragma omp parallel for schedule(static)
    for (int g = 0; g < row_blocks; ++g) {
        double4_t* block = packed.data() + std::size_t(g) * nx;

        // block[k][lane] represents input row 4*g+lane, column k.
        for (int k = 0; k < nx; ++k) {
            double4_t v = {};
            for (int lane = 0; lane < 4; ++lane) {
                const int y = 4 * g + lane;
                if (y < ny) {
                    v[lane] = static_cast<double>(data[std::size_t(y) * nx + k]);
                }
            }
            block[k] = v;
        }

        // Each lane accumulates statistics for a DIFFERENT row.
        double4_t s0 = {}, s1 = {}, s2 = {}, s3 = {};
        int k = 0;
        for (; k + 3 < nx; k += 4) {
            s0 += block[k];
            s1 += block[k + 1];
            s2 += block[k + 2];
            s3 += block[k + 3];
        }
        for (; k < nx; ++k) s0 += block[k];
        const double4_t means = ((s0 + s1) + (s2 + s3)) / double(nx);

        double4_t q0 = {}, q1 = {}, q2 = {}, q3 = {};
        k = 0;
        for (; k + 3 < nx; k += 4) {
            const double4_t v0 = block[k]     - means;
            const double4_t v1 = block[k + 1] - means;
            const double4_t v2 = block[k + 2] - means;
            const double4_t v3 = block[k + 3] - means;
            block[k] = v0;
            block[k + 1] = v1;
            block[k + 2] = v2;
            block[k + 3] = v3;
            q0 += v0 * v0;
            q1 += v1 * v1;
            q2 += v2 * v2;
            q3 += v3 * v3;
        }
        for (; k < nx; ++k) {
            const double4_t v = block[k] - means;
            block[k] = v;
            q0 += v * v;
        }
        const double4_t norm2 = (q0 + q1) + (q2 + q3);
        double4_t scale = {};
        for (int lane = 0; lane < 4; ++lane) {
            if (norm2[lane] > 0.0) {
                scale[lane] = 1.0 / std::sqrt(norm2[lane]);
            }
        }
        for (int k2 = 0; k2 < nx; ++k2) {
            block[k2] *= scale;
        }
    }
}

static void matrix_mult(
    int ny, int nx,
    const std::vector<double4_t>& packed, float* result
) {
    constexpr int B = 48;  // Output tile: at most B x B correlations.
    constexpr int K = 256; // ORIGINAL input columns per cache slice.
    constexpr int G = B / 4; // Four rows per packed group.
    static_assert(B > 0 && B % 4 == 0 && K > 0);

    // ib and jb are original-row indices, not packed-group indices.
    std::vector<std::pair<int, int>> tiles;
    for (int ib = 0; ib < ny; ib += B) {
        for (int jb = 0; jb <= ib; jb += B) {
            tiles.emplace_back(ib, jb);
        }
    }
    const int tile_count = static_cast<int>(tiles.size());

    #pragma omp parallel for schedule(dynamic, 1)
    for (int t = 0; t < tile_count; ++t) {
        const int ib = tiles[t].first;
        const int jb = tiles[t].second;
        const int ni = (std::min(B, ny - ib) + 3) / 4;
        const int nj = (std::min(B, ny - jb) + 3) / 4;
        const bool diagonal = (ib == jb);

        // Private to this tile. Each 4x4 microtile has four vectors.
        // Keep the V5 permutation order until ALL k-slices are done.
        alignas(64) double4_t partial[G][G][4] = {};

        for (int kb = 0; kb < nx; kb += K) {
            const int kend = kb + std::min(K, nx - kb);

            for (int ii = 0; ii < ni; ++ii) {
                const double4_t* a = packed.data()
                    + std::size_t(ib / 4 + ii) * nx;
                const int jlimit = diagonal ? ii + 1 : nj;

                for (int jj = 0; jj < jlimit; ++jj) {
                    const double4_t* b = packed.data()
                        + std::size_t(jb / 4 + jj) * nx;

                    // Continue the sums from the previous slice.
                    double4_t vv00 = partial[ii][jj][0];
                    double4_t vv01 = partial[ii][jj][1];
                    double4_t vv10 = partial[ii][jj][2];
                    double4_t vv11 = partial[ii][jj][3];

                    // Two independent k streams: 8 vector accumulation chains.
                    double4_t w00 = {}, w01 = {}, w10 = {}, w11 = {};
                    int k = kb;
                    for (; k + 1 < kend; k += 2) {
                        const double4_t x0 = a[k];
                        const double4_t y0 = b[k];
                        const double4_t x2 = swap2(x0);
                        const double4_t y1 = swap1(y0);
                        vv00 += x0 * y0;
                        vv01 += x0 * y1;
                        vv10 += x2 * y0;
                        vv11 += x2 * y1;

                        const double4_t u0 = a[k + 1];
                        const double4_t v0 = b[k + 1];
                        const double4_t u2 = swap2(u0);
                        const double4_t v1 = swap1(v0);
                        w00 += u0 * v0;
                        w01 += u0 * v1;
                        w10 += u2 * v0;
                        w11 += u2 * v1;
                    }
                    // Handle an odd-length final slice (or an odd K).
                    for (; k < kend; ++k) {
                        const double4_t x0 = a[k];
                        const double4_t y0 = b[k];
                        const double4_t x2 = swap2(x0);
                        const double4_t y1 = swap1(y0);
                        vv00 += x0 * y0;
                        vv01 += x0 * y1;
                        vv10 += x2 * y0;
                        vv11 += x2 * y1;
                    }
                    vv00 += w00;
                    vv01 += w01;
                    vv10 += w10;
                    vv11 += w11;

                    // Save double-precision vector sums, NOT float results.
                    partial[ii][jj][0] = vv00;
                    partial[ii][jj][1] = vv01;
                    partial[ii][jj][2] = vv10;
                    partial[ii][jj][3] = vv11;
                }
            }
        }

        // Decode V5's permutations only once, after all input columns.
        for (int ii = 0; ii < ni; ++ii) {
            const int jlimit = diagonal ? ii + 1 : nj;
            for (int jj = 0; jj < jlimit; ++jj) {
                const double4_t vv[4] = {
                    partial[ii][jj][0],
                    swap1(partial[ii][jj][1]),
                    partial[ii][jj][2],
                    swap1(partial[ii][jj][3])
                };

                for (int jl = 0; jl < 4; ++jl) {
                    const int j = jb + 4 * jj + jl;
                    for (int il = 0; il < 4; ++il) {
                        const int i = ib + 4 * ii + il;
                        if (i < ny && j < ny && j <= i) {
                            // Every lane is a different correlation.
                            // Do NOT perform a horizontal sum here.
                            result[std::size_t(j) * ny + i] =
                                static_cast<float>(vv[il ^ jl][jl]);
                        }
                    }
                }
            }
        }
    }
}

void correlate(int ny, int nx, const float* data, float* result) {
    if (ny <= 0 || nx <= 0) return;
    const int row_blocks = (ny + 3) / 4;
    std::vector<double4_t> packed(
        std::size_t(row_blocks) * nx, double4_t{}
    );
    prepare_columns(ny, nx, data, packed);
    matrix_mult(ny, nx, packed, result);
}
