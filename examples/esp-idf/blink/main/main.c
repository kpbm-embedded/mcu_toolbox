#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "pins.h"

static const char *TAG = "blink";

void app_main(void)
{
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);
    for (int on = 0;; on = !on) {
        gpio_set_level(PIN_LED, on); /* SuperMini LED is active low: logic is inverted, harmless */
        ESP_LOGI(TAG, "LED %s", on ? "high" : "low");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
