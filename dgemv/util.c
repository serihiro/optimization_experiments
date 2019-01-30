#include "util.h"
#include <stdio.h>

void assert_result(int n, double *v, double expected)
{
    for (int i = 0; i < n; ++i)
    {
        if (v[i] != expected)
        {
            printf("%d, %f\n", i, v[i]);
        }
        assert(v[i] == expected);
    }
}
