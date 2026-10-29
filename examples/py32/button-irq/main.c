#include "board.h"

/* EXTI lines 4..15 share one vector on PY32F0xx (EXTI0_1, EXTI2_3, EXTI4_15); PA12 is line 12. */
void EXTI4_15_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag(KEY_LINE)) {
        LL_GPIO_TogglePin(LED_PORT, LED_PIN);
        LL_EXTI_ClearFlag(KEY_LINE);
    }
}

int main(void)
{
    BSP_RCC_HSI_24MConfig();
    LL_IOP_GRP1_EnableClock(LED_CLK | KEY_CLK);
    LL_GPIO_SetPinMode(LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT);

    LL_GPIO_InitTypeDef gi = {0};
    gi.Pin = KEY_PIN; gi.Mode = LL_GPIO_MODE_INPUT; gi.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(KEY_PORT, &gi);

    LL_EXTI_InitTypeDef ei = {0};
    ei.Line = KEY_LINE; ei.LineCommand = ENABLE; ei.Mode = LL_EXTI_MODE_IT; ei.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&ei);
    NVIC_SetPriority(EXTI4_15_IRQn, 0);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
    for (;;) __WFI();
}
