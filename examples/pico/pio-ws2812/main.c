#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

/* On-board WS2812 on GPIO16: PICO_DEFAULT_WS2812_PIN in waveshare_rp2040_zero.h and
 * waveshare_rp2350_zero.h; Waveshare wiki lists the RGB LED on GP16. Colour order is GRB.
 * pico.h / pico2.h define no WS2812 pin: there the fallback below drives an external
 * strip's DIN on GP16 (first pixel only). Override with -DPICO_DEFAULT_WS2812_PIN=<n>. */
#ifndef PICO_DEFAULT_WS2812_PIN
#define PICO_DEFAULT_WS2812_PIN 16
#endif

static inline void put_pixel(PIO pio, uint sm, uint8_t r, uint8_t g, uint8_t b) {
    pio_sm_put_blocking(pio, sm, ((uint32_t)g << 16 | (uint32_t)r << 8 | b) << 8u);
}
static void wheel(uint8_t p, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (p < 85)       { *r = p * 3;          *g = 255 - p * 3; *b = 0; }
    else if (p < 170) { p -= 85;  *r = 255 - p * 3; *g = 0;          *b = p * 3; }
    else              { p -= 170; *r = 0;          *g = p * 3;      *b = 255 - p * 3; }
}
int main(void) {
    PIO pio = pio0; uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, PICO_DEFAULT_WS2812_PIN, 800000);
    for (uint8_t p = 0;; p++) {
        uint8_t r, g, b; wheel(p, &r, &g, &b);
        put_pixel(pio, sm, r / 8, g / 8, b / 8);     /* dimmed */
        sleep_ms(20);
    }
}
