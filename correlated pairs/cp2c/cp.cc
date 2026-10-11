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

typedef double double4_t __attribute__ ((vector_size (4 * sizeof(double))));


// void minus_mean_row(int ny, int nx, const float *data, double *matrix) {
void minus_mean_row(int ny, int nx, const float *data, std::vector<double4_t> &matrix_v, int na) {
    for (int y = 0; y < ny; y++) {
        double mean = 0.0;
        for (int x = 0; x < nx; x++) {
            mean += static_cast<double>(data[x + y * nx]);
        }
        mean /= nx;
        for (int x = 0; x < nx; x++) {
            int k = x / 4;
            int lane = x % 4;
            matrix_v[y * na + k][lane] = static_cast<double>(data[x + y * nx]) - mean;
        }
    }
}

void normalize_row(int ny, std::vector<double4_t> &matrix_v, int na) {
    for (int y = 0; y < ny; y++) {
        double4_t sum_of_square_root = {0.0, 0.0, 0.0, 0.0};
        for (int x = 0; x < na; x++) {
            double4_t val = matrix_v[x + y*na];
            sum_of_square_root += val * val;
        }
        double norm = sqrt(sum_of_square_root[0] + sum_of_square_root[1] + sum_of_square_root[2] + sum_of_square_root[3]);
        if (norm == 0.0) {
            for (int x = 0; x < na; x++) {
                matrix_v[x + y*na] = double4_t{0.0, 0.0, 0.0, 0.0};
            }
        } else {
            for (int x = 0; x < na; x++) {
                matrix_v[x + y*na] /= norm;
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
        for (int j = 0; j <= i; j++) {
            double4_t sum = {0.0, 0.0, 0.0, 0.0};
            for (int k = 0; k < na; k++) {
                sum += matrix_v[k + i * na] * matrix_v[k + j * na];
            }
            result[i + j * ny] = static_cast<float>(sum[0] + sum[1] + sum[2] + sum[3]);
        }
    }
}

void correlate(int ny, int nx, const float *data, float *result) {
    constexpr int nb = 4;
    // vectors per input row
    int na = (nx + nb - 1) / nb;
    std::vector<double4_t> matrix_v(ny * na);
    minus_mean_row(ny, nx, data, matrix_v, na);
    normalize_row(ny, matrix_v, na);
    matrix_mult(ny, matrix_v, result, na);
}
