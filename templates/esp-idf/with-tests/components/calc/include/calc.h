#pragma once

/* Pure logic kept free of ESP-IDF headers so it can be tested on the host. */
int calc_clamp(int v, int lo, int hi);
int calc_sum(const int *a, int n);
