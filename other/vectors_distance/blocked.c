#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARAAY_SIZE 10000
#define BLOCK_SIZE 32
float X[ARAAY_SIZE];
float Y[ARAAY_SIZE];

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

int main(void) {
  srand(time(NULL));

  for (int i = 0; i < ARAAY_SIZE; i++) {
    X[i] = (float)rand() / 32767.0;
    Y[i] = (float)rand() / 32767.0;
  }

  float *out = (float *)malloc(ARAAY_SIZE * ARAAY_SIZE * sizeof(float));

  printf("START\n");
  clock_t start = clock();
  for (int i = 0; i < ARAAY_SIZE; i += BLOCK_SIZE) {
    for (int j = 0; j < ARAAY_SIZE; j += BLOCK_SIZE) {
      for (int ii = i; ii < MIN(i + BLOCK_SIZE, ARAAY_SIZE); ++ii) {
        for (int jj = j; jj < MIN(j + BLOCK_SIZE, ARAAY_SIZE); ++jj) {
          out[jj + ii * ARAAY_SIZE] =
              sqrt(pow(X[ii] - X[jj], 2) + pow(X[ii] - Y[jj], 2));
        }
      }
    }
  }
  clock_t end = clock();
  printf("END\n");

  printf("%lf秒かかりました\n", (double)(end - start) / CLOCKS_PER_SEC);

  free(out);
  return 0;
}