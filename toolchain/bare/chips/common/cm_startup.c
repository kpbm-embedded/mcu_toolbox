/* Cortex-M startup shared by the Arm chips: vector table (16 system + CHIP_NUM_IRQ entries), .data/.bss init,
 * VTOR, chip_init(), C++/constructor init, main(). Every peripheral IRQ calls bare_irq_handler(irq); the system
 * handlers are weak, so an app defines SysTick_Handler, HardFault_Handler, ... by name to take them over. */
#include <stdint.h>
#include "bare.h"

#ifndef CHIP_NUM_IRQ
#error "CHIP_NUM_IRQ must be defined by chips/<chip>/chip.cmake"
#endif

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
extern void __libc_init_array(void);

void Reset_Handler(void);
void Default_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak));
void MemManage_Handler(void) __attribute__((weak, alias("HardFault_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("HardFault_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("HardFault_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
static void IRQ_Dispatch(void);

__attribute__((section(".isr_vector"), used)) void (*const bare_vectors[16 + CHIP_NUM_IRQ])(void) = {
  (void (*)(void))&_estack, Reset_Handler, NMI_Handler, HardFault_Handler, MemManage_Handler, BusFault_Handler,
  UsageFault_Handler, 0, 0, 0, 0, SVC_Handler, DebugMon_Handler, 0, PendSV_Handler, SysTick_Handler,
  [16 ... 16 + CHIP_NUM_IRQ - 1] = IRQ_Dispatch,
};

void Reset_Handler(void)
{
  uint32_t *src = &_sidata, *dst = &_sdata;
  while (dst < &_edata) *dst++ = *src++;
  for (dst = &_sbss; dst < &_ebss;) *dst++ = 0;
  REG32(0xE000ED08u) = (uint32_t)bare_vectors;   /* VTOR */
#if defined(__ARM_FP)
  REG32(0xE000ED88u) |= 0xFu << 20;               /* CPACR: full access to CP10/CP11 (FPU) */
  __asm volatile("dsb\n isb");
#endif
  chip_init();
  __libc_init_array();
  chip_exit(main());
}

void Default_Handler(void)
{
  uint32_t ipsr;
  __asm volatile("mrs %0, ipsr" : "=r"(ipsr));
  bare_printf("unexpected exception %lu\nRESULT: FAIL (exception)\n", (unsigned long)ipsr);
  chip_exit(1);
}

static void IRQ_Dispatch(void)
{
  uint32_t ipsr;
  __asm volatile("mrs %0, ipsr" : "=r"(ipsr));
  bare_irq_handler((int)ipsr - 16);
}

void HardFault_Handler(void)
{
  bare_printf("HardFault: HFSR=%08lx CFSR=%08lx\nRESULT: FAIL (fault)\n", (unsigned long)REG32(0xE000ED2Cu),
              (unsigned long)REG32(0xE000ED28u));
  chip_exit(1);
}
