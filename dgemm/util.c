#include "util.h"
#include <stdio.h>

void assert_result(int n, double m[n][n], double expected) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      assert(m[i][j] == expected);
}

void print_performance(double elapsed_time, unsigned long long flps) {
  printf("elapsed time = %.6f sec, %.6f GFLOPS\n", elapsed_time,
         flps / elapsed_time / 1e9);
}
