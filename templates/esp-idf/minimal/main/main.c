#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_PIN {{LED_PIN}} /* default for {{TARGET}} / {{BOARD}}; change for your board */

static const char *TAG = "{{NAME}}";

void app_main(void)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "{{NAME}} started on {{TARGET}} ({{BOARD}})");
    for (int on = 0;; on = !on) {
        gpio_set_level(LED_PIN, on);
        ESP_LOGI(TAG, "tick %d", on);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
