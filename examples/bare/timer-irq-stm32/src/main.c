/* TIM2 at 1 kHz update rate (prescaler to 1 MHz, ARR 999), update interrupt -> NVIC IRQ 28 -> bare_irq_handler.
 * After 50 interrupts the timer is stopped: no further interrupts may arrive while SysTick keeps counting.
 * (Emulators do not keep the two clocks in a fixed ratio, so the elapsed SysTick time is only printed.) */
#include "bare.h"

#if defined(CHIP_STM32F103)
#define TIMCLK 8000000u
#define RCC_APB1ENR 0x4002101Cu
#else
#define TIMCLK 16000000u
#define RCC_APB1ENR 0x40023840u
#endif
#define TIM2 0x40000000u
#define TIM2_IRQ 28
static volatile uint32_t tim_irqs, ms;

void SysTick_Handler(void) { ms++; }

void bare_irq_handler(int irq)
{
  if (irq == TIM2_IRQ) { REG32(TIM2 + 0x10) = 0; tim_irqs++; return; }   /* SR: clear UIF */
  bare_printf("unexpected IRQ %d\n", irq);
}

int main(void)
{
  REG32(RCC_APB1ENR) |= 1u;                     /* TIM2EN */
  REG32(TIM2 + 0x28) = TIMCLK / 1000000u - 1u;  /* PSC: 1 MHz */
  REG32(TIM2 + 0x2C) = 999;                     /* ARR: 1 kHz */
  REG32(TIM2 + 0x14) = 1;                       /* EGR.UG: load PSC */
  REG32(TIM2 + 0x10) = 0;
  REG32(TIM2 + 0x0C) = 1;                       /* DIER.UIE */
  REG32(0xE000E100u) = 1u << TIM2_IRQ;          /* NVIC ISER0 */
  REG32(0xE000E014u) = TIMCLK / 1000u - 1u;     /* SysTick 1 ms as the reference clock */
  REG32(0xE000E018u) = 0;
  REG32(0xE000E010u) = 0x7;
  REG32(TIM2 + 0x00) = 1;                       /* CR1.CEN */
  while (tim_irqs < 50) __asm volatile("wfi");
  REG32(TIM2 + 0x00) = 0;                       /* CEN off */
  uint32_t stopped_at = tim_irqs, t0 = ms;
  while (ms < t0 + 20) __asm volatile("wfi");
  REG32(0xE000E010u) = 0;
  bare_printf("50 TIM2 interrupts in %lu ms of SysTick time\n", (unsigned long)t0);
  test_check(stopped_at >= 50, "TIM2 update interrupts arrived through the NVIC");
  test_check(tim_irqs == stopped_at, "no TIM2 interrupt after CEN was cleared (20 ms of SysTick)");
  test_check(t0 > 0, "SysTick ran alongside");
  test_done();
}
