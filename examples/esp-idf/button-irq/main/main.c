#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "pins.h"

static const char *TAG = "button-irq";
static QueueHandle_t q;

static void IRAM_ATTR isr(void *arg)
{
    uint32_t pin = (uint32_t)(uintptr_t)arg;
    xQueueSendFromISR(q, &pin, NULL);
}

void app_main(void)
{
    q = xQueueCreate(8, sizeof(uint32_t));
    gpio_config_t io = { .pin_bit_mask = 1ULL << PIN_BUTTON, .mode = GPIO_MODE_INPUT,
                         .pull_up_en = GPIO_PULLUP_ENABLE, .intr_type = GPIO_INTR_NEGEDGE };
    ESP_ERROR_CHECK(gpio_config(&io));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_BUTTON, isr, (void *)(uintptr_t)PIN_BUTTON));
    int64_t last = 0; uint32_t pin, count = 0;
    for (;;) {
        if (xQueueReceive(q, &pin, portMAX_DELAY)) {
            int64_t now = esp_timer_get_time();
            if (now - last > 50000) { /* 50 ms debounce */
                ESP_LOGI(TAG, "button on GPIO%lu pressed (%lu)", (unsigned long)pin, (unsigned long)++count);
            }
            last = now;
        }
    }
}
