/*
This is the function you need to implement. Quick reference:
- input rows: 0 <= y < ny
- input columns: 0 <= x < nx
- element at row y and column x is stored in data[x + y*nx]
- the correlation between rows i and j has to be stored in result[i + j*ny]
- only elements with 0 <= j <= i < ny need to be filled
*/
#include <cmath>

void normalize_mean_0(double *data_v, const float *data, int i, int nx) {
    double sum = 0.0;
    for (int x = 0; x < nx; x++) {
        sum += static_cast<double>(data[x + i*nx]);
    }
    double mean = sum / nx;
    for (int x = 0; x < nx; x++) {
        data_v[x + i*nx] = static_cast<double>(data[x + i*nx]) - mean;
    }
}

void sum_square_equal_1(double *data_v, int i, int nx) {
    double sum_square = 0.0;
    for (int x = 0; x < nx; x++) {
        double value = data_v[x + i*nx];
        sum_square += value * value;
    }
    double norm_factor = std::sqrt(sum_square);
    if (norm_factor == 0.0) {
        for (int x = 0; x < nx; x++) {
            data_v[x + i*nx] = 0.0;
        }
        return;
    }
    for (int x = 0; x < nx; x++) {
        data_v[x + i*nx] /= norm_factor;
    }
}


void correlate(int ny, int nx, const float *data, float *result) {
    double *data_v = new double[ny * nx];
    for (int i = 0; i < ny; i++) {
        normalize_mean_0(data_v, data, i, nx);
        sum_square_equal_1(data_v, i, nx);
    }
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j <= i; j++) {
            double sum = 0.0;
            for (int x = 0; x < nx; x++) {
                sum += data_v[x + i * nx] * data_v[x + j * nx];
            }
            result[i + j * ny] = static_cast<float>(sum);
        }
    }
    delete[] data_v;
}

