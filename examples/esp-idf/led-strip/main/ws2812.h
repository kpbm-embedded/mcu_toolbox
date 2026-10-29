#pragma once
/* Minimal WS2812 driver on the built-in RMT driver (no managed component). */
#include <stdint.h>
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "esp_check.h"

typedef struct { rmt_channel_handle_t ch; rmt_encoder_handle_t enc; } ws2812_t;

static inline esp_err_t ws2812_init(ws2812_t *w, int gpio)
{
    rmt_tx_channel_config_t c = { .gpio_num = gpio, .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, .mem_block_symbols = 64, .trans_queue_depth = 2 };
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&c, &w->ch), "ws2812", "channel");
    rmt_bytes_encoder_config_t e = {
        .bit0 = { .level0 = 1, .duration0 = 3, .level1 = 0, .duration1 = 9 }, /* 0.3us/0.9us */
        .bit1 = { .level0 = 1, .duration0 = 9, .level1 = 0, .duration1 = 3 }, /* 0.9us/0.3us */
        .flags.msb_first = 1 };
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&e, &w->enc), "ws2812", "encoder");
    return rmt_enable(w->ch);
}

static inline esp_err_t ws2812_set(ws2812_t *w, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t grb[3] = { g, r, b };
    rmt_transmit_config_t t = { .loop_count = 0 };
    ESP_RETURN_ON_ERROR(rmt_transmit(w->ch, w->enc, grb, sizeof grb, &t), "ws2812", "tx");
    return rmt_tx_wait_all_done(w->ch, 100);
}
