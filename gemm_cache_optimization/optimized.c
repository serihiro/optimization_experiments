#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BLOCK_SIZE 64

static double now_sec(void) {
  return (double)clock() / CLOCKS_PER_SEC;
}

static void fill_matrix(double *m, int n, int seed) {
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      m[(size_t)i * n + j] = ((i * 131 + j * 17 + seed) % 100) / 100.0;
    }
  }
}

static void transpose(const double *src, double *dst, int n) {
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      dst[(size_t)j * n + i] = src[(size_t)i * n + j];
}

static double checksum(const double *m, int n) {
  double s = 0.0;
  for (int i = 0; i < n * n; ++i)
    s += m[i];
  return s;
}

int main(int argc, char **argv) {
  const int n = (argc >= 2) ? atoi(argv[1]) : 512;
  double *a = (double *)calloc((size_t)n * n, sizeof(double));
  double *b = (double *)calloc((size_t)n * n, sizeof(double));
  double *bt = (double *)calloc((size_t)n * n, sizeof(double));
  double *c = (double *)calloc((size_t)n * n, sizeof(double));

  if (!a || !b || !bt || !c) {
    fprintf(stderr, "allocation failed\n");
    return 1;
  }

  fill_matrix(a, n, 7);
  fill_matrix(b, n, 11);
  transpose(b, bt, n);

  const double t0 = now_sec();
  for (int ii = 0; ii < n; ii += BLOCK_SIZE) {
    const int i_max = (ii + BLOCK_SIZE < n) ? (ii + BLOCK_SIZE) : n;
    for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
      const int j_max = (jj + BLOCK_SIZE < n) ? (jj + BLOCK_SIZE) : n;
      for (int kk = 0; kk < n; kk += BLOCK_SIZE) {
        const int k_max = (kk + BLOCK_SIZE < n) ? (kk + BLOCK_SIZE) : n;

        for (int i = ii; i < i_max; ++i) {
          const double *a_row = &a[(size_t)i * n];
          for (int j = jj; j < j_max; ++j) {
            const double *bt_row = &bt[(size_t)j * n];
            double sum = c[(size_t)i * n + j];

            int k = kk;
            for (; k + 3 < k_max; k += 4) {
              sum += a_row[k] * bt_row[k];
              sum += a_row[k + 1] * bt_row[k + 1];
              sum += a_row[k + 2] * bt_row[k + 2];
              sum += a_row[k + 3] * bt_row[k + 3];
            }
            for (; k < k_max; ++k)
              sum += a_row[k] * bt_row[k];

            c[(size_t)i * n + j] = sum;
          }
        }
      }
    }
  }
  const double elapsed = now_sec() - t0;
  const double gflops = (2.0 * n * n * n) / elapsed / 1e9;

  printf("elapsed_sec=%.6f gflops=%.3f checksum=%.12f\n", elapsed, gflops,
         checksum(c, n));

  free(a);
  free(b);
  free(bt);
  free(c);
  return 0;
}
