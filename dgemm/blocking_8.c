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

  int block_size, ii, jj, k, kk;
  double elapsed_time;
  block_size = 8;
  elapsed_time = clock();
  for (i = 0; i < N; i += block_size) {
    for (j = 0; j < N; j += block_size) {
      for (k = 0; k < N; k += block_size) {
        for (ii = i; ii < i + block_size; ++ii) {
          for (jj = j; jj < j + block_size; ++jj) {
            for (kk = k; kk < k + block_size; ++kk) {
              M3[ii][jj] += M1[ii][kk] * M2[kk][jj];
            }
          }
        }
      }
    }
  }
  elapsed_time = clock() - elapsed_time;
  printf("elapsed time = %.6f sec\n", elapsed_time / CLOCKS_PER_SEC);
  assert_result(N, M3, EXPECTED);

  return 0;
}
