/* {{NAME}}: start here. chip_init() already set up the console (chips/<chip>/chip.c); main() returns into
 * chip_exit(). Keep test_check()/test_done() for emulator runs, or loop forever on a board. */
#include "bare.h"

int main(void)
{
  bare_printf("{{NAME}} on %s\n", BARE_CHIP);
  test_check(1 + 1 == 2, "replace with your checks");
  test_done();
}
