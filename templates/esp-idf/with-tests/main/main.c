#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "calc.h"

static const char *TAG = "{{NAME}}";

void app_main(void)
{
    for (int i = 0;; i++) {
        ESP_LOGI(TAG, "{{NAME}} on {{TARGET}}: clamp(%d,0,10)=%d", i * 4, calc_clamp(i * 4, 0, 10));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
