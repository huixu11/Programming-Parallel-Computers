/*
This is the function you need to implement. Quick reference:
- input rows: 0 <= y < ny
- input columns: 0 <= x < nx
- element at row y and column x is stored in data[x + y*nx]
- the correlation between rows i and j has to be stored in result[i + j*ny]
- only elements with 0 <= j <= i < ny need to be filled
*/
#include <math.h>
#include <vector>
#include <x86intrin.h>

typedef double double4_t __attribute__ ((vector_size (4 * sizeof(double))));


// =================================Vector in a column===============================================
// Keep the V5 permutation/register-reuse strategy, but express the data
// movement using vector shuffles instead of explicit _mm256_* intrinsics.
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

static void minus_mean_row_vector_column(int ny, int nx, const float *data, std::vector<double> &mean, std::vector<double> &inv_norm) {
    for (int y = 0; y < ny; y++) {
        const std::size_t data_row = static_cast<std::size_t>(y) * nx;
        double sum = 0.0;
        for (int x = 0; x < nx; x++) {
            sum += static_cast<double>(data[data_row + static_cast<std::size_t>(x)]);
        }
        const double row_mean = sum / static_cast<double>(nx);

        double sum_of_squares = 0.0;
        for (int x = 0; x < nx; x++) {
            double centered = static_cast<double>(data[data_row + static_cast<std::size_t>(x)]) - row_mean;
            sum_of_squares += centered * centered;
        }
        const double row_norm = sqrt(sum_of_squares);
        
        const std::size_t yi = static_cast<std::size_t>(y);
        mean[yi] = row_mean;
        inv_norm[yi] = row_norm > 0.0 ? 1.0 / row_norm : 0.0;
    }
}



// 2. Pack four normalized rows into one double4_t.
//    packed[block * nx + x][lane] stores the value of
//    row block * 4 + lane at feature x.
static void packed_rows(int ny, int nx, const float *data, const std::vector<double> &mean, const std::vector<double> &inv_norm, std::vector<double4_t> &packed, int nblocks, int W) {
    for (int block = 0; block < nblocks; block++) {
        const std::size_t block_row = static_cast<std::size_t>(block) * static_cast<std::size_t>(nx);
        for (int x = 0; x < nx; x++) {
            double4_t row_values = {0.0, 0.0, 0.0, 0.0};
            for (int lane = 0; lane < W; lane++) {
                const int row = block * W + lane;
                if (row < ny) {
                    const std::size_t yi = static_cast<std::size_t>(row);
                    const std::size_t data_idx = yi * static_cast<std::size_t>(nx) + static_cast<std::size_t>(x);
                    row_values[lane] = (static_cast<double>(data[data_idx]) - mean[yi]) * inv_norm[yi];
                }
            }
            packed[block_row + static_cast<std::size_t>(x)] = row_values;
        }
    }
}

static void matrix_mult_vector_column(int ny, int nx, std::vector<double4_t> &packed, float *result, int nblocks, int W) { 
    for (int bi = 0; bi < nblocks; bi++) {
        const std::size_t base_a = static_cast<std::size_t>(bi) * static_cast<std::size_t>(nx);
        for (int bj = 0; bj <= bi; bj++) {
            const std::size_t base_b = static_cast<std::size_t>(bj) * static_cast<std::size_t>(nx);
            double4_t vv[4]= {0.0, 0.0, 0.0, 0.0};
            for (int k = 0; k < nx; k++) {
                const double4_t val_a = packed[base_a + static_cast<std::size_t>(k)];
                const double4_t val_b = packed[base_b + static_cast<std::size_t>(k)];
                const double4_t val_a_2 = swap2(val_a);
                const double4_t val_b_1 = swap1(val_b);
                vv[0] += val_a * val_b;
                vv[1] += val_a * val_b_1;
                vv[2] += val_a_2 * val_b;
                vv[3] += val_a_2 * val_b_1;
            }

            // Rearrange odd accumulators to restore lane ordering.
            vv[1] = swap1(vv[1]);
            vv[3] = swap1(vv[3]);

            for (int ib = 0; ib < W; ib++) {
                const int i = bi * W + ib;
                if (i >= ny) {
                    continue;
                }
                for (int jb = 0; jb < W; jb++) {
                    const int j = bj * W + jb;
                    if (j >= ny || i < j) {
                        continue;
                    }
                    // Perform the matrix multiplication and store the result
                    const int q = ib ^ jb;
                    const double corr = vv[q][jb];

                    const std::size_t result_idx = static_cast<std::size_t>(j) * static_cast<std::size_t>(ny) + static_cast<std::size_t>(i);
                    result[result_idx] = static_cast<float>(corr);
                }
            }
        }
    }
}




