#include "pico/stdlib.h"
#include <stdio.h>
#include "config.h"
#include "logic.h"

int main(void) {
    stdio_init_all();
    gpio_init(APP_LED_PIN);
    gpio_set_dir(APP_LED_PIN, GPIO_OUT);
    for (uint32_t n = 0;; n++) {
        gpio_put(APP_LED_PIN, triangle_u8(n * 32) > 127);
        printf("{{NAME}}: step %u ramp %u\n", (unsigned)n, triangle_u8(n * 32));
        sleep_ms(250);
    }
}
