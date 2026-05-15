#include <iostream>
#include <limits>
#include <vector>
#include <random>
#include <chrono>

void step(float *r, const float *d, int n) {
    /*
    n = 4000
    Elapsed time: 523.321 seconds
    Check sum: 316608
    Min value: 3.57372e-06
    Max value: 0.088756
    */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float v = std::numeric_limits<float>::max();
            for (int k = 0; k < n; k++) {
                float x = d[i*n + k];
                float y = d[k*n + j];
                float z = x + y;
                v = std::min(v, z);
            }
            r[i*n + j] = v;
        }
    }
}


// void step(float *r, const float *d, int n) {
//     /*
//     n = 4000
//     Elapsed time: 20.8727 seconds
//     Check sum: 316608
//     Min value: 3.57372e-06
//     Max value: 0.088756
//     */
//     const float INF = std::numeric_limits<float>::max();
//     for (int i = 0; i < n; i++) {
//         for (int j = 0; j < n; j++) {
//             r[i*n + j] = INF;
//         }
//         for (int k = 0; k < n; k++) {
//             float x = d[i*n + k];
//             for (int j = 0; j < n; j++) {
//                 float y = d[k*n + j];
//                 float z = x + y;
//                 r[i*n + j] = std::min(r[i*n + j], z);
//             }
//         }
//     }
// }


// int main() {
//     constexpr int n = 3;
//     const float d[n*n] = {
//         0, 8, 2,
//         1, 0, 9,
//         4, 5, 0,
//     };
//     float r[n*n];
//     step(r, d, n);
//     for (int i = 0; i < n; i++) {
//         for (int j = 0; j < n; j++) {
//             std::cout << r[i*n + j] << " ";
//         }
//         std::cout << "\n";
//     }
//     return 0;
// }


int main() {
    constexpr int n = 4000; // must be known at compile time
    std::vector<float> d(n*n);
    std::vector<float> r(n*n);
    std::mt19937 rng(0);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (int i = 0; i < n*n; i++) {
        d[i] = dist(rng);
    }
    auto start = std::chrono::high_resolution_clock::now();
    step(r.data(), d.data(), n);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    double check_sum = 0;
    float min_val = std::numeric_limits<float>::max();
    float max_val = -std::numeric_limits<float>::max();
    for (int i = 0; i < n*n; i++) {
        check_sum += r[i];
        min_val = std::min(min_val, r[i]);
        max_val = std::max(max_val, r[i]);
    }
    std::cout << "n = " << n << "\n";
    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
    std::cout << "Check sum: " << check_sum << std::endl;
    std::cout << "Min value: " << min_val << std::endl;
    std::cout << "Max value: " << max_val << std::endl;
    return 0;
}