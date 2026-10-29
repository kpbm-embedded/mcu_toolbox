/* .noinit is not touched by the startup code, so it keeps its value across a reset (not a power cycle). First boot:
 * no magic -> store it, count = 1, request a system reset (AIRCR.SYSRESETREQ). Second boot: count = 2 -> checks.
 * ramfunc() lives in .ramfunc: copied to RAM with .data and executed from there. */
#include "bare.h"

#define MAGIC 0xB007C0DEu
static uint32_t boot_magic __attribute__((section(".noinit")));
static uint32_t boot_count __attribute__((section(".noinit")));

__attribute__((section(".ramfunc"), noinline)) static uint32_t ramfunc(uint32_t x) { return x * 3u + 1u; }

int main(void)
{
  if (boot_magic != MAGIC) { boot_magic = MAGIC; boot_count = 0; }
  boot_count++;
  bare_printf("boot %lu\n", (unsigned long)boot_count);
  if (boot_count == 1) {
    REG32(0xE000ED0Cu) = (0x05FAu << 16) | (1u << 2);   /* AIRCR: VECTKEY | SYSRESETREQ */
    for (;;) { }
  }
  uint32_t addr = (uint32_t)&ramfunc;
  bare_printf("ramfunc at %08lx\n", (unsigned long)addr);
  test_check(boot_count == 2, ".noinit kept the boot counter across the reset");
  test_check(addr >= 0x20000000u && addr < 0x20400000u, "ramfunc() runs from RAM");
  test_check(ramfunc(4) == 13, "ramfunc() computes from RAM");
  test_done();
}
