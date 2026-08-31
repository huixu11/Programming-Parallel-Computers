#include "mm.h"
#include <cstring>
/*
This is the function you need to implement. Quick reference:
 matrix A: (m, k); A[i, j] = A[i*k + j]
 matrix B: (k, n); B[i, j] = B[i*n + j]
 matrix C: (m, n); C[i, j] = C[i*n + j]

 Look at how memory is accessed when j is incrementing:

C[i*n + j] accesses memory sequentially.

B[p*n + j] accesses memory sequentially.

A[i*k + p] stays constant for the entire inner loop.

This sequential access pattern is incredibly cache-friendly. 
The CPU's hardware prefetcher will notice you are reading straight through memory
 and will pull the data into the cache before you even ask for it. 
 Furthermore, it allows the compiler to easily auto-vectorize your code.
*/
void gemm(int m, int n, int k, const std::int8_t *A, const std::int8_t *B, std::int32_t *C) {
    memset(C, 0, sizeof(std::int32_t) * m * n);
    for (int i = 0; i < m; i++) {
        for (int p = 0; p < k; p++) {
            // important: iterate over p in the loop to improve cache locality
            int32_t a_val = static_cast<int32_t>(A[i*k + p]);
            for (int j = 0; j < n; j++) {
                C[i*n + j] += a_val * static_cast<int32_t>(B[p*n + j]);
            }
        }
    }
}
