/* SysTick fires every 1 ms (reload from the core clock); main sleeps with WFI between ticks. Every 10th tick the
 * SysTick handler pends PendSV (lowest priority): PendSV must run after SysTick returns, never nested in it. */
#include "bare.h"

#ifndef CORE_HZ
#if defined(CHIP_MPS2_AN385)
#define CORE_HZ 25000000u
#elif defined(CHIP_STM32F103)
#define CORE_HZ 8000000u
#elif defined(CHIP_NRF52840)
#define CORE_HZ 64000000u
#else
#define CORE_HZ 16000000u
#endif
#endif
#define SYST_CSR 0xE000E010u
#define SYST_RVR 0xE000E014u
#define SYST_CVR 0xE000E018u
#define SCB_ICSR 0xE000ED04u
#define SCB_SHPR3 0xE000ED20u

static volatile uint32_t ticks, pendsv_runs, nested;
static volatile int in_systick;

void SysTick_Handler(void)
{
  in_systick = 1;
  ticks++;
  if (ticks % 10 == 0) REG32(SCB_ICSR) = 1u << 28;   /* PENDSVSET */
  in_systick = 0;
}

void PendSV_Handler(void)
{
  if (in_systick) nested++;
  pendsv_runs++;
}

int main(void)
{
  REG32(SCB_SHPR3) = (0xFFu << 16) | (0x40u << 24);   /* PendSV lowest, SysTick higher */
  REG32(SYST_RVR) = CORE_HZ / 1000u - 1u;
  REG32(SYST_CVR) = 0;
  REG32(SYST_CSR) = 0x7;                               /* processor clock, interrupt, enable */
  while (ticks < 100) __asm volatile("wfi");
  REG32(SYST_CSR) = 0;
  bare_printf("ticks %lu, PendSV %lu, nested %lu\n", (unsigned long)ticks, (unsigned long)pendsv_runs, (unsigned long)nested);
  test_check(ticks >= 100, "100 SysTick interrupts woke the WFI loop");
  test_check(pendsv_runs == 10, "PendSV ran once per 10 ticks");
  test_check(nested == 0, "PendSV never preempted SysTick (priorities)");
  test_done();
}
