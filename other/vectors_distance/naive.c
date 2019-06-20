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
  printf("START\n");
  clock_t start = clock();
  for (int i = 0; i < ARAAY_SIZE; i++) {
    for (int j = 0; j < ARAAY_SIZE; j++) {
      out[j + i * ARAAY_SIZE] = sqrt(pow(X[i] - X[j], 2) + pow(Y[i] - Y[j], 2));
    }
  }
  clock_t end = clock();
  printf("END\n");

  printf("%lf秒かかりました\n", (double)(end - start) / CLOCKS_PER_SEC);

  free(out);
  return 0;
}