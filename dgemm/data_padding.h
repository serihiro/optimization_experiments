#include <stdio.h>
#include <time.h>

const int N = 1024;
const double EXPECTED = N * 6.0;
unsigned long FLOPs = N * N * N * 2l;
double M1[1024][1025];
double M2[1024][1025];
double M3[1024][1025];
