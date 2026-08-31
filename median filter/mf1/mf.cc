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
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int a_min = std::max(x - hx, 0);
      int a_max = std::min(x + hx + 1, nx);
      int b_min = std::max(y - hy, 0);
      int b_max = std::min(y + hy + 1, ny);
      int count = (a_max - a_min) * (b_max - b_min);
      double *temp = new double[count];
      int idx = 0;
      for (int b = b_min; b < b_max; ++b) {
        for (int a = a_min; a < a_max; ++a) {
          temp[idx++] = static_cast<double>(in[a + b * nx]);
        }
      }
      if (count % 2 == 1) {
        std::nth_element(temp, temp + count / 2, temp + count);
        out[x + y * nx] = static_cast<float>(temp[count / 2]);
      } else {
        std::nth_element(temp, temp + count / 2, temp + count);
        double mid1 = temp[count / 2];
        std::nth_element(temp, temp + count / 2 - 1, temp + count);
        double mid2 = temp[count / 2 - 1];
        out[x + y * nx] = static_cast<float>((mid1 + mid2) / 2.0);
      }
      delete[] temp;
    }
  }
}
