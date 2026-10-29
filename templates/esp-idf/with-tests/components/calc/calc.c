#include "calc.h"

int calc_clamp(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

int calc_sum(const int *a, int n)
{
    int s = 0;
    for (int i = 0; i < n; i++) s += a[i];
    return s;
}
