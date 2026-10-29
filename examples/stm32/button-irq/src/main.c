/* User button on EXTI toggles the LED; main prints the press count. The ISR only sets a flag (50 ms debounce by SysTick).
 * Boards without a button in the board layer (bluepill-f103): PA0, active high with internal pull-down, EXTI0 rising edge
 * (BluePill+: on-board KEY, unverified; plain BluePill: wire a push button from PA0 to 3V3). */
#include "board.h"

static volatile uint32_t presses, last_ms;
static volatile uint8_t pending;

void board_button_hook(void)    /* called by the board layer from the EXTI ISR */
{
  uint32_t now = HAL_GetTick();
  if (now - last_ms < 50) return;
  last_ms = now;
  board_led_toggle();
  presses++;
  pending = 1;
}

#if !BOARD_HAS_BUTTON
static void ext_button_init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_0, .Mode = GPIO_MODE_IT_RISING, .Pull = GPIO_PULLDOWN, .Speed = GPIO_SPEED_FREQ_LOW };
  HAL_GPIO_Init(GPIOA, &g);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
void EXTI0_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0); }
void HAL_GPIO_EXTI_Callback(uint16_t pin) { if (pin == GPIO_PIN_0) board_button_hook(); }
#endif

int main(void)
{
  board_init();
#if BOARD_HAS_BUTTON
  board_button_init(1);
#else
  ext_button_init();
#endif
  printf("button-irq on %s: press the user button\r\n", BOARD_NAME);
  for (;;) {
    if (pending) { pending = 0; printf("press #%lu\r\n", (unsigned long)presses); }
    __WFI();
  }
}
