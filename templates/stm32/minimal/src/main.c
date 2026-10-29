/* {{NAME}} on {{TARGET}}: LED {{LED_PIN}} blinks at 2 Hz, "hello" on USART1 (PA9 TX / PA10 RX, 115200 8N1).
 * SYSCLK {{SYSCLK_MHZ}} MHz. Board layer (clock, pins): mcu/stm32/boards/{{TARGET}}/. */
#include "board.h"

int main(void)
{
  board_init();
  printf("%s up on %s, SYSCLK %lu Hz\r\n", "{{NAME}}", BOARD_NAME, (unsigned long)HAL_RCC_GetSysClockFreq());
  for (unsigned n = 0;; n++) {
    board_led_toggle();
    if (!(n & 1)) printf("tick %u\r\n", n / 2);
    HAL_Delay(250);
  }
}
