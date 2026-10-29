/* Memory-to-memory DMA with the async_memcpy driver (GDMA), compared with CPU memcpy. */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_async_memcpy.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "soc/soc_caps.h"

#if !SOC_ASYNC_MEMCPY_SUPPORTED
#error "this chip has no async memcpy (GDMA/CP-DMA) engine"
#endif

static const char *TAG = "dma-memcpy";
#define N (32 * 1024)

static bool IRAM_ATTR done_cb(async_memcpy_handle_t h, async_memcpy_event_t *e, void *arg)
{
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR((SemaphoreHandle_t)arg, &woken);
    return woken == pdTRUE;
}

void app_main(void)
{
    async_memcpy_config_t cfg = ASYNC_MEMCPY_DEFAULT_CONFIG();
    async_memcpy_handle_t mcp;
    ESP_ERROR_CHECK(esp_async_memcpy_install(&cfg, &mcp));
    uint8_t *src = heap_caps_aligned_calloc(64, 1, N, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    uint8_t *dst = heap_caps_aligned_calloc(64, 1, N, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    SemaphoreHandle_t done = xSemaphoreCreateBinary();
    if (!src || !dst || !done) { ESP_LOGE(TAG, "alloc failed"); return; }

    for (uint32_t round = 0;; round++) {
        for (int i = 0; i < N; i++) src[i] = (uint8_t)(i * 7 + round);
        memset(dst, 0, N);
        int64_t t0 = esp_timer_get_time();
        ESP_ERROR_CHECK(esp_async_memcpy(mcp, dst, src, N, done_cb, done));
        xSemaphoreTake(done, portMAX_DELAY);
        int64_t t_dma = esp_timer_get_time() - t0;
        bool ok = memcmp(src, dst, N) == 0;
        t0 = esp_timer_get_time();
        memcpy(dst, src, N);
        int64_t t_cpu = esp_timer_get_time() - t0;
        ESP_LOGI(TAG, "round %lu: DMA %d bytes in %lld us (%s), CPU memcpy %lld us",
                 (unsigned long)round, N, (long long)t_dma, ok ? "match" : "MISMATCH", (long long)t_cpu);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
