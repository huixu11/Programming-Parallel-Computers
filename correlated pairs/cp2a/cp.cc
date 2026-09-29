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
        double mean0 = 0;
        double mean1 = 0;
        double mean2 = 0;
        double mean3 = 0;
        
        int x = 0;

        for (; x + 3 < nx; x += 4) {
            mean0 += data[x + y * nx];
            mean1 += data[x + 1 + y * nx];
            mean2 += data[x + 2 + y * nx];
            mean3 += data[x + 3 + y * nx];
        }
        double mean = mean0 + mean1 + mean2 + mean3;
        for (; x < nx; x++) {
            mean += data[x + y * nx];
        }

        mean /= nx;
        
        x = 0;
        for (; x + 3 < nx; x += 4) {
            matrix[x + y * nx] = data[x + y * nx] - mean;
            matrix[x + 1 + y * nx] = data[x + 1 + y * nx] - mean;
            matrix[x + 2 + y * nx] = data[x + 2 + y * nx] - mean;
            matrix[x + 3 + y * nx] = data[x + 3 + y * nx] - mean;
        }
        for (; x < nx; x++) {
            matrix[x + y * nx] = data[x + y * nx] - mean;
        }
    }
}

void normalize_row(int ny, int nx, double *matrix) {
    for (int y = 0; y < ny; y++) {
        double size0 = 0;
        double size1 = 0;
        double size2 = 0;
        double size3 = 0;
        int x = 0;
        for (; x + 3 < nx; x += 4) {
            size0 += matrix[x + y * nx] * matrix[x + y * nx];
            size1 += matrix[x + 1 + y * nx] * matrix[x + 1 + y * nx];
            size2 += matrix[x + 2 + y * nx] * matrix[x + 2 + y * nx];
            size3 += matrix[x + 3 + y * nx] * matrix[x + 3 + y * nx];
        }
        double size = size0 + size1 + size2 + size3;
        for (; x < nx; x++) {
            size += matrix[x + y * nx] * matrix[x + y * nx];
        }
        size = sqrt(size);

        x = 0;
        if (size == 0.0) {
            for (; x + 3 < nx; x+=4) {
                matrix[x + y * nx] = 0.0;
                matrix[x + 1 + y * nx] = 0.0;
                matrix[x + 2 + y * nx] = 0.0;
                matrix[x + 3 + y * nx] = 0.0;
            }
            for (; x < nx; x++) {
                matrix[x + y * nx] = 0.0;
            }
        } else {
            for (; x + 3 < nx; x+=4) {
                matrix[x + y * nx] /= size;
                matrix[x + 1 + y * nx] /= size;
                matrix[x + 2 + y * nx] /= size;
                matrix[x + 3 + y * nx] /= size;
            }
            for (; x < nx; x++) {
                matrix[x + y * nx] /= size;
            }
        }
    }
}

void matrix_mult(int ny, int nx, double *matrix, float *result) {
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j <= i; j++) {
            double sum_square0 = 0.0;
            double sum_square1 = 0;
            double sum_square2 = 0;
            double sum_square3 = 0;
            int k = 0;

            for (; k + 3 < nx; k+=4) {
                sum_square0 += matrix[k + i * nx] * matrix[k + j * nx];
                sum_square1 += matrix[k + 1 + i * nx] * matrix[k + 1 + j * nx];
                sum_square2 += matrix[k + 2 + i * nx] * matrix[k + 2 + j * nx];
                sum_square3 += matrix[k + 3 + i * nx] * matrix[k + 3 + j * nx];
            }
            double sum_square = sum_square0 + sum_square1 + sum_square2 + sum_square3;

            for (; k < nx; k++) {
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
