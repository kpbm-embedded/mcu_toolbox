/* STM32F103 (BluePill): USART1 TX on PA9 (AF push-pull) at 115200 from the 8 MHz HSI. RM0008. */
#include "bare.h"

#define RCC 0x40021000u
#define GPIOA 0x40010800u
#define USART1 0x40013800u
void chip_init(void)
{
  REG32(RCC + 0x18) |= (1u << 2) | (1u << 14);                         /* APB2ENR: IOPAEN, USART1EN */
  REG32(GPIOA + 0x04) = (REG32(GPIOA + 0x04) & ~(0xFu << 4)) | (0xBu << 4);   /* CRH: PA9 AF push-pull 50 MHz */
  REG32(USART1 + 0x08) = 69;                                            /* BRR: 8 MHz / 115200 */
  REG32(USART1 + 0x0C) = (1u << 13) | (1u << 3) | (1u << 2);            /* CR1: UE, TE, RE */
}

void chip_putc(char c)
{
  while (!(REG32(USART1 + 0x00) & (1u << 7))) { }
  REG32(USART1 + 0x04) = (uint8_t)c;
}

void chip_exit(int code) { (void)code; for (;;) __asm volatile("wfi"); }
