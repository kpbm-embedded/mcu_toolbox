/* Stop mode with EXTI wakeup: the MCU blinks 3 times, prints, enters Stop (clocks off, SRAM and registers kept, regulator in
 * low-power mode) and wakes on the user button edge. After wakeup the system runs from HSI, so the clock tree is restored
 * with board_clock_init() before the console is used again.
 * Boards without a button in the board layer (bluepill-f103): PA0, active high with internal pull-down, EXTI0 rising edge. */
#include "board.h"

static volatile uint32_t wakeups;

void board_button_hook(void) { wakeups++; }   /* EXTI ISR: only counts, the wake itself is the point */

#if !BOARD_HAS_BUTTON
static void wake_button_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_0, .Mode = GPIO_MODE_IT_RISING, .Pull = GPIO_PULLDOWN, .Speed = GPIO_SPEED_FREQ_LOW };
  HAL_GPIO_Init(GPIOA, &g);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
void EXTI0_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0); }
void HAL_GPIO_EXTI_Callback(uint16_t pin) { if (pin == GPIO_PIN_0) board_button_hook(); }
#else
static void wake_button_init(void) { board_button_init(1); }
#endif

static void enter_stop(void)
{
  while (__HAL_UART_GET_FLAG(&board_uart, UART_FLAG_TC) == RESET) {}   /* let the last console byte leave */
  board_led_set(0);
  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  /* woken up: running on HSI (F1/F4/G4: HSI 8/16 MHz, H7: HSI 64 MHz); PLL and HSE are off */
  board_clock_init();
  HAL_ResumeTick();
}

int main(void)
{
  board_init();
  wake_button_init();
  printf("low-power on %s: Stop mode, wake with the user button\r\n", BOARD_NAME);
  for (uint32_t n = 1;; n++) {
    for (int i = 0; i < 6; i++) { board_led_toggle(); HAL_Delay(150); }
    printf("cycle %lu: entering Stop (SYSCLK %lu Hz)\r\n", (unsigned long)n, (unsigned long)HAL_RCC_GetSysClockFreq());
    enter_stop();
    printf("woke up (button events %lu), SYSCLK %lu Hz\r\n", (unsigned long)wakeups, (unsigned long)HAL_RCC_GetSysClockFreq());
  }
}
