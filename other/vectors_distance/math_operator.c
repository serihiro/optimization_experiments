#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARAAY_SIZE 10000
float X[ARAAY_SIZE];
float Y[ARAAY_SIZE];

int main(void) {
  srand(time(NULL));

  for (int i = 0; i < ARAAY_SIZE; i++) {
    X[i] = (float)rand() / 32767.0;
    Y[i] = (float)rand() / 32767.0;
  }

  float *out = (float *)malloc(ARAAY_SIZE * ARAAY_SIZE * sizeof(float));
  float x_diff, y_diff;
  printf("START\n");
  clock_t start = clock();
  for (int i = 0; i < ARAAY_SIZE; i++) {
    for (int j = 0; j < ARAAY_SIZE; j++) {
      x_diff = X[i] - X[j];
      y_diff = Y[i] - Y[j];
      out[j + i * ARAAY_SIZE] = sqrt(x_diff * x_diff + y_diff * y_diff);
    }
  }
  clock_t end = clock();
  printf("END\n");
  printf("%lf秒かかりました\n", (double)(end - start) / CLOCKS_PER_SEC);

  free(out);
  return 0;
}