#include "board.h"
#include "py32f0xx_ll_lptim.h"

/* Stop mode, woken by LPTIM1 (LSI 32.768 kHz / 128, 256 counts = ~1 s, EXTI line 29) or by the key (EXTI falling edge).
 * Based on Puya LL/LPTIM/LPTIM1_Wakeup and LL/PWR/PWR_Stop. The LED is pulsed after every wake-up; USART1 reports why. */
static volatile uint8_t woke_by_key;

void LPTIM1_IRQHandler(void)
{
    if (LL_LPTIM_IsActiveFlag_ARRM(LPTIM1)) LL_LPTIM_ClearFLAG_ARRM(LPTIM1);
}

void EXTI4_15_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag(KEY_LINE)) { LL_EXTI_ClearFlag(KEY_LINE); woke_by_key = 1; }
}

static void lptim_init(void)
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LPTIM1);
    LL_RCC_LSI_Enable();
    while (!LL_RCC_LSI_IsReady()) {}
    LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM1_CLKSOURCE_LSI);
    LL_LPTIM_SetPrescaler(LPTIM1, LL_LPTIM_PRESCALER_DIV128);
    LL_LPTIM_SetUpdateMode(LPTIM1, LL_LPTIM_UPDATE_MODE_ENDOFPERIOD);
    LL_LPTIM_EnableIT_ARRM(LPTIM1);
    LL_LPTIM_Enable(LPTIM1);
    LL_LPTIM_SetAutoReload(LPTIM1, 255);
    LL_EXTI_EnableIT(LL_EXTI_LINE_29);       /* LPTIM1 wake-up line */
    NVIC_SetPriority(LPTIM1_IRQn, 1);
    NVIC_EnableIRQ(LPTIM1_IRQn);
}

static void key_init(void)
{
    LL_IOP_GRP1_EnableClock(KEY_CLK);
    LL_GPIO_InitTypeDef gi = {0};
    gi.Pin = KEY_PIN; gi.Mode = LL_GPIO_MODE_INPUT; gi.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(KEY_PORT, &gi);
    LL_EXTI_InitTypeDef ei = {0};
    ei.Line = KEY_LINE; ei.LineCommand = ENABLE; ei.Mode = LL_EXTI_MODE_IT; ei.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&ei);
    NVIC_SetPriority(EXTI4_15_IRQn, 1);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}

int main(void)
{
    BSP_RCC_HSI_24MConfig();
    BSP_USART_Config(115200);
    LL_mDelay(3000);                          /* keep SWD usable for 3 s after reset (as PWR_Stop does) */
    LL_IOP_GRP1_EnableClock(LED_CLK);
    LL_GPIO_SetPinMode(LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    lptim_init();
    key_init();
    printf("low-power: stop mode, wake by LPTIM1 (~1 s) or key\r\n");
    for (unsigned n = 1;; n++) {
        woke_by_key = 0;
        LL_LPTIM_StartCounter(LPTIM1, LL_LPTIM_OPERATING_MODE_ONESHOT);
        while (!LL_USART_IsActiveFlag_TC(DEBUG_USART)) {}   /* let the UART finish before the clocks stop */
        LL_PWR_EnableLowPowerRunMode();       /* low-power regulator in stop */
        LL_LPM_EnableDeepSleep();
        __WFI();
        LL_LPM_EnableSleep();
        LL_GPIO_SetOutputPin(LED_PORT, LED_PIN);
        printf("wake %u by %s\r\n", n, woke_by_key ? "key" : "LPTIM");
        LL_mDelay(20);
        LL_GPIO_ResetOutputPin(LED_PORT, LED_PIN);
    }
}
