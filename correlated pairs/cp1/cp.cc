/*
This is the function you need to implement. Quick reference:
- input rows: 0 <= y < ny
- input columns: 0 <= x < nx
- element at row y and column x is stored in data[x + y*nx]
- the correlation between rows i and j has to be stored in result[i + j*ny]
- only elements with 0 <= j <= i < ny need to be filled

因为 **Pearson correlation 本质上就是：两个向量先减去各自均值，然后做 L2 normalization，最后取 dot product（点积）**。

假设矩阵中的两行分别是：

$$
a=(a_1,a_2,\dots,a_n)
$$

$$
b=(b_1,b_2,\dots,b_n)
$$

我们想计算它们的 Pearson correlation。

### 1. 从 correlation 的定义开始

Pearson correlation 是：

$$
\mathrm{corr}(a,b)
=
\frac{
\sum_{k=1}^n (a_k-\bar a)(b_k-\bar b)
}{
\sqrt{\sum_{k=1}^n(a_k-\bar a)^2}
\sqrt{\sum_{k=1}^n(b_k-\bar b)^2}
}
$$

其中：

$$
\bar a = \frac{1}{n}\sum_k a_k,
\qquad
\bar b = \frac{1}{n}\sum_k b_k
$$

关键就是观察这个公式。

---

### 2. 第一步：让每一行 mean = 0

定义 centered vectors：

$$
a'_k=a_k-\bar a
$$

$$
b'_k=b_k-\bar b
$$

于是：

$$
\sum_k a'_k=0
$$

$$
\sum_k b'_k=0
$$

correlation 变成：

$$
\mathrm{corr}(a,b)
=
\frac{
\sum_k a'_k b'_k
}{
\sqrt{\sum_k(a'_k)^2}
\sqrt{\sum_k(b'_k)^2}
}
$$

注意 numerator：

$$
\sum_k a'_k b'_k
$$

其实就是两个向量的 **dot product**：

$$
a'\cdot b'
$$

所以：

$$
\mathrm{corr}(a,b)
=
\frac{a'\cdot b'}{\|a'\|\|b'\|}
$$

你可能已经看出来了：**这就是 cosine similarity。**

换句话说：

> Pearson correlation = centered vectors 的 cosine similarity。

---

### 3. 第二步：让每一行 sum of squares = 1

现在再 normalize：

$$
x_a=\frac{a'}{\sqrt{\sum_k(a'_k)^2}}
=\frac{a'}{\|a'\|}
$$

同样：

$$
x_b=\frac{b'}{\|b'\|}
$$

那么：

$$
\sum_k x_{a,k}^2=1
$$

也就是：

$$
\|x_a\|=1
$$

$$
\|x_b\|=1
$$

现在计算两行的 dot product：

$$
x_a\cdot x_b
$$

代进去：

$$
=
\frac{a'}{\|a'\|}
\cdot
\frac{b'}{\|b'\|}
$$

所以：

$$
=
\frac{a'\cdot b'}
{\|a'\|\|b'\|}
$$

这正好就是：

$$
\boxed{\mathrm{corr}(a,b)}
$$

因此，只要每一行经过：

$$
\boxed{
x_i
=
\frac{
a_i-\mathrm{mean}(a_i)
}{
\sqrt{\sum_k(a_{ik}-\mathrm{mean}(a_i))^2}
}
}
$$

那么两个 normalized rows 的点积就是它们原来的 correlation。

---

## 4. 为什么 \(XX^T\) 一次得到所有 pairwise correlations？

假设：

$$
X=
\begin{bmatrix}
---x_1---\\
---x_2---\\
---x_3---\\
\vdots\\
---x_m---
\end{bmatrix}
$$

每一行代表一个已经完成上述 normalization 的 vector。

那么：

$$
Y=XX^T
$$

矩阵乘法的第 \(i,j\) 个元素：

$$
Y_{ij}
=
\sum_k X_{ik}X^T_{kj}
$$

而：

$$
X^T_{kj}=X_{jk}
$$

所以：

$$
Y_{ij}
=
\sum_kX_{ik}X_{jk}
$$

也就是：

$$
Y_{ij}=x_i\cdot x_j
$$

而我们刚才已经证明：

$$
x_i\cdot x_j
=
\mathrm{corr}(\text{row}_i,\text{row}_j)
$$

因此：

$$
\boxed{
Y_{ij}
=
\mathrm{corr}(\text{row}_i,\text{row}_j)
}
$$

于是：

$$
XX^T
=
\begin{bmatrix}
1 & corr(1,2)&corr(1,3)&\cdots\\
corr(2,1)&1&corr(2,3)&\cdots\\
corr(3,1)&corr(3,2)&1&\cdots\\
\vdots&&&\ddots
\end{bmatrix}
$$

这就是 **correlation matrix**。

---

### 一个很直观的理解

Pearson correlation 实际上在问：

> 「去掉两个变量各自的平均水平之后，它们变化的方向有多一致？」

所以先：

$$
a \rightarrow a-\bar a
$$

去掉 baseline。

然后 normalize：

$$
a-\bar a
\rightarrow
\frac{a-\bar a}{\|a-\bar a\|}
$$

去掉 magnitude。

这样剩下的只有 **direction**。

最后 dot product：

$$
x_i\cdot x_j
$$

就是比较两个 direction：

* \(1\)：完全同方向 → correlation = 1
* \(0\)：正交 → correlation = 0
* \(-1\)：完全反方向 → correlation = -1

所以可以记成：

$$
\boxed{
\text{Correlation}
=
\text{Center}
+
\text{Normalize}
+
\text{Dot Product}
}
$$

也正因为如此，把所有 normalized row 放进 \(X\) 后，一次矩阵乘法

$$
\boxed{XX^T}
$$

就相当于**同时算出了每一对 row 的 dot product，也就是所有 pairwise correlations**。

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

