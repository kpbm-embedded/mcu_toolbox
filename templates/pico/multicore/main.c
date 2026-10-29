#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <stdio.h>
#include "config.h"

/* Core 1 blinks the LED; core 0 reports a counter received through the SIO FIFO. */
static void core1_main(void) {
    gpio_init(APP_LED_PIN);
    gpio_set_dir(APP_LED_PIN, GPIO_OUT);
    for (uint32_t n = 0;; n++) {
        gpio_put(APP_LED_PIN, n & 1);
        multicore_fifo_push_blocking(n);
        sleep_ms(500);
    }
}

int main(void) {
    stdio_init_all();
    multicore_launch_core1(core1_main);
    while (true) {
        uint32_t n = multicore_fifo_pop_blocking();
        printf("{{NAME}}: core1 tick %u\n", (unsigned)n);
    }
}