//=====================================Vector in a row============================================
// void minus_mean_row(int ny, int nx, const float *data, double *matrix) {
void minus_mean_row(int ny, int nx, const float *data, std::vector<double4_t> &matrix_v, int na) {
    for (int y = 0; y < ny; y++) {
        const std::size_t data_row = static_cast<std::size_t>(y) * nx;
        const std::size_t matrix_row = static_cast<std::size_t>(y) * na;
        double mean = 0.0;
        for (int x = 0; x < nx; x++) {
            mean += static_cast<double>(data[data_row + static_cast<std::size_t>(x)]);
        }
        mean /= static_cast<double>(nx);
        for (int x = 0; x < nx; x++) {
            int k = x / 4;
            int lane = x % 4;
            matrix_v[matrix_row + static_cast<std::size_t>(k)][lane] = static_cast<double>(data[data_row + static_cast<std::size_t>(x)]) - mean;
        }
    }
}

void normalize_row(int ny, std::vector<double4_t> &matrix_v, int na) {
    for (int y = 0; y < ny; y++) {
        const std::size_t matrix_row = static_cast<std::size_t>(y) * na;
        double4_t sum_of_square_root = {0.0, 0.0, 0.0, 0.0};
        for (int x = 0; x < na; x++) {
            double4_t val = matrix_v[matrix_row + static_cast<std::size_t>(x)];
            sum_of_square_root += val * val;
        }
        double norm = sqrt(sum_of_square_root[0] + sum_of_square_root[1] + sum_of_square_root[2] + sum_of_square_root[3]);
        if (norm == 0.0) {
            for (int x = 0; x < na; x++) {
                matrix_v[matrix_row + static_cast<std::size_t>(x)] = double4_t{0.0, 0.0, 0.0, 0.0};
            }
        } else {
            for (int x = 0; x < na; x++) {
                matrix_v[matrix_row + static_cast<std::size_t>(x)] /= norm;
            }
        }
        
    }
}

void matrix_mult(int ny, std::vector<double4_t> &matrix_v, float *result, int na) {
    /*
        matrix * matrix^{T} = matrix[row] * matrix[row]
        correlate matrix column <= row
    */
    for (int i = 0; i < ny; i++) {
        const std::size_t matrix_row_i = static_cast<std::size_t>(i) * na;
        for (int j = 0; j <= i; j++) {
            const std::size_t matrix_row_j = static_cast<std::size_t>(j) * na;
            double4_t sum = {0.0, 0.0, 0.0, 0.0};
            for (int k = 0; k < na; k++) {
                sum += matrix_v[matrix_row_i + static_cast<std::size_t>(k)] * matrix_v[matrix_row_j + static_cast<std::size_t>(k)];
            }
            const std::size_t result_index = static_cast<std::size_t>(i) + static_cast<std::size_t>(j) * static_cast<std::size_t>(ny);
            result[result_index] = static_cast<float>(sum[0] + sum[1] + sum[2] + sum[3]);
        }
    }
}


void correlate(int ny, int nx, const float *data, float *result) {
    if (ny <= 0 || nx <= 0) return;
    // constexpr int nb = 4;
    // // ====================================
    // // vectors per input row https://ppc.cs.aalto.fi/ch2/v4/
    // int na = (nx + nb - 1) / nb;
    // std::vector<double4_t> matrix_v(ny * na);
    // minus_mean_row(ny, nx, data, matrix_v, na);
    // normalize_row(ny, matrix_v, na);
    // matrix_mult(ny, matrix_v, result, na);
    // // ====================================
    
    // vectors per input column https://ppc.cs.aalto.fi/ch2/v5/
    // Compute row means and inverse centered norms in double precision.
    std::vector<double> mean(static_cast<std::size_t>(ny));
    std::vector<double> inv_norm(static_cast<std::size_t>(ny));

    minus_mean_row_vector_column(ny, nx, data, mean, inv_norm);

    // Pack four rows at each feature position into one double4_t.
    // packed[block * nx + k][lane] contains normalized row block*4+lane,
    // feature k. std::vector provides suitably aligned storage for vector types
    // with C++17 and a conforming GCC/Clang implementation.
    constexpr int W = 4;
    const int nblocks = (ny + W - 1) / W;
    std::vector<double4_t> packed(static_cast<std::size_t>(nblocks) * nx);
    packed_rows(ny, nx, data, mean, inv_norm, packed, nblocks, W);

    matrix_mult_vector_column(ny, nx, packed, result, nblocks, W);

}
