#include "util.h"
#include <stdio.h>

void assert_result(int n, double m[n][n], double expected) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      assert(m[i][j] == expected);
}
