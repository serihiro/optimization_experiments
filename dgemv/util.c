#include "util.h"

void assert_result(int n, double *v, double expected)
{
    for (int i = 0; i < n; ++i)
        assert(v[i] == expected);
}
