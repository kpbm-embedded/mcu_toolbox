/* STM32H743: USART1 on PA9 (AF7) at 115200 from the 64 MHz HSI (reset clock). RM0433. */
#include "bare.h"

#define RCC 0x58024400u
#define GPIOA 0x58020000u
#define USART1 0x40011000u
void chip_init(void)
{
  REG32(RCC + 0xE0) |= 1u << 0;                                   /* AHB4ENR.GPIOAEN */
  REG32(RCC + 0xF0) |= 1u << 4;                                   /* APB2ENR.USART1EN */
  REG32(GPIOA + 0x00) = (REG32(GPIOA + 0x00) & ~(3u << 18)) | (2u << 18);
  REG32(GPIOA + 0x24) = (REG32(GPIOA + 0x24) & ~(0xFu << 4)) | (7u << 4);
  REG32(USART1 + 0x0C) = 556;                                     /* BRR: 64 MHz / 115200 */
  REG32(USART1 + 0x00) = (1u << 3) | (1u << 2) | 1u;              /* CR1: TE, RE, UE */
}

void chip_putc(char c)
{
  while (!(REG32(USART1 + 0x1C) & (1u << 7))) { }   /* ISR.TXE */
  REG32(USART1 + 0x28) = (uint8_t)c;                /* TDR */
}

void chip_exit(int code) { (void)code; for (;;) __asm volatile("wfi"); }
