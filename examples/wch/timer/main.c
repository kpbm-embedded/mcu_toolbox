#include "board.h"

void TIM1_UP_IRQHandler(void) INTERRUPT_DECORATOR;
void TIM1_UP_IRQHandler(void)
{
    if (TIM1->INTFR & TIM_UIF) {
        TIM1->INTFR = ~TIM_UIF;
        funDigitalWrite(LED_PIN, funDigitalRead(LED_PIN) ? FUN_LOW : FUN_HIGH);
    }
}

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_INIT();
#if defined(BOARD_CH32H417)
    RCC->HB2PCENR |= RCC_HB2Periph_TIM1;
#else
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;
#endif
    /* 10 kHz tick, assuming the TIM1 kernel clock = FUNCONF_SYSTEM_CORE_CLOCK (true on V003/V203/V307 with ch32fun's
     * default prescalers; on the H417 it is the same assumption ch32fun's UART_BRR makes, unverified) */
    TIM1->PSC = (FUNCONF_SYSTEM_CORE_CLOCK / 10000) - 1;
    TIM1->ATRLR = 5000 - 1;                                 /* 500 ms */
    TIM1->DMAINTENR |= TIM_UIE;
    NVIC_EnableIRQ(TIM1_UP_IRQn);
    TIM1->CTLR1 |= TIM_CEN;
    for (;;) __WFI();
}
