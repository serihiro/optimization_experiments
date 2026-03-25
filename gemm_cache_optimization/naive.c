#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
  double *c = (double *)calloc((size_t)n * n, sizeof(double));

  if (!a || !b || !c) {
    fprintf(stderr, "allocation failed\n");
    return 1;
  }

  fill_matrix(a, n, 7);
  fill_matrix(b, n, 11);

  const double t0 = now_sec();
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      double sum = 0.0;
      for (int k = 0; k < n; ++k) {
        sum += a[(size_t)i * n + k] * b[(size_t)k * n + j];
      }
      c[(size_t)i * n + j] = sum;
    }
  }
  const double elapsed = now_sec() - t0;
  const double gflops = (2.0 * n * n * n) / elapsed / 1e9;

  printf("elapsed_sec=%.6f gflops=%.3f checksum=%.12f\n", elapsed, gflops,
         checksum(c, n));

  free(a);
  free(b);
  free(c);
  return 0;
}
