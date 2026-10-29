#include "board.h"

#define PERIOD 1000   /* 24 MHz / 24 / 1000 = 1 kHz */

int main(void)
{
    BSP_RCC_HSI_24MConfig();
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_GPIO_InitTypeDef gi = {0};
    gi.Pin = PWM_PIN; gi.Mode = LL_GPIO_MODE_ALTERNATE; gi.Alternate = PWM_AF;
    LL_GPIO_Init(PWM_PORT, &gi);

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
    LL_TIM_InitTypeDef ti = {0};
    ti.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1; ti.CounterMode = LL_TIM_COUNTERMODE_UP;
    ti.Prescaler = 24 - 1; ti.Autoreload = PERIOD - 1;
    LL_TIM_Init(TIM1, &ti);

    LL_TIM_OC_InitTypeDef oc = {0};
    oc.OCMode = LL_TIM_OCMODE_PWM1; oc.OCState = LL_TIM_OCSTATE_ENABLE;
    oc.OCPolarity = LL_TIM_OCPOLARITY_HIGH; oc.OCIdleState = LL_TIM_OCIDLESTATE_LOW; oc.CompareValue = 0;
    LL_TIM_OC_Init(TIM1, PWM_CH, &oc);
    LL_TIM_EnableAllOutputs(TIM1);
    LL_TIM_EnableCounter(TIM1);

    int duty = 0, step = 10;
    for (;;) {
        PWM_SET(duty);
        duty += step;
        if (duty >= PERIOD || duty <= 0) step = -step;
        LL_mDelay(10);
    }
}
