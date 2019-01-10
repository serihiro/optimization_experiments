#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "data.h"
#include "util.h"

int main()
{
    int i, j;
    for (i = 0; i < N; ++i)
    {
        for (j = 0; j < N; ++j)
        {
            M1[i][j] = 2.0;
            M2[i][j] = 3.0;
        }
    }

    int k, i1, i2, i3, j1, j2, j3;
    double elapsed_time;
    elapsed_time = clock();
    for (i = 0; i < N; i += 4)
    {
        i1 = i + 1;
        i2 = i + 2;
        i3 = i + 3;
        for (j = 0; j < N; j += 4)
        {
            j1 = j + 1;
            j2 = j + 2;
            j3 = j + 3;
            for (k = 0; k < N; k += 1)
            {
                M3[i][j] += M1[i][k] * M2[k][j];
                M3[i][j1] += M1[i][k] * M2[k][j1];
                M3[i][j2] += M1[i][k] * M2[k][j2];
                M3[i][j3] += M1[i][k] * M2[k][j3];

                M3[i1][j] += M1[i1][k] * M2[k][j];
                M3[i1][j1] += M1[i1][k] * M2[k][j1];
                M3[i1][j2] += M1[i1][k] * M2[k][j2];
                M3[i1][j3] += M1[i1][k] * M2[k][j3];

                M3[i2][j] += M1[i2][k] * M2[k][j];
                M3[i2][j1] += M1[i2][k] * M2[k][j1];
                M3[i2][j2] += M1[i2][k] * M2[k][j2];
                M3[i2][j3] += M1[i2][k] * M2[k][j3];

                M3[i3][j] += M1[i3][k] * M2[k][j];
                M3[i3][j1] += M1[i3][k] * M2[k][j1];
                M3[i3][j2] += M1[i3][k] * M2[k][j2];
                M3[i3][j3] += M1[i3][k] * M2[k][j3];
            }
        }
    }
    elapsed_time = clock() - elapsed_time;
    printf("elapsed time = %.6f sec\n", elapsed_time / CLOCKS_PER_SEC);
    assert_result(N, M3, EXPECTED);

    return 0;
}
