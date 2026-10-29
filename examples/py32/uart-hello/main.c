#include "board.h"

#if defined(PY32F071x8)
int main(void)                               /* HAL + BSP: USART2 PA2 TX / PA3 RX, 115200 8N1 */
{
    HAL_Init();
    BSP_USART_Config();
    printf("hello from PY32F071, SystemCoreClock %lu Hz\r\n", (unsigned long)SystemCoreClock);
    unsigned n = 0;
    for (;;) {
        uint8_t c;
        if (HAL_UART_Receive(&DebugUartHandle, &c, 1, 1000) == HAL_OK) printf("echo: %c\r\n", c);
        else printf("alive %u\r\n", ++n);
    }
}
#else
int main(void)
{
    BSP_RCC_HSI_24MConfig();
    BSP_USART_Config(115200);
    printf("hello from PY32, SystemCoreClock %lu Hz\r\n", (unsigned long)SystemCoreClock);
    unsigned n = 0, ms = 0;
    for (;;) {
        if (LL_USART_IsActiveFlag_RXNE(DEBUG_USART)) {
            uint8_t c = LL_USART_ReceiveData8(DEBUG_USART);
            printf("echo: %c\r\n", c);
        }
        LL_mDelay(1);
        if (++ms >= 1000) { ms = 0; printf("alive %u\r\n", ++n); }
    }
}
#endif
