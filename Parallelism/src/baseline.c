void baseline(int N, int step, double *p, double *p_next) {
  for (int k = 0; k < step; k++) {
    #pragma omp parallel for
      for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
          p_next[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
                              p[i * N + j + 1] + p[i * N + j - 1]) /
                              4.0f;
        }
      }
      double *temp = p;
      p = p_next;
      p_next = temp;
  }
}

// void baseline(int N, int step, double *p) {
//   for (int k = 0; k < step; ++k)
//   {
//     #pragma omp parallel for
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

// void baseline(int N, int step, double *p) {
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

// void baseline(int N, int step, double *p) {
//   for (int k = 0; k < step; ++k)
//   {
//     #pragma omp parallel for
//     for (int i = 1; i < N-1; ++i)
//     {
//       int j = 1 + (i % 2 + k % 2) % 2;
//       for (; j < N - 3; j += 4) {
//         p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
//                         p[i * N + j - 1] + p[i * N + j + 1]) /
//                        4.0;
//         p[i * N + j + 2] = (p[(i - 1) * N + j + 2] + p[(i + 1) * N + j + 2] +
//                             p[i * N + j + 1] + p[i * N + j + 3]) /
//                            4.0;
//       }
//       if (j < N - 1) {
//         p[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
//                         p[i * N + j - 1] + p[i * N + j + 1]) /
//                        4.0;
//       }
//     }
 
//   }
// }