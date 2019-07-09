#include "data.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  int i, j;
  for (i = 0; i < N; ++i) {
    V1[i] = 3.0;
    for (j = 0; j < N; ++j) {
      M1[i][j] = 2.0;
    }
  }

  double elapsed_time;
  int tmp_sum;
  elapsed_time = clock();
  for (i = 0; i < N; ++i) {
    tmp_sum = 0;
#pragma omp parallel for reduction(+ : tmp_sum)
    for (j = 0; j < N; ++j)
      tmp_sum += M1[i][j] * V1[j];
    V2[i] = tmp_sum;
  }
  elapsed_time = clock() - elapsed_time;
  printf("elapsed time = %.6f sec\n", elapsed_time / CLOCKS_PER_SEC);

  assert_result(N, V2, EXPECTED);

  return 0;
}
