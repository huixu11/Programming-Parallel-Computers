/*
This is the function you need to implement. Quick reference:
- input rows: 0 <= y < ny
- input columns: 0 <= x < nx
- element at row y and column x is stored in data[x + y*nx]
- the correlation between rows i and j has to be stored in result[i + j*ny]
- only elements with 0 <= j <= i < ny need to be filled
*/

#include <math.h>

void minus_mean_row(int ny, int nx, const float *data, double *matrix) {
    for (int y = 0; y < ny; y++) {
        double mean = 0;
        for (int x = 0; x < nx; x++) {
            mean += data[x + y * nx];
        }
        mean /= nx;
        for (int x = 0; x < nx; x++) {
            matrix[x + y * nx] = data[x + y * nx] - mean;
        }
    }
}

void normalize_row(int ny, int nx, double *matrix) {
    for (int y = 0; y < ny; y++) {
        double size = 0;
        for (int x = 0; x < nx; x++) {
            size += matrix[x + y * nx] * matrix[x + y * nx];
        }
        size = sqrt(size);
        for (int x = 0; x < nx; x++) {
            if (size == 0.0) {
                matrix[x + y * nx] = 0.0;
            } else {
                matrix[x + y * nx] /= size;
            }
        }
    }
}

void matrix_mult(int ny, int nx, double *matrix, float *result) {
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j <= i; j++) {
            double sum_square = 0.0;
            for (int k = 0; k < nx; k++) {
                sum_square += matrix[k + i * nx] * matrix[k + j * nx];
            }
            result[i + j * ny] = static_cast<float>(sum_square);
        }
    }
}

void correlate(int ny, int nx, const float *data, float *result) {
    double *matrix = new double[ny * nx];
    minus_mean_row(ny, nx, data, matrix);
    normalize_row(ny, nx, matrix);
    matrix_mult(ny, nx, matrix, result);
    delete[] matrix;
}
