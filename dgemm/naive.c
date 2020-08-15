#include "data.h"
#include "util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  int i, j;
  for (i = 0; i < N; ++i) {
    for (j = 0; j < N; ++j) {
      M1[i][j] = 2.0;
      M2[i][j] = 3.0;
    }
  }

  int k;
  double elapsed_time;
  elapsed_time = clock();
  for (i = 0; i < N; ++i) {
    for (j = 0; j < N; ++j) {
      for (k = 0; k < N; ++k) {
        M3[i][j] += M1[i][k] * M2[k][j];
      }
    }
  }
  elapsed_time = clock() - elapsed_time;
  double time = elapsed_time / CLOCKS_PER_SEC;
  print_performance(time, FLOPs);
  assert_result(N, M3, EXPECTED);

  return 0;
}
