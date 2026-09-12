/*
This is the function you need to implement. Quick reference:
- input rows: 0 <= y < ny
- input columns: 0 <= x < nx
- element at row y and column x is stored in data[x + y*nx]
- the correlation between rows i and j has to be stored in result[i + j*ny]
- only elements with 0 <= j <= i < ny need to be filled
*/
# include <cmath>

void normalize_mean_0(double *data_v, const float *data, int i, int nx) {
    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;
    double s3 = 0.0;
    int x = 0;
    for (; x + 3 < nx; x += 4) {
        s0 += static_cast<double>(data[x + i*nx]);
        s1 += static_cast<double>(data[x + 1 + i*nx]);
        s2 += static_cast<double>(data[x + 2 + i*nx]);
        s3 += static_cast<double>(data[x + 3 + i*nx]);
    }
    double sum = s0 + s1 + s2 + s3;
    for (; x < nx; x++) {
        sum += static_cast<double>(data[x + i*nx]);
    }
    double mean = sum / nx;
    for (int x = 0; x < nx; x++) {
        data_v[x + i * nx] = static_cast<double>(data[x + i * nx]) - mean;
    }
}


void sum_square_equal_1(double *data_v, int i, int nx) {
    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;
    double s3 = 0.0;
    int x = 0;
    for (; x + 3 < nx; x += 4) {
        double v0 = data_v[x + i * nx];
        double v1 = data_v[x + 1 + i * nx];
        double v2 = data_v[x + 2 + i * nx];
        double v3 = data_v[x + 3 + i * nx];
        s0 += v0 * v0;
        s1 += v1 * v1;
        s2 += v2 * v2;
        s3 += v3 * v3;
    }
    double sum_square = s0 + s1 + s2 + s3;
    for (; x < nx; x++) {
        double value = data_v[x + i * nx];
        sum_square += value * value;
    }
    double norm_factor = std::sqrt(sum_square);
    if (norm_factor == 0.0) {
        for (int x = 0; x < nx; x++) {
            data_v[x + i * nx] = 0.0;
        }
        return;
    }
    for (int x = 0; x < nx; x++) {
        data_v[x + i * nx] /= norm_factor;
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
            double s0 = 0.0;
            double s1 = 0.0;
            double s2 = 0.0;
            double s3 = 0.0;

            int x = 0;
            for (; x + 3 < nx; x += 4) {
                s0 += data_v[x + i * nx] * data_v[x + j * nx];
                s1 += data_v[x + 1 + i * nx] * data_v[x + 1 + j * nx];
                s2 += data_v[x + 2 + i * nx] * data_v[x + 2 + j * nx];
                s3 += data_v[x + 3 + i * nx] * data_v[x + 3 + j * nx];
            }
            double sum = s0 + s1 + s2 + s3;
            // add the items that are not been added yet
            for (; x < nx; x++) {
                sum += data_v[x + i * nx] * data_v[x + j * nx];
            }
            result[i + j * ny] = static_cast<float>(sum);
        }
    }
    delete[] data_v;
}
