/* mps2-an385 (qemu): CMSDK APB UART0 console, semihosting SYS_EXIT. */
#include "bare.h"

#define UART0 0x40004000u
void chip_init(void)
{
  REG32(UART0 + 0x10) = 16;   /* BAUDDIV (ignored by qemu) */
  REG32(UART0 + 0x08) = 0x3;  /* CTRL: TX + RX enable */
}

void chip_putc(char c)
{
  while (REG32(UART0 + 0x04) & 1u) { }   /* STATE.TXFULL */
  REG32(UART0) = (uint8_t)c;
}

void chip_exit(int code)
{
  register uint32_t r0 __asm("r0") = 0x18;                              /* SYS_EXIT */
  register uint32_t r1 __asm("r1") = code == 0 ? 0x20026u : 0x20023u;   /* ApplicationExit -> 0, else 1 */
  __asm volatile("bkpt 0xAB" : : "r"(r0), "r"(r1) : "memory");
  for (;;) { }
}
