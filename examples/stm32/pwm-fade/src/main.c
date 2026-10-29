/* TIM3 CH1 -> PA6 PWM, 1 kHz, triangle duty ramp. Wire an LED (+ resistor to GND) to PA6; the on-board LEDs are not on a PWM pin. */
#include "board.h"

int main(void)
{
  board_init();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM3_CLK_ENABLE();
  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_6; g.Mode = GPIO_MODE_AF_PP; g.Pull = GPIO_NOPULL; g.Speed = GPIO_SPEED_FREQ_LOW;
#if !defined(BOARD_F1)
  g.Alternate = GPIO_AF2_TIM3;
#endif
  HAL_GPIO_Init(GPIOA, &g);

  TIM_HandleTypeDef t = {0};
  t.Instance = TIM3;
  t.Init.Prescaler = BOARD_TIM_APB1_HZ / 1000000U - 1U;     /* 1 MHz counter */
  t.Init.CounterMode = TIM_COUNTERMODE_UP;
  t.Init.Period = 999;                                      /* 1 kHz */
  t.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  t.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_PWM_Init(&t);
  TIM_OC_InitTypeDef oc = {0};
  oc.OCMode = TIM_OCMODE_PWM1; oc.Pulse = 0; oc.OCPolarity = TIM_OCPOLARITY_HIGH; oc.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&t, &oc, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&t, TIM_CHANNEL_1);
  printf("pwm-fade on %s: TIM3_CH1 PA6, 1 kHz\r\n", BOARD_NAME);

  int duty = 0, step = 5;
  for (;;) {
    __HAL_TIM_SET_COMPARE(&t, TIM_CHANNEL_1, (uint32_t)duty);
    duty += step;
    if (duty >= 1000) { duty = 1000; step = -5; } else if (duty <= 0) { duty = 0; step = 5; }
    HAL_Delay(5);
  }
}
