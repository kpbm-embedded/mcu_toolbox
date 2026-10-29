#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_PIN {{LED_PIN}} /* default for {{TARGET}} / {{BOARD}}; change for your board */

static const char *TAG = "{{NAME}}";
static QueueHandle_t events;

static void blink_task(void *arg)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    for (int on = 0;; on = !on) {
        gpio_set_level(LED_PIN, on);
        xQueueSend(events, &on, 0); /* report the new LED state */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void log_task(void *arg)
{
    int on;
    for (;;) {
        if (xQueueReceive(events, &on, portMAX_DELAY)) ESP_LOGI(TAG, "LED -> %d", on);
    }
}

void app_main(void)
{
    events = xQueueCreate(8, sizeof(int));
    xTaskCreate(blink_task, "blink", 2048, NULL, 5, NULL);
    xTaskCreate(log_task, "log", 3072, NULL, 4, NULL);
}
