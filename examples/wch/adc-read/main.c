#include "board.h"

/* ch32fun ADC helpers: funAnalogInit() + funAnalogRead(n). V003: A2 = PC4. V203/V307/H417: channel 4 = PA4
 * (H417: "PA4/ADC4" in the CH32H417 datasheet pin table; funAnalogInit has a CH32H41x branch). */
#if defined(BOARD_CH32V003)
 #define ADC_PIN PC4
 #define ADC_CH  ANALOG_2
 #define VDD_MV  3300   /* V003 can run at 3.3 V or 5 V: set this to your supply */
#else
 #define ADC_PIN PA4
 #define ADC_CH  ANALOG_4
 #define VDD_MV  3300
#endif

int main(void)
{
    SystemInit();
    funGpioInitAll();
    PIN_IN(ADC_PIN, GPIO_CNF_IN_ANALOG);
    funAnalogInit();
    Delay_Ms(100);
    printf("adc-read: %s ch %d\r\n", BOARD_STR, ADC_CH);
    for (;;) {
        int raw = funAnalogRead(ADC_CH);          /* 10-bit on V003 (0..1023), 12-bit on V203/V307 */
#if defined(BOARD_CH32V003)
        int mv = raw * VDD_MV / 1023;
#else
        int mv = raw * VDD_MV / 4095;
#endif
        printf("raw %d = %d mV\r\n", raw, mv);
        Delay_Ms(500);
    }
}
