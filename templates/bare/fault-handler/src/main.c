/* Enable the UsageFault exception and the divide-by-zero / unaligned traps (SCB->CCR). Each fault is decoded from
 * CFSR and the stacked exception frame (PC of the faulting instruction); the handler then skips that 16-bit
 * instruction and returns, so main() continues and can check what was caught. */
#include "bare.h"

#define SCB_CCR   0xE000ED14u
#define SCB_SHCSR 0xE000ED24u
#define SCB_CFSR  0xE000ED28u
#define UFSR_UNALIGNED (1u << 24)
#define UFSR_DIVBYZERO (1u << 25)

static volatile uint32_t last_cfsr, last_pc, faults;

void fault_c(uint32_t *frame)                 /* frame: r0 r1 r2 r3 r12 lr pc xpsr */
{
  last_cfsr = REG32(SCB_CFSR);
  last_pc = frame[6];
  REG32(SCB_CFSR) = last_cfsr;                /* write-1-to-clear */
  faults++;
  uint16_t insn = *(uint16_t *)last_pc;       /* skip the faulting instruction (16 or 32 bit Thumb) */
  frame[6] += ((insn & 0xE000u) == 0xE000u && (insn & 0x1800u)) ? 4 : 2;
}

__attribute__((naked)) void UsageFault_Handler(void)
{
  __asm volatile("tst lr, #4\n ite eq\n mrseq r0, msp\n mrsne r0, psp\n b fault_c");
}

__attribute__((noinline)) static int divide(volatile int a, volatile int b) { return a / b; }
__attribute__((noinline)) static uint32_t load(volatile uint8_t *p) { return *(volatile uint32_t *)(p + 1); }

int main(void)
{
  static uint8_t buf[8] __attribute__((aligned(4)));
  REG32(SCB_SHCSR) |= 1u << 18;               /* USGFAULTENA */
  REG32(SCB_CCR) |= (1u << 4) | (1u << 3);    /* DIV_0_TRP, UNALIGN_TRP */
  (void)divide(1, 0);
  bare_printf("fault 1: CFSR=%08lx PC=%08lx\n", (unsigned long)last_cfsr, (unsigned long)last_pc);
  test_check(last_cfsr & UFSR_DIVBYZERO, "divide by zero trapped (CFSR.DIVBYZERO)");
  test_check(last_pc >= (uint32_t)divide && last_pc < (uint32_t)divide + 64, "stacked PC points into divide()");
  (void)load(buf);
  bare_printf("fault 2: CFSR=%08lx PC=%08lx\n", (unsigned long)last_cfsr, (unsigned long)last_pc);
  test_check(last_cfsr & UFSR_UNALIGNED, "unaligned word load trapped (CFSR.UNALIGNED)");
  test_check(faults == 2, "both faults handled and execution resumed");
  test_done();
}
