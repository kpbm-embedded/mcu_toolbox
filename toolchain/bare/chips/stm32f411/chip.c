/* STM32F411 (BlackPill): USART1 on PA9 (AF7) at 115200 from the 16 MHz HSI (reset clock). RM0383. */
#include "bare.h"

#define RCC 0x40023800u
#define GPIOA 0x40020000u
#define USART1 0x40011000u
void chip_init(void)
{
  REG32(RCC + 0x30) |= 1u << 0;                                   /* AHB1ENR.GPIOAEN */
  REG32(RCC + 0x44) |= 1u << 4;                                   /* APB2ENR.USART1EN */
  REG32(GPIOA + 0x00) = (REG32(GPIOA + 0x00) & ~(3u << 18)) | (2u << 18);   /* PA9 alternate function */
  REG32(GPIOA + 0x24) = (REG32(GPIOA + 0x24) & ~(0xFu << 4)) | (7u << 4);   /* AFRH: PA9 = AF7 */
  REG32(USART1 + 0x08) = 0x8B;                                    /* BRR: 16 MHz / 115200 */
  REG32(USART1 + 0x0C) = (1u << 13) | (1u << 3) | (1u << 2);      /* CR1: UE, TE, RE */
}

void chip_putc(char c)
{
  while (!(REG32(USART1 + 0x00) & (1u << 7))) { }   /* SR.TXE */
  REG32(USART1 + 0x04) = (uint8_t)c;
}

void chip_exit(int code) { (void)code; for (;;) __asm volatile("wfi"); }
