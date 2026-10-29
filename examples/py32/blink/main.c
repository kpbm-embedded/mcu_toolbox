#include "board.h"

#if defined(PY32F071x8)
int main(void)                               /* HAL: HAL_Init sets up the 1 ms SysTick (HSI 8 MHz after reset) */
{
    HAL_Init();
    LED_CLK_ENABLE();
    GPIO_InitTypeDef gi = { .Pin = LED_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW };
    HAL_GPIO_Init(LED_PORT, &gi);
    for (;;) {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        HAL_Delay(500);
    }
}
#else
int main(void)
{
    BSP_RCC_HSI_24MConfig();                 /* 24 MHz HSI, also sets up SysTick for LL_mDelay */
    LL_IOP_GRP1_EnableClock(LED_CLK);
    LL_GPIO_SetPinMode(LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT);
    for (;;) {
        LL_GPIO_TogglePin(LED_PORT, LED_PIN);
        LL_mDelay(500);
    }
}
#endif
