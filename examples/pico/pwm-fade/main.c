#include "board.h"
#include "hardware/pwm.h"
#include <stdio.h>
int main(void) {
    stdio_init_all();
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(LED_PIN);
    uint chan = pwm_gpio_to_channel(LED_PIN);
    pwm_set_wrap(slice, 999);                 /* 1000 steps */
    pwm_set_clkdiv(slice, 4.f);               /* ~31 kHz on a 125 MHz clock */
    pwm_set_enabled(slice, true);
    int level = 0, dir = 5;
    while (true) {
        pwm_set_chan_level(slice, chan, (uint16_t)(level * level / 1000));   /* gamma-ish */
        level += dir;
        if (level >= 1000 || level <= 0) { dir = -dir; level += dir; }
        sleep_ms(4);
    }
}
