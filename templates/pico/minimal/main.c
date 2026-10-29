#include "pico/stdlib.h"
#include <stdio.h>
#include "config.h"

int main(void) {
    stdio_init_all();
    gpio_init(APP_LED_PIN);
    gpio_set_dir(APP_LED_PIN, GPIO_OUT);
    for (unsigned n = 0;; n++) {
        gpio_put(APP_LED_PIN, n & 1);
        printf("{{NAME}}: tick %u\n", n);
        sleep_ms(500);
    }
}
