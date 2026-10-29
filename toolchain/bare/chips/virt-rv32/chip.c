/* qemu riscv virt: 16550 UART console, SiFive test device ("finisher") for the exit status. */
#include "bare.h"

#define UART 0x10000000u
#define FINISHER 0x00100000u
void chip_init(void) { }

void chip_putc(char c)
{
  while (!(*(volatile uint8_t *)(UART + 5) & 0x20)) { }   /* LSR.THRE */
  *(volatile uint8_t *)UART = (uint8_t)c;
}

void chip_exit(int code)
{
  REG32(FINISHER) = code == 0 ? 0x5555u : (((uint32_t)code << 16) | 0x3333u);   /* pass / fail(code) */
  for (;;) { }
}
