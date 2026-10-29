#include "board.h"
#include <stdio.h>
static volatile uint32_t ticks;
static bool on_timer(repeating_timer_t *t) {
    (void)t;
    gpio_xor_mask(1u << LED_PIN);
    ticks++;
    return true;            /* keep repeating */
}
int main(void) {
    stdio_init_all();
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);
    static repeating_timer_t timer;
    add_repeating_timer_ms(-500, on_timer, NULL, &timer);   /* negative: period measured start to start */
    uint32_t seen = 0;
    while (true) {
        if (ticks != seen) { seen = ticks; printf("tick %u\n", (unsigned)seen); }
        sleep_ms(20);
    }
}
