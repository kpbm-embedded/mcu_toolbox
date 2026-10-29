/* Firmware side: the same index walk as src/ringidx.bend, printed for test.sh to compare with Bend's result. */
#include "bare.h"

static uint32_t next(uint32_t i, uint32_t size) { return (i + 1u) % size; }

int main(void)
{
  uint32_t i = 0;
  for (uint32_t s = 0; s < 1000; s++) i = next(i, 8);
  bare_printf("walk(0, 8, 1000) = %lu\n", (unsigned long)i);
  test_check(i == 1000u % 8u, "index walk wraps correctly");
  test_done();
}
