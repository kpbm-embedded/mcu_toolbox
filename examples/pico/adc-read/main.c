#include "board.h"
#include "hardware/adc.h"
#include <stdio.h>
int main(void) {
    stdio_init_all();
    adc_init();
    adc_gpio_init(26);                 /* ADC0 */
    adc_set_temp_sensor_enabled(true);
    const float conv = 3.3f / (1 << 12);
    while (true) {
        adc_select_input(0);
        uint16_t raw = adc_read();
        adc_select_input(4);           /* internal temperature sensor (RP2040 and RP2350A) */
        float v = adc_read() * conv;
        float c = 27.0f - (v - 0.706f) / 0.001721f;   /* RP2040 datasheet 4.9.5; same formula used for RP2350 in pico-examples */
        printf("GP26 raw=%u (%.3f V)  die temp=%.1f C\n", raw, raw * conv, c);
        sleep_ms(1000);
    }
}
