/* {{NAME}} on {{TARGET}}: LED {{LED_PIN}} flashes SOS; steps come from src/logic.c (unit-tested on the host in tests/). */
#include "board.h"
#include "logic.h"

int main(void)
{
  board_init();
  printf("%s on %s: SOS pattern, %u steps\r\n", "{{NAME}}", BOARD_NAME, sos_len());
  for (unsigned step = 0;; step++) {
    board_led_set(sos_led_on(step));
    HAL_Delay(sos_step_ms(step));
  }
}
