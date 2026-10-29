#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "pins.h"
#include "ws2812.h"

static const char *TAG = "led-strip";

void app_main(void)
{
    ws2812_t w;
    ESP_ERROR_CHECK(ws2812_init(&w, PIN_WS2812));
    static const uint8_t col[][3] = { {16,0,0}, {0,16,0}, {0,0,16}, {16,16,16}, {0,0,0} };
    for (int i = 0;; i = (i + 1) % 5) {
        ESP_ERROR_CHECK(ws2812_set(&w, col[i][0], col[i][1], col[i][2]));
        ESP_LOGI(TAG, "colour %d (r=%d g=%d b=%d)", i, col[i][0], col[i][1], col[i][2]);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
