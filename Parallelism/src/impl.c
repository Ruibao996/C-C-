// #include <immintrin.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

// void impl(int N, int step, double *p) {
//     double *p_next = (double*)aligned_alloc(32, N * N * sizeof(double));
//     if (!p_next) {
//         // Handle memory allocation failure
//         return;
//     }

//     memcpy(p_next, p, N * N * sizeof(double)); // Ensure p_next has the same initial values as p

//     for (int k = 0; k < step; k++) {
//         #pragma omp parallel for
//         for (int i = 1; i < N - 1; i++) {
//             int j;
//             for (j = 1; j <= N - 5; j += 4) {
//                 // Ensure the memory access is within bounds
//                 if (j + 3 >= N - 1) break;

//                 // Load data from memory
//                 __m256d top = _mm256_loadu_pd(&p[(i - 1) * N + j]);
//                 __m256d bottom = _mm256_loadu_pd(&p[(i + 1) * N + j]);
//                 __m256d left = _mm256_loadu_pd(&p[i * N + j - 1]);
//                 __m256d right = _mm256_loadu_pd(&p[i * N + j + 1]);

//                 // Compute the average
//                 __m256d sum = _mm256_add_pd(top, bottom);
//                 sum = _mm256_add_pd(sum, left);
//                 sum = _mm256_add_pd(sum, right);
//                 __m256d avg = _mm256_mul_pd(sum, _mm256_set1_pd(0.25));

//                 // Store the result
//                 _mm256_storeu_pd(&p_next[i * N + j], avg);
//             }
//             // Handle the remaining elements
//             for (; j < N - 1; j++) {
//                 p_next[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
//                                      p[i * N + j + 1] + p[i * N + j - 1]) / 4.0;
//             }
//         }
//         // Swap pointers
//         double *temp = p;
//         p = p_next;
//         p_next = temp;
//     }

//     // If the number of steps is odd, the final result is in p_next, so copy it back to p
    // if (step % 2 == 1) {
    //     memcpy(p, p_next, N * N * sizeof(double));
    // }
    // else {
    //     // Free the memory allocated for p_next
    //     free(p_next);
    // }
// }

// void impl(int N, int step, double *p) {
//   for (int k = 0; k < step; ++k)
//   {
//     #pragma omp parallel for schedule(dynamic, 128)
//     for (int i = 1; i < N-1; ++i)
//     {
//       int j = 1 + (i % 2 + k % 2) % 2;
//       for (; j < N - 1; j += 2) {
//         p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
//                         p[i * N + j - 1] + p[i * N + j + 1]) /
//                        4.0;
//       }
//     }
//   }
// }

// void impl(int N, int step, double *p) {
//   for (int k = 0; k < step; ++k)
//   {
//     #pragma omp parallel for
//     for (int i = 1; i < N-1; ++i)
//     {
//       int j = 1 + (i % 2 + k % 2) % 2;
//       for (; j < N - 1; j += 4) {
//         p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
//                         p[i * N + j - 1] + p[i * N + j + 1]) /
//                        4.0;
//         if (j + 2 < N - 1) {
//           p[i * N + j + 2] = (p[(i - 1) * N + j + 2] + p[(i + 1) * N + j + 2] +
//                               p[i * N + j + 1] + p[i * N + j + 3]) /
//                              4.0;
//         }
//       }
//     }
//   }
// }

void impl(int N, int step, double *p) {
  for (int k = 0; k < step; ++k)
  {
    #pragma omp parallel for schedule(dynamic, 64)
    for (int i = 1; i < N-1; ++i)
    {
      int j = 1 + (i % 2 + k % 2) % 2;
      for (; j < N - 3; j += 4) {
        p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
                        p[i * N + j - 1] + p[i * N + j + 1]) /
                       4.0;
        p[i * N + j + 2] = (p[(i - 1) * N + j + 2] + p[(i + 1) * N + j + 2] +
                            p[i * N + j + 1] + p[i * N + j + 3]) /
                           4.0;
      }
      if (j < N - 1) {
        p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
                        p[i * N + j - 1] + p[i * N + j + 1]) /
                       4.0;
      }
    }
  }
}

// void impl(int N, int step, double *p) {
//   for (int k = 0; k < step; ++k)
//   {
//     #pragma omp parallel for
//     for (int i = 1; i < N-1; ++i)
//     {
//       int j = 1 + (i % 2 + k % 2) % 2;
//       for (; j < N - 1; j += 8) {
//         p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
//                         p[i * N + j - 1] + p[i * N + j + 1]) /
//                        4.0;
//         if (j + 2 < N - 1) {
//           p[i * N + j + 2] = (p[(i - 1) * N + j + 2] + p[(i + 1) * N + j + 2] +
//                               p[i * N + j + 1] + p[i * N + j + 3]) /
//                              4.0;
//         if (j + 4 < N - 1) {
//           p[i * N + j + 4] = (p[(i - 1) * N + j + 4] + p[(i + 1) * N + j + 4] +
//                               p[i * N + j + 3] + p[i * N + j + 5]) /
//                              4.0;
//         }
//         if (j + 6 < N - 1) {
//           p[i * N + j + 6] = (p[(i - 1) * N + j + 6] + p[(i + 1) * N + j + 6] +
//                               p[i * N + j + 5] + p[i * N + j + 7]) /
//                              4.0;
//         }
//         }
//       }
//     }
//   }
// }

// void impl(int N, int step, double *p) {
//   for (int k = 0; k < step; ++k) {
// #pragma omp parallel for
//     for (int i = 1; i < N - 1; ++i) {
//       int j = 1 + (i % 2 + k % 2) % 2;
//       for (; j < N - 9; j += 8) {
//         p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
//                         p[i * N + j - 1] + p[i * N + j + 1]) /
//                        4.0;

//         p[i * N + j + 2] = (p[(i - 1) * N + j + 2] + p[(i + 1) * N + j + 2] +
//                             p[i * N + j + 1] + p[i * N + j + 3]) /
//                            4.0;

//         p[i * N + j + 4] = (p[(i - 1) * N + j + 4] + p[(i + 1) * N + j + 4] +
//                             p[i * N + j + 3] + p[i * N + j + 5]) /
//                            4.0;

//         p[i * N + j + 6] = (p[(i - 1) * N + j + 6] + p[(i + 1) * N + j + 6] +
//                             p[i * N + j + 5] + p[i * N + j + 7]) /
//                            4.0;
//       }

//       if (j < N - 1) {
//         p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
//                         p[i * N + j - 1] + p[i * N + j + 1]) /
//                        4.0;
//       }

//       if (j + 2 < N - 1) {
//         p[i * N + j + 2] = (p[(i - 1) * N + j + 2] + p[(i + 1) * N + j + 2] +
//                             p[i * N + j + 1] + p[i * N + j + 3]) /
//                            4.0;
//       }

//       if (j + 4 < N - 1) {
//         p[i * N + j + 4] = (p[(i - 1) * N + j + 4] + p[(i + 1) * N + j + 4] +
//                             p[i * N + j + 3] + p[i * N + j + 5]) /
//                            4.0;
//       }

//       if (j + 6 < N - 1) {
//         p[i * N + j + 6] = (p[(i - 1) * N + j + 6] + p[(i + 1) * N + j + 6] +
//                             p[i * N + j + 5] + p[i * N + j + 7]) /
//                            4.0;
//       }

//     }
//   }
// }