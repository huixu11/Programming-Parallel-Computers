struct Result {
    float avg[3];
};

/*
This is the function you need to implement. Quick reference:
- x coordinates: 0 <= x < nx
- y coordinates: 0 <= y < ny
- horizontal position: 0 <= x0 < x1 <= nx
- vertical position: 0 <= y0 < y1 <= ny
- color components: 0 <= c < 3
- input: data[c + 3 * x + 3 * nx * y]
- output: avg[c]
*/
Result calculate(int ny, int nx, const float *data, int y0, int x0, int y1, int x1) {
    Result result{{0.0f, 0.0f, 0.0f}};
    if (y0 < 0 || y1 > ny || x0 < 0 || x1 > nx || y0 >= y1 || x0 >= x1) {
        return result; // Return zero averages for invalid input
    }
    double count = static_cast<double>((y1 - y0) * (x1 - x0));
    double sum[3] = {0.0, 0.0, 0.0};
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            for (int c = 0; c < 3; ++c) {
                sum[c] += static_cast<double>(data[c + 3 * x + 3 * nx * y]);
            }
        }
    }
    for (int c = 0; c < 3; ++c) {
        result.avg[c] = static_cast<float>(sum[c] / count);
    }
    return result;
}
