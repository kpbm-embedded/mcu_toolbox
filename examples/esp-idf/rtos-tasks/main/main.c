#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "rtos-tasks";
static QueueHandle_t q;
static SemaphoreHandle_t lock;
static int shared;

static void producer(void *arg)
{
    for (int i = 0;; i++) {
        xQueueSend(q, &i, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void consumer(void *arg)
{
    int v;
    for (;;) {
        if (xQueueReceive(q, &v, portMAX_DELAY)) {
            xSemaphoreTake(lock, portMAX_DELAY);
            shared += v;
            xSemaphoreGive(lock);
            ESP_LOGI(TAG, "got %d, sum %d (core %d)", v, shared, (int)xPortGetCoreID());
        }
    }
}

void app_main(void)
{
    q = xQueueCreate(4, sizeof(int));
    lock = xSemaphoreCreateMutex();
    xTaskCreate(producer, "producer", 2048, NULL, 5, NULL);
    xTaskCreate(consumer, "consumer", 3072, NULL, 5, NULL);
}
