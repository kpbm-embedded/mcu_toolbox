#include "debug.h"

/* BluePill Plus CH32V203: user LED PB2 (active high), source: Zephyr boards/weact/bluepillplus_ch32v203.dts */
#define LED_PORT GPIOB
#define LED_PINN GPIO_Pin_2

int main(void)
{
    GPIO_InitTypeDef gi = {0};
    u8 on = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);          /* USART1 TX = PA9 */
    printf("{{NAME}} on {{BOARD}}, SystemClk %d\r\n", SystemCoreClock);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gi.GPIO_Pin = LED_PINN;
    gi.GPIO_Mode = GPIO_Mode_Out_PP;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &gi);

    for (;;) {
        Delay_Ms(500);
        GPIO_WriteBit(LED_PORT, LED_PINN, (on ^= 1) ? Bit_SET : Bit_RESET);
    }
}
