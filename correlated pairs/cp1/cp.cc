/*
This is the function you need to implement. Quick reference:
- input rows: 0 <= y < ny
- input columns: 0 <= x < nx
- element at row y and column x is stored in data[x + y*nx]
- the correlation between rows i and j has to be stored in result[i + j*ny]
- only elements with 0 <= j <= i < ny need to be filled

可以。完整证明最好分成“保持 correlation 不变”和“最后变成 dot product”两部分。

设原始两行是

$$
x=(x_1,\dots,x_n),\qquad y=(y_1,\dots,y_n)
$$

它们的 Pearson correlation 是

$$
\mathrm{corr}(x,y)
=
\frac{
\sum_{k=1}^n (x_k-\bar x)(y_k-\bar y)
}{
\sqrt{\sum_{k=1}^n (x_k-\bar x)^2}
\sqrt{\sum_{k=1}^n (y_k-\bar y)^2}
}
$$

其中

$$
\bar x=\frac1n\sum_k x_k,\qquad
\bar y=\frac1n\sum_k y_k
$$

### 第一步：减去均值不会改变 correlation

定义

$$
x'_k=x_k-\bar x,\qquad
y'_k=y_k-\bar y
$$

那么

$$
\bar{x'}=0,\qquad \bar{y'}=0
$$

而

$$
x'_k-\bar{x'}
=
x_k-\bar x
$$

同理

$$
y'_k-\bar{y'}
=
y_k-\bar y
$$

所以

$$
\mathrm{corr}(x',y')
=
\frac{
\sum_k (x_k-\bar x)(y_k-\bar y)
}{
\sqrt{\sum_k(x_k-\bar x)^2}
\sqrt{\sum_k(y_k-\bar y)^2}
}
$$

因此

$$
\boxed{\mathrm{corr}(x',y')=\mathrm{corr}(x,y)}
$$

这一步说明：**把每一行 center 到 mean 0，不会改变原来的 pairwise correlation。**

---

### 第二步：把每一行的平方和变成 1

定义

$$
\|x'\|=\sqrt{\sum_k (x'_k)^2}
$$

然后令

$$
\hat x_k=\frac{x'_k}{\|x'\|}
$$

同理

$$
\hat y_k=\frac{y'_k}{\|y'\|}
$$

于是：

$$
\sum_k \hat x_k^2=1,
\qquad
\sum_k \hat y_k^2=1
$$

也就是

$$
\|\hat x\|=\|\hat y\|=1
$$

这种 positive scaling 也不会改变 correlation，因此

$$
\boxed{
\mathrm{corr}(\hat x,\hat y)
=
\mathrm{corr}(x',y')
=
\mathrm{corr}(x,y)
}
$$

---

### 第三步：此时 correlation 退化成 dot product

因为

$$
\bar{\hat x}=0,\qquad
\bar{\hat y}=0
$$

所以 Pearson correlation 变成

$$
\mathrm{corr}(\hat x,\hat y)
=
\frac{
\sum_k \hat x_k\hat y_k
}{
\sqrt{\sum_k\hat x_k^2}
\sqrt{\sum_k\hat y_k^2}
}
$$

而我们已经把两个 norm 都变成了 1，所以：

$$
\mathrm{corr}(\hat x,\hat y)
=
\sum_k \hat x_k\hat y_k
$$

即

$$
\boxed{
\mathrm{corr}(x,y)=\hat x\cdot\hat y
}
$$

---

现在把所有 normalized rows \(\hat x_i\) 放进矩阵 \(X\)。

那么

$$
Y=XX^T
$$

其中第 \(i,j\) 个元素：

$$
Y_{ij}
=
\sum_k X_{ik}X_{jk}
$$

就是第 \(i\) 行和第 \(j\) 行的 dot product：

$$
Y_{ij}=\hat x_i\cdot\hat x_j
$$

而前面已经证明：

$$
\hat x_i\cdot\hat x_j
=
\mathrm{corr}(x_i,x_j)
$$

所以最终

$$
\boxed{
Y_{ij}=\mathrm{corr}(x_i,x_j)
}
$$

因此

$$
\boxed{Y=XX^T}
$$

就是整个 pairwise correlation matrix。

最核心的逻辑链是：

$$
\text{原始数据}
\rightarrow
\text{减均值，correlation 不变}
\rightarrow
\text{除以 norm，correlation 不变}
\rightarrow
\text{此时 correlation = dot product}
\rightarrow
XX^T
$$

这才是完整证明。
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

