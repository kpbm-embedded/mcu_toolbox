#include "board.h"

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM1)) {
        LL_TIM_ClearFlag_UPDATE(TIM1);
        LL_GPIO_TogglePin(LED_PORT, LED_PIN);
    }
}

int main(void)
{
    BSP_RCC_HSI_24MConfig();
    LL_IOP_GRP1_EnableClock(LED_CLK);
    LL_GPIO_SetPinMode(LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT);

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
    LL_TIM_InitTypeDef ti = {0};
    ti.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1; ti.CounterMode = LL_TIM_COUNTERMODE_UP;
    ti.Prescaler = 24000 - 1;      /* 24 MHz / 24000 = 1 kHz */
    ti.Autoreload = 500 - 1;       /* 500 ms period -> LED toggles every 500 ms */
    LL_TIM_Init(TIM1, &ti);
    LL_TIM_EnableIT_UPDATE(TIM1);
    NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0);
    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    LL_TIM_EnableCounter(TIM1);
    for (;;) __WFI();
}
