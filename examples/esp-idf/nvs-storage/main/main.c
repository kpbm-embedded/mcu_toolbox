#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "nvs-storage";

void app_main(void)
{
    esp_err_t r = nvs_flash_init();
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) { ESP_ERROR_CHECK(nvs_flash_erase()); r = nvs_flash_init(); }
    ESP_ERROR_CHECK(r);
    nvs_handle_t h;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &h));
    uint32_t boots = 0;
    r = nvs_get_u32(h, "boots", &boots);
    if (r != ESP_OK && r != ESP_ERR_NVS_NOT_FOUND) ESP_ERROR_CHECK(r);
    boots++;
    ESP_ERROR_CHECK(nvs_set_u32(h, "boots", boots));
    ESP_ERROR_CHECK(nvs_set_str(h, "name", "mcu-example"));
    ESP_ERROR_CHECK(nvs_commit(h));
    char s[32]; size_t len = sizeof s;
    ESP_ERROR_CHECK(nvs_get_str(h, "name", s, &len));
    ESP_LOGI(TAG, "boot count %lu, name '%s'", (unsigned long)boots, s);
    nvs_close(h);
    ESP_LOGI(TAG, "restarting in 3 s");
    for (int i = 0; i < 3; i++) vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
}
