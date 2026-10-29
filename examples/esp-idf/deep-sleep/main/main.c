#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_attr.h"

static const char *TAG = "deep-sleep";
static RTC_DATA_ATTR int wakeups;

void app_main(void)
{
    ESP_LOGI(TAG, "wake cause %d, wakeups %d", (int)esp_sleep_get_wakeup_cause(), wakeups++);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(5ULL * 1000 * 1000));
    ESP_LOGI(TAG, "sleeping 5 s");
    esp_deep_sleep_start();
}
