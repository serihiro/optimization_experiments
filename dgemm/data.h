#include <stdio.h>
#include <time.h>

const long N = 1024l;
const double EXPECTED = N * 6.0;
unsigned long long FLOPs = N * N * N * 2l;
double M1[1024][1024];
double M2[1024][1024];
double M3[1024][1024];
