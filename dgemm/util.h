#include <assert.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

void assert_result(int n, double m[n][n], double expected);
void print_performance(double elapsed_time, unsigned long flps);
