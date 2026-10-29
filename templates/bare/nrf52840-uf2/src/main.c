/* {{NAME}}: blink the blue LED (P0.15 on the nice!nano) and print on UART0 (TX P0.06). */
#include "bare.h"
#define P0 0x50000000u

int main(void)
{
  REG32(P0 + 0x514) = 1u << 15;            /* DIRSET: P0.15 output */
  for (unsigned n = 0;; n++) {
    REG32(P0 + ((n & 1) ? 0x508 : 0x50C)) = 1u << 15;   /* OUTSET / OUTCLR */
    bare_printf("{{NAME}} %u\n", n);
    for (volatile int i = 0; i < 800000; i++) { }
  }
}
