#include "board.h"
#include <stdio.h>
static volatile uint32_t presses;
static volatile uint32_t last_us;

static void on_button(uint gpio, uint32_t events) {
    uint32_t now = time_us_32();
    if (now - last_us < 50000) return;      /* 50 ms debounce */
    last_us = now;
    presses++;
    gpio_xor_mask(1u << LED_PIN);
    (void)gpio; (void)events;
}

int main(void) {
    stdio_init_all();
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN); gpio_set_dir(BUTTON_PIN, GPIO_IN); gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &on_button);
    uint32_t seen = 0;
    while (true) {
        if (presses != seen) { seen = presses; printf("presses=%u\n", (unsigned)seen); }
        sleep_ms(10);
    }
}
