/* nRF52840: UART0 (legacy, non-DMA registers) TX on P0.06 at 115200. nRF52840 PS v1.7, section UART. */
#include "bare.h"

#define UART0 0x40002000u
void chip_init(void)
{
  REG32(UART0 + 0x50C) = 6;            /* PSEL.TXD = P0.06 */
  REG32(UART0 + 0x524) = 0x01D7E000u;  /* BAUDRATE 115200 */
  REG32(UART0 + 0x500) = 4;            /* ENABLE = UART */
  REG32(UART0 + 0x008) = 1;            /* TASKS_STARTTX */
}

void chip_putc(char c)
{
  REG32(UART0 + 0x11C) = 0;            /* EVENTS_TXDRDY */
  REG32(UART0 + 0x51C) = (uint8_t)c;   /* TXD */
  while (REG32(UART0 + 0x11C) == 0) { }
}

void chip_exit(int code) { (void)code; for (;;) __asm volatile("wfi"); }
