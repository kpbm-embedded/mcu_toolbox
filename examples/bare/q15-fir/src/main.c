/* 8-tap low-pass FIR in Q15 (int16 samples and coefficients, 32-bit accumulator, rounding) compared with the same
 * filter in float. The Q15 result must stay within 2 LSB of the float one for a full-scale chirp. */
#include <math.h>
#include "bare.h"

#define TAPS 8
#define N 128
static const float coef_f[TAPS] = { 0.02f, 0.06f, 0.14f, 0.28f, 0.28f, 0.14f, 0.06f, 0.02f };

int main(void)
{
  int16_t coef_q[TAPS], x[N];
  for (int k = 0; k < TAPS; k++) coef_q[k] = (int16_t)lrintf(coef_f[k] * 32768.0f);
  for (int n = 0; n < N; n++) x[n] = (int16_t)lrintf(30000.0f * sinf(0.002f * (float)(n * n)));
  int max_err = 0;
  for (int n = TAPS - 1; n < N; n++) {
    int32_t acc = 1 << 14;                           /* rounding */
    float ref = 0.0f;
    for (int k = 0; k < TAPS; k++) { acc += (int32_t)coef_q[k] * x[n - k]; ref += coef_f[k] * (float)x[n - k]; }
    int err = (int)(acc >> 15) - (int)lrintf(ref);
    if (err < 0) err = -err;
    if (err > max_err) max_err = err;
  }
  bare_printf("max |q15 - float| = %d LSB over %d outputs\n", max_err, N - TAPS + 1);
#if defined(__ARM_FP)
  test_check(1, "float reference ran on the FPU (hard-float build)");
#endif
  test_check(max_err <= 2, "Q15 filter within 2 LSB of the float filter");
  test_done();
}
