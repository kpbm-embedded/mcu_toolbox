/* TIM2 update interrupt at 1 Hz (10 kHz counter, ARR 9999) toggles the LED; main prints the tick count. */
#include "board.h"

static TIM_HandleTypeDef htim2;
static volatile uint32_t ticks;

void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *h)
{
  if (h->Instance == TIM2) { ticks++; board_led_toggle(); }
}

int main(void)
{
  board_init();
  __HAL_RCC_TIM2_CLK_ENABLE();
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = BOARD_TIM_APB1_HZ / 10000U - 1U;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);
  HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  HAL_TIM_Base_Start_IT(&htim2);
  printf("timer on %s: TIM2 1 Hz\r\n", BOARD_NAME);
  uint32_t seen = 0;
  for (;;) {
    if (ticks != seen) { seen = ticks; printf("tick %lu\r\n", (unsigned long)seen); }
    __WFI();
  }
}
