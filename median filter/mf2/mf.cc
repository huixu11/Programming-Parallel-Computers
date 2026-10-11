/*
This is the function you need to implement. Quick reference:
- input rows: 0 <= y < ny
- input columns: 0 <= x < nx
- element at row y and column x is stored in in[x + y*nx]
- for each pixel (x, y), store the median of the pixels (a, b) which satisfy
  max(x-hx, 0) <= a < min(x+hx+1, nx), max(y-hy, 0) <= b < min(y+hy+1, ny)
  in out[x + y*nx].
*/
#include <algorithm>

void mf(int ny, int nx, int hy, int hx, const float *in, float *out) {
  #pragma omp parallel for
  for (int y = 0; y < ny; y++) {
    for (int x = 0; x < nx; x++) {
      int x_min = std::max(x - hx, 0);
      int x_max = std::min(x + hx + 1, nx);
      int y_min = std::max(y - hy, 0);
      int y_max = std::min(y + hy + 1, ny);
      int count = (x_max - x_min) * (y_max - y_min);
      double *temp = new double[count];
      int idx = 0;
      for (int y = y_min; y < y_max; y++) {
        for (int x = x_min; x < x_max; x++) {
          temp[idx++] = in[x + y * nx];
        }
      }
      if (count % 2 == 1) {
        std::nth_element(temp, temp + count / 2, temp + count);
        out[x + y * nx] = temp[count / 2];
      } else {
        std::nth_element(temp, temp + count / 2 - 1, temp + count);
        double median1 = temp[count / 2 - 1];
        std::nth_element(temp, temp + count / 2, temp + count);
        double median2 = temp[count / 2];
        out[x + y * nx] = (median1 + median2) / 2.0;
      }
      delete[] temp;
    }
  }
}
