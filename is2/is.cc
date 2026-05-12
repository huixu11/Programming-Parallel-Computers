struct Result {
    int y0;
    int x0;
    int y1;
    int x1;
    float outer[3];
    float inner[3];
};

/*
This is the function you need to implement. Quick reference:
- x coordinates: 0 <= x < nx
- y coordinates: 0 <= y < ny
- color components: 0 <= c < 3
- input: data[c + 3 * x + 3 * nx * y]
*/
int pref_index(int c, int y, int x, int nx) {
    return c + 3 * x + 3 * (nx + 1) * y;
}

int data_index(int c, int y, int x, int nx) {
    return c + 3 * x + 3 * nx * y;
}

void pref_sum(int ny, int nx, const float *data, double *pref_sum_data) {
    for (int c = 0; c < 3; c++) {
        for (int y = 1; y <= ny; y++) {
            for (int x = 1; x <= nx; x++) {
                double val = data[data_index(c, y-1, x-1, nx)];
                double left = (x > 0) ? pref_sum_data[pref_index(c, y, x-1, nx)] : 0;
                double up = (y > 0) ? pref_sum_data[pref_index(c, y-1, x, nx)] : 0;
                double diag = (x > 0 && y > 0) ? pref_sum_data[pref_index(c, y-1, x-1, nx)] : 0;
                pref_sum_data[pref_index(c, y, x, nx)] = val + left + up - diag;
            }
        }
    }
}

double rectangle_sum(int y0, int x0, int y1, int x1, int c, int nx, const double *pref_sum_data) {
    return pref_sum_data[pref_index(c, y1, x1, nx)] 
            - pref_sum_data[pref_index(c, y0, x1, nx)] 
            - pref_sum_data[pref_index(c, y1, x0, nx)] 
            + pref_sum_data[pref_index(c, y0, x0, nx)];
}


/*
Let color(y,x,c) = data[c + 3 * x + 3 * nx * y].
If (x,y) is located outside the rectangle: error(y,x,c) = outer[c] - color(y,x,c).
If (x,y) is located inside the rectangle: error(y,x,c) = inner[c] - color(y,x,c).

error(y,x,c) = outer[c] - color(y,x,c)
(outer[c] - color(y,x,c))^2 + (inner[c] - color(y,x,c))^2 

minimize sum of squared errors of outer

(color - val)^2 + (color - val)^2 + ... 
(color - val) + (color - val) + ... + (color - val) = 0

sum(color) = A_outer * val
val = sum(color)/A_outer

sum((val_outer_color - color(y,x,c))^2)
=sum(val^2 + color(y,x,c)^2 - 2*val*color(y,x,c))
=A_outer*val^2 + sum(color(y,x,c)^2) - 2*val*sum(color(y,x,c))
=A_outer*val^2 + sum(color(y,x,c)^2) - 2*val*sum(color(y,x,c))
=sum(color(y,x,c)^2) - A_outer*val^2
=sum(color(y,x,c)^2) - sum(color(y,x,c))^2/A_outer

min sum(all_pixels^2) - sum(outer)^2/A_outer - sum(inner)^2/A_inner
= constant - (sum(outer)^2/A_outer + sum(inner)^2/A_inner)

max  sum(outer)^2/A_outer + sum(inner)^2/A_inner
*/

Result segment(int ny, int nx, const float *data) {
    Result result{0, 0, 0, 0, {0, 0, 0}, {0, 0, 0}};
    double *pref_sum_data = new double[3 * (nx+1) * (ny+1)]();
    pref_sum(ny, nx, data, pref_sum_data);
    double best_score = -1;
    for (int h = 1; h <= ny; h++) {
        for (int w = 1; w <= nx; w++) {
            double inner_area = w * h;
            double outer_area = (nx * ny) - inner_area;
            double inverse_inner_area = 1.0 / inner_area;
            double inverse_outer_area = (outer_area > 0) ? 1.0 / outer_area : 0;
            // Implementation for segmenting
            for (int y0 = 0; y0 + h <= ny; y0++) {
                int y1 = y0 + h;
                for (int x0 = 0; x0 + w <= nx; x0++) {
                    int x1 = x0 + w;
                    double score = 0;
                    for (int c = 0; c < 3; c++) {
                        double sum_inner_c = rectangle_sum(y0, x0, y1, x1, c, nx, pref_sum_data);
                        double sum_outer_c = pref_sum_data[pref_index(c, ny, nx, nx)] - sum_inner_c;
                        score += sum_outer_c * sum_outer_c * inverse_outer_area;
                        score += sum_inner_c * sum_inner_c * inverse_inner_area;
                    }
                    if (score > best_score) {
                        best_score = score;
                        result.y0 = y0;
                        result.x0 = x0;
                        result.y1 = y1;
                        result.x1 = x1;
                    }
                }
            }
        }
    }
    double inner_area = (result.x1 - result.x0) * (result.y1 - result.y0);
    double outer_area = (nx * ny) - inner_area;
    for (int c = 0; c < 3; c++) {
        double sum_inner_c = rectangle_sum(result.y0, result.x0, result.y1, result.x1, c, nx, pref_sum_data);
        double sum_outer_c = pref_sum_data[pref_index(c, ny, nx, nx)] - sum_inner_c;
        result.inner[c] = sum_inner_c / inner_area;
        result.outer[c] = outer_area > 0 ? sum_outer_c / outer_area : 0;
    }
    delete[] pref_sum_data;
    return result;
}
