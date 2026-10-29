/* The application, started by the bootloader. */
#include "bare.h"

int main(void)
{
  uint32_t vtor = REG32(0xE000ED08u);
  bare_printf("app: running, VTOR=%08lx\n", (unsigned long)vtor);
  test_check(vtor == 0x00010100u, "vector table relocated to the app");
  test_check((uint32_t)&main > 0x00010100u, "app code runs from its slot");
  test_check(*(const uint32_t *)0x00010000u == 0x31505041u, "image header 'APP1' in front of the app");
  test_done();
}
