/* {{NAME}}: talks to the CMSDK UART of mps2-an385 directly. */
#include <stdint.h>
#define UART0 0x40004000u
static void putc_(char c) { while (*(volatile uint32_t *)(UART0 + 4) & 1u) { } *(volatile uint32_t *)UART0 = (uint8_t)c; }
static void puts_(const char *s) { while (*s) putc_(*s++); }
static int counter = 5;   /* .data */
static int zero;          /* .bss */

int main(void)
{
  *(volatile uint32_t *)(UART0 + 8) = 1;   /* TX enable */
  int ok = counter == 5 && zero == 0;
  puts_(ok ? "own startup: sections ok\nRESULT: PASS\n" : "own startup: sections wrong\nRESULT: FAIL\n");
  return ok ? 0 : 1;
}
