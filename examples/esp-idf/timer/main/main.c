#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "timer";

static void periodic_cb(void *arg)
{
    static int n;
    ESP_LOGI(TAG, "periodic #%d at %lld us", ++n, (long long)esp_timer_get_time());
}

static void oneshot_cb(void *arg)
{
    ESP_LOGI(TAG, "one-shot fired");
}

void app_main(void)
{
    esp_timer_handle_t p, o;
    const esp_timer_create_args_t pa = { .callback = periodic_cb, .name = "periodic" };
    const esp_timer_create_args_t oa = { .callback = oneshot_cb, .name = "oneshot" };
    ESP_ERROR_CHECK(esp_timer_create(&pa, &p));
    ESP_ERROR_CHECK(esp_timer_create(&oa, &o));
    ESP_ERROR_CHECK(esp_timer_start_periodic(p, 500 * 1000));
    ESP_ERROR_CHECK(esp_timer_start_once(o, 5 * 1000 * 1000));
    vTaskDelay(portMAX_DELAY);
}
