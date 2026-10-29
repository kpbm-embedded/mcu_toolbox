/* CH32V203 / CH32V307 standby (deepest mode, RAM lost, wake = reset). Wake-up sources: WKUP pin PA0 rising edge
 * (PWR_CSR.EWUP) and the independent watchdog (LSI ~40 kHz / 128, reload 1561 -> ~5 s), as in ch32fun
 * examples_v30x/lowpower_standbyMode_test. After each wake the reset cause is printed via SDI printf (minichlink -T), see funconfig.h. */
#include "board.h"

static void iwdg_start(uint16_t prescaler, uint16_t reload)
{
    IWDG->CTLR = IWDG_WriteAccess_Enable;
    IWDG->PSCR = prescaler;
    IWDG->RLDR = reload;
    IWDG->CTLR = CTLR_KEY_Reload;
    IWDG->CTLR = CTLR_KEY_Enable;
}

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_INIT();
    RCC->APB1PCENR |= RCC_APB1Periph_PWR;

    uint32_t csr = PWR->CSR, rst = RCC->RSTSCKR;
    PWR->CTLR |= PWR_CTLR_CSBF | PWR_CTLR_CWUF;
    RCC->RSTSCKR |= RCC_RMVF;
    Delay_Ms(100);
    printf("standby (%s): ", BOARD_STR);
    if (!(csr & PWR_CSR_SBF)) printf("cold start / reset pin\r\n");
    else if (rst & RCC_IWDGRSTF) printf("woke from standby by the IWDG (~5 s)\r\n");
    else printf("woke from standby by WKUP pin PA0\r\n");

    for (int i = 0; i < 6; i++) { LED_SET(!(i & 1)); Delay_Ms(150); }
    LED_SET(0);
    Delay_Ms(3000);                  /* window to reflash before the chip sleeps again */

    printf("entering standby\r\n");
    Delay_Ms(5);
    iwdg_start(IWDG_Prescaler_128, 1561);
    PWR->CSR |= PWR_CSR_EWUP;        /* PA0 becomes the WKUP input (rising edge) */
    PWR->CTLR |= PWR_CTLR_PDDS;      /* deep sleep = standby */
    NVIC->SCTLR |= (1 << 2);         /* SLEEPDEEP */
    __WFI();
    for (;;) {}
}
