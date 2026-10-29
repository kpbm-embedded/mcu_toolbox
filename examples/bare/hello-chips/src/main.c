/* One source for seven chips: console output, .data/.bss initialisation and a printf, checked in the emulator. */
#include <stdio.h>
#include <string.h>
#include "bare.h"

static int initialised = 42;     /* .data: copied from flash by the startup code */
static int zeroed;               /* .bss: cleared by the startup code */

int main(void)
{
  char buf[32];
  bare_printf("hello from %s\n", BARE_CHIP);
  test_check(initialised == 42, ".data initialised from flash");
  test_check(zeroed == 0, ".bss cleared");
  int n = snprintf(buf, sizeof buf, "%d-%s", 7, "ok");
  test_check(n == 4 && strcmp(buf, "7-ok") == 0, "newlib-nano snprintf works");
  test_done();
}
