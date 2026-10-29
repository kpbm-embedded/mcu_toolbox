/* CH32V003: USART1 TX on PD5 (default mapping) at 115200, HSI 24 MHz (reset clock, no PLL). CH32V003 RM. */
#include "bare.h"

#define RCC 0x40021000u
#define GPIOD 0x40011400u
#define USART1 0x40013800u
void chip_init(void)
{
  REG32(RCC + 0x18) |= (1u << 5) | (1u << 14);                            /* APB2PCENR: IOPDEN, USART1EN */
  REG32(GPIOD + 0x00) = (REG32(GPIOD + 0x00) & ~(0xFu << 20)) | (0xBu << 20);   /* CFGLR: PD5 AF push-pull */
  REG32(USART1 + 0x08) = 208;                                              /* BRR: 24 MHz / 115200 */
  REG32(USART1 + 0x0C) = (1u << 13) | (1u << 3);                           /* CTLR1: UE, TE */
}

void chip_putc(char c)
{
  while (!(REG32(USART1 + 0x00) & (1u << 7))) { }
  REG32(USART1 + 0x04) = (uint8_t)c;
}

void chip_exit(int code) { (void)code; for (;;) __asm volatile("wfi"); }
