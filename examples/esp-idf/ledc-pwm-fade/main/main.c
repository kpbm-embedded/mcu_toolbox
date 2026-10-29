#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "pins.h"

static const char *TAG = "ledc-fade";
#define MODE LEDC_LOW_SPEED_MODE
#define CH   LEDC_CHANNEL_0

void app_main(void)
{
    ledc_timer_config_t t = { .speed_mode = MODE, .duty_resolution = LEDC_TIMER_10_BIT, .timer_num = LEDC_TIMER_0,
                              .freq_hz = 5000, .clk_cfg = LEDC_AUTO_CLK };
    ESP_ERROR_CHECK(ledc_timer_config(&t));
    ledc_channel_config_t c = { .gpio_num = PIN_LED, .speed_mode = MODE, .channel = CH, .timer_sel = LEDC_TIMER_0, .duty = 0, .hpoint = 0 };
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    ESP_ERROR_CHECK(ledc_fade_func_install(0));
    for (;;) {
        ESP_LOGI(TAG, "fade up");
        ledc_set_fade_with_time(MODE, CH, 1023, 1500);
        ledc_fade_start(MODE, CH, LEDC_FADE_WAIT_DONE);
        ESP_LOGI(TAG, "fade down");
        ledc_set_fade_with_time(MODE, CH, 0, 1500);
        ledc_fade_start(MODE, CH, LEDC_FADE_WAIT_DONE);
    }
}
