/* MPU region 0: whole address space, full access (background); region 1: 32 bytes at guard[] no access. A write
 * into guard[] (as a stack overflow would do) raises MemManage with MMARVALID and MMFAR = the address. */
#include "bare.h"

#define MPU_CTRL 0xE000ED94u
#define MPU_RNR  0xE000ED98u
#define MPU_RBAR 0xE000ED9Cu
#define MPU_RASR 0xE000EDA0u
#define SCB_SHCSR 0xE000ED24u
#define SCB_CFSR 0xE000ED28u
#define SCB_MMFAR 0xE000ED34u

static volatile uint32_t guard[8] __attribute__((aligned(32)));
static volatile uint32_t fault_addr, mm_faults;

void mm_c(uint32_t *frame)
{
  uint32_t cfsr = REG32(SCB_CFSR);
  if (cfsr & (1u << 7)) fault_addr = REG32(SCB_MMFAR);   /* MMARVALID */
  REG32(SCB_CFSR) = cfsr & 0xFFu;
  mm_faults++;
  uint16_t insn = *(uint16_t *)frame[6];
  frame[6] += ((insn & 0xE000u) == 0xE000u && (insn & 0x1800u)) ? 4 : 2;
}

__attribute__((naked)) void MemManage_Handler(void)
{
  __asm volatile("tst lr, #4\n ite eq\n mrseq r0, msp\n mrsne r0, psp\n b mm_c");
}

int main(void)
{
  REG32(MPU_RNR) = 0; REG32(MPU_RBAR) = 0;
  REG32(MPU_RASR) = (3u << 24) | (31u << 1) | 1u;                    /* 4 GB, AP=full, enable */
  REG32(MPU_RNR) = 1; REG32(MPU_RBAR) = (uint32_t)guard;
  REG32(MPU_RASR) = (1u << 28) | (0u << 24) | (4u << 1) | 1u;        /* 32 B, XN, no access, enable */
  REG32(SCB_SHCSR) |= 1u << 16;                                      /* MEMFAULTENA */
  REG32(MPU_CTRL) = 1;                                               /* enable, no PRIVDEFENA */
  __asm volatile("dsb\n isb");
  guard[3] = 0xDEAD;                                                 /* the "overflow" */
  REG32(MPU_CTRL) = 0;
  uint32_t base = (uint32_t)&guard[0];
  bare_printf("MemManage at %08lx (guard region %08lx..%08lx)\n", (unsigned long)fault_addr, (unsigned long)base,
              (unsigned long)(base + 31u));
  test_check(mm_faults == 1, "write into the guard region raised MemManage");
  test_check(fault_addr >= base && fault_addr < base + 32u && (fault_addr & 3u) == 0, "MMFAR holds an address inside the guard region");
  test_check(guard[3] == 0, "the store did not happen");
  test_done();
}
