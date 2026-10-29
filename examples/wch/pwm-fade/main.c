#include "board.h"

/* V003: TIM2_CH3 on PC0 (default mapping, = the default LED pin). V203/V307: TIM2_CH2 on PA1 (default mapping);
 * wire an LED + resistor from PA1 to GND (the on-board LED is not a timer pin). */
#if defined(BOARD_CH32V003)
 #define PWM_GPIO_EN  RCC_APB2Periph_GPIOC
 #define PWM_CFG_PIN  PC0
 #define SETDUTY(v)   (TIM2->CH3CVR = (v))
 #define OCMODE()     (TIM2->CHCTLR2 |= TIM_OC3M_2 | TIM_OC3M_1 | TIM_OC3PE)
 #define CCEN         TIM_CC3E
#elif defined(BOARD_CH32H417)
 /* PA1 = TIM2_CH2 on AF1 (CH32H417 datasheet pin table: "ADC_IN1/TIM2_CH2(AF1)") */
 #define PWM_CFG_PIN  PA1
 #define SETDUTY(v)   (TIM2->CH2CVR = (v))
 #define OCMODE()     (TIM2->CHCTLR1 |= TIM_OC2M_2 | TIM_OC2M_1 | TIM_OC2PE)
 #define CCEN         TIM_CC2E
#else
 #define PWM_GPIO_EN  RCC_APB2Periph_GPIOA
 #define PWM_CFG_PIN  PA1
 #define SETDUTY(v)   (TIM2->CH2CVR = (v))
 #define OCMODE()     (TIM2->CHCTLR1 |= TIM_OC2M_2 | TIM_OC2M_1 | TIM_OC2PE)
 #define CCEN         TIM_CC2E
#endif

int main(void)
{
    SystemInit();
#if defined(BOARD_CH32H417)
    funGpioInitAll();
    RCC->HB1PCENR |= RCC_HB1Periph_TIM2;
    funPinAF(PWM_CFG_PIN, 1);
#else
    RCC->APB2PCENR |= PWM_GPIO_EN | RCC_APB2Periph_AFIO;
    RCC->APB1PCENR |= RCC_APB1Periph_TIM2;
#endif
    PIN_MODE(PWM_CFG_PIN, GPIO_CNF_OUT_PP_AF);

    TIM2->PSC = 0;
    TIM2->ATRLR = 255;            /* 8-bit duty, f = SystemCoreClock / 256 (>=48 kHz on V003, 562 kHz on 144 MHz parts) */
    OCMODE();
    TIM2->CTLR1 |= TIM_ARPE;
    TIM2->CCER |= CCEN;
    TIM2->SWEVGR |= TIM_UG;
    TIM2->CTLR1 |= TIM_CEN;

    int duty = 0, step = 1;
    for (;;) {
        SETDUTY(duty);
        duty += step;
        if (duty >= 255 || duty <= 0) step = -step;
        Delay_Ms(8);
    }
}
