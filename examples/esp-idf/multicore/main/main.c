/* Dual-core demo: one task pinned to each core, a cross-core FreeRTOS queue,
 * and esp_ipc_call() to run a function on a chosen core. */
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_ipc.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "sdkconfig.h"

#if CONFIG_FREERTOS_NUMBER_OF_CORES < 2
#error "multicore needs a dual-core target with CONFIG_FREERTOS_UNICORE=n (esp32, esp32s3, esp32p4)"
#endif

static const char *TAG = "multicore";

typedef struct { uint32_t seq; int from_core; int64_t t_us; } msg_t;
static QueueHandle_t q;

static void producer(void *arg) /* pinned to core 0 */
{
    for (uint32_t i = 0;; i++) {
        msg_t m = { .seq = i, .from_core = xPortGetCoreID(), .t_us = esp_timer_get_time() };
        xQueueSend(q, &m, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void consumer(void *arg) /* pinned to core 1 */
{
    msg_t m;
    for (;;) {
        if (xQueueReceive(q, &m, portMAX_DELAY)) {
            ESP_LOGI(TAG, "core %d got #%lu from core %d, latency %lld us", (int)xPortGetCoreID(),
                     (unsigned long)m.seq, m.from_core, (long long)(esp_timer_get_time() - m.t_us));
        }
    }
}

static void ipc_fn(void *arg) /* runs on the core given to esp_ipc_call */
{
    *(int *)arg = xPortGetCoreID();
}

void app_main(void)
{
    ESP_LOGI(TAG, "app_main on core %d, %d cores", (int)xPortGetCoreID(), CONFIG_FREERTOS_NUMBER_OF_CORES);
    for (int c = 0; c < 2; c++) {
        int ran_on = -1;
        ESP_ERROR_CHECK(esp_ipc_call_blocking(c, ipc_fn, &ran_on));
        ESP_LOGI(TAG, "esp_ipc_call_blocking(core %d) ran on core %d", c, ran_on);
    }
    q = xQueueCreate(8, sizeof(msg_t));
    xTaskCreatePinnedToCore(producer, "producer", 3072, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(consumer, "consumer", 3072, NULL, 5, NULL, 1);
}
