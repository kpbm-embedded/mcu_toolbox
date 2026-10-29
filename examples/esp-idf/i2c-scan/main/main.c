#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "pins.h"

static const char *TAG = "i2c-scan";

void app_main(void)
{
    i2c_master_bus_handle_t bus;
    i2c_master_bus_config_t cfg = { .i2c_port = -1, .sda_io_num = PIN_I2C_SDA, .scl_io_num = PIN_I2C_SCL,
                                    .clk_source = I2C_CLK_SRC_DEFAULT, .glitch_ignore_cnt = 7, .flags.enable_internal_pullup = true };
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus));
    for (;;) {
        int found = 0;
        for (int a = 0x08; a < 0x78; a++) {
            if (i2c_master_probe(bus, a, 50) == ESP_OK) {
                ESP_LOGI(TAG, "device at 0x%02x", a);
                found++;
            }
        }
        ESP_LOGI(TAG, "scan done, %d device(s)", found);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
