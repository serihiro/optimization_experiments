#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"
#include "data.h"

int main()
{
    int i, j;
    for (i = 0; i < N; ++i)
    {
        V1[i] = 3.0;
        for (j = 0; j < N; ++j)
        {
            M1[i][j] = 2.0;
        }
    }

    double elapsed_time;
    elapsed_time = clock();
    for (i = 0; i < N; ++i)
    {
        for (j = 0; j < N; ++j)
        {
            V2[j] += M1[i][j] * V1[j];
        }
    }
    elapsed_time = clock() - elapsed_time;
    printf("elapsed time = %.6f sec\n", elapsed_time / CLOCKS_PER_SEC);

    assert_result(N, V2, EXPECTED);

    return 0;
}
