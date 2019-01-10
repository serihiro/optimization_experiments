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

    int k;
    double elapsed_time;
    elapsed_time = clock();
    for (i = 0; i < N; ++i)
    {
        for (k = 0; k < N; ++k)
        {
            for (j = 0; j < N; ++j)
            {
                M3[i][j] += M1[i][k] * M2[k][j];
            }
        }
    }
    elapsed_time = clock() - elapsed_time;
    printf("elapsed time = %.6f sec\n", elapsed_time / CLOCKS_PER_SEC);
    assert_result(N, M3, EXPECTED);

    return 0;
}
