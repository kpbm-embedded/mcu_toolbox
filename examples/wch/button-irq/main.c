#include "board.h"

#if defined(BOARD_CH32V003)
/* PC3 = EXTI line 3, port C = 2 in the 2-bit AFIO_EXTICR fields; the vector covers lines 7..0 */
#define KEY_LINE_MASK 3
void EXTI7_0_IRQHandler(void) INTERRUPT_DECORATOR;
void EXTI7_0_IRQHandler(void)
{
    if (EXTI->INTFR & (1 << KEY_LINE_MASK)) {
        funDigitalWrite(LED_PIN, funDigitalRead(LED_PIN) ? FUN_LOW : FUN_HIGH);
        EXTI->INTFR = 1 << KEY_LINE_MASK;
    }
}
#elif defined(BOARD_CH32H417)
/* PA0 = EXTI line 0; CH32H41x has one vector for lines 7..0 and the port select in AFIO->EXTICR1 (4 bits per line) */
void EXTI7_0_IRQHandler(void) INTERRUPT_DECORATOR;
void EXTI7_0_IRQHandler(void)
{
    if (EXTI->INTFR & 1) {
        funDigitalWrite(LED_PIN, funDigitalRead(LED_PIN) ? FUN_LOW : FUN_HIGH);
        EXTI->INTFR = 1;
    }
}
#else
/* PA0 = EXTI line 0, port A = 0 in the 4-bit AFIO_EXTICR[0] field */
void EXTI0_IRQHandler(void) INTERRUPT_DECORATOR;
void EXTI0_IRQHandler(void)
{
    if (EXTI->INTFR & 1) {
        funDigitalWrite(LED_PIN, funDigitalRead(LED_PIN) ? FUN_LOW : FUN_HIGH);
        EXTI->INTFR = 1;
    }
}
#endif

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_INIT();
    LED_SET(0);
#if !defined(BOARD_CH32H417)
    RCC->APB2PCENR |= RCC_APB2Periph_AFIO;   /* H41x: funGpioInitAll() already enabled AFIO */
#endif
    PIN_IN(KEY_PIN, GPIO_CNF_IN_PUPD);
    funDigitalWrite(KEY_PIN, KEY_PULL_UP ? FUN_HIGH : FUN_LOW);     /* ODR selects pull-up / pull-down */
#if defined(BOARD_CH32V003)
    AFIO->EXTICR = (AFIO->EXTICR & ~AFIO_EXTICR_EXTI3) | (2 << (2 * 3));   /* PC3 */
    EXTI->INTENR = 1 << KEY_LINE_MASK;
    if (KEY_PRESSED) EXTI->RTENR = 1 << KEY_LINE_MASK; else EXTI->FTENR = 1 << KEY_LINE_MASK;
    NVIC_EnableIRQ(EXTI7_0_IRQn);
#elif defined(BOARD_CH32H417)
    AFIO->EXTICR1 = (AFIO->EXTICR1 & ~AFIO_EXTICR1_EXTI0) | AFIO_EXTICR1_EXTI0_PA;   /* PA0 */
    EXTI->INTENR = 1;
    if (KEY_PRESSED) EXTI->RTENR = 1; else EXTI->FTENR = 1;
    NVIC_EnableIRQ(EXTI7_0_IRQn);
#else
    AFIO->EXTICR[0] = (AFIO->EXTICR[0] & ~0xF) | 0;                          /* PA0 */
    EXTI->INTENR = 1;
    if (KEY_PRESSED) EXTI->RTENR = 1; else EXTI->FTENR = 1;
    NVIC_EnableIRQ(EXTI0_IRQn);
#endif
    for (;;) __WFI();
}
