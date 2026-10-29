/* printf over USART1 (PA9 TX, PA10 RX, 115200 8N1) plus a polled echo of received bytes. */
#include "board.h"

int main(void)
{
  board_init();
  printf("\r\nhello from %s, SYSCLK %lu Hz\r\n", BOARD_NAME, (unsigned long)HAL_RCC_GetSysClockFreq());
  uint32_t next = HAL_GetTick() + 1000, n = 0;
  for (;;) {
    uint8_t c;
    if (HAL_UART_Receive(&board_uart, &c, 1, 10) == HAL_OK) {   /* echo */
      HAL_UART_Transmit(&board_uart, &c, 1, 100);
      if (c == '\r') HAL_UART_Transmit(&board_uart, (uint8_t *)"\n", 1, 100);
      board_led_toggle();
    }
    if ((int32_t)(HAL_GetTick() - next) >= 0) { next += 1000; printf("uptime %lu s, n=%lu\r\n", (unsigned long)(HAL_GetTick() / 1000), (unsigned long)n++); }
    /* a framing/overrun error latches in the HAL; clear it so RX keeps working */
    __HAL_UART_CLEAR_OREFLAG(&board_uart);
  }
}
