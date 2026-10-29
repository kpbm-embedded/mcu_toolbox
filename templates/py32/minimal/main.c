#include "board.h"

#if defined(PY32F071x8)
int main(void)                               /* PY32F071: HAL driver (the template has no LL driver for F07x) */
{
    HAL_Init();
    BSP_USART_Config();                      /* USART2 PA2/PA3, 115200 */
    printf("{{NAME}} on {{BOARD}}\r\n");
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
    BSP_RCC_HSI_24MConfig();
    BSP_USART_Config(115200);
    printf("{{NAME}} on {{BOARD}}\r\n");
    LL_IOP_GRP1_EnableClock(LED_CLK);
    LL_GPIO_SetPinMode(LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT);
    for (;;) {
        LL_GPIO_TogglePin(LED_PORT, LED_PIN);
        LL_mDelay(500);
    }
}
#endif
