#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "wifi-scan";
#define MAX_AP 20

void app_main(void)
{
    esp_err_t r = nvs_flash_init();
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) { ESP_ERROR_CHECK(nvs_flash_erase()); r = nvs_flash_init(); }
    ESP_ERROR_CHECK(r);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    for (;;) {
        ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true)); /* blocking */
        wifi_ap_record_t ap[MAX_AP]; uint16_t n = MAX_AP;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&n, ap));
        ESP_LOGI(TAG, "%u AP(s)", n);
        for (int i = 0; i < n; i++)
            ESP_LOGI(TAG, "%-32s rssi %d ch %d", (char *)ap[i].ssid, ap[i].rssi, ap[i].primary);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
