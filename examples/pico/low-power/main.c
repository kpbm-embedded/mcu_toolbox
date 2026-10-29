/* Low power with pico_low_power (pico-sdk 2.3): LED on while awake, then
 *   1. SLEEP   for 5 s (clocks gated, woken by the system timer), then
 *   2. DORMANT until the button on GP14 is pressed (falling edge; all oscillators stopped).
 * Console: UART0 GP0/GP1 115200. */
#include "board.h"
#include <stdio.h>
#include "pico/low_power.h"

#if PICO_RP2040
#define DORMANT_SRC DORMANT_CLOCK_SOURCE_XOSC     /* as pico-examples low_power_dormant_gpio.c */
#else
#define DORMANT_SRC DORMANT_CLOCK_SOURCE_LPOSC
#endif

int main(void) {
    stdio_init_all();
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN); gpio_set_dir(BUTTON_PIN, GPIO_IN); gpio_pull_up(BUTTON_PIN);

    for (unsigned run = 1;; run++) {
        gpio_put(LED_PIN, 1);
        printf("run %u: awake 2 s\n", run);
        sleep_ms(2000);

        printf("sleep 5 s\n");
        gpio_put(LED_PIN, 0);
        int rc = low_power_sleep_for_ms(5000, NULL, true);
        gpio_put(LED_PIN, 1);
        printf("woke from sleep (rc=%d)\n", rc);
        sleep_ms(500);

        printf("dormant until GP%u goes low (press the button)\n", BUTTON_PIN);
        stdio_flush();
        gpio_put(LED_PIN, 0);
        rc = low_power_dormant_until_gpio_pin_state(BUTTON_PIN, true, false, DORMANT_SRC, NULL);
        printf("woke from dormant (rc=%d)\n", rc);
        while (!gpio_get(BUTTON_PIN)) tight_loop_contents();   /* wait for release */
    }
}
