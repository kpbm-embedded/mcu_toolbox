#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "pins.h"

static const char *TAG = "uart-hello";
#define U UART_NUM_1

void app_main(void)
{
    uart_config_t cfg = { .baud_rate = 115200, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE,
                          .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, .source_clk = UART_SCLK_DEFAULT };
    ESP_ERROR_CHECK(uart_driver_install(U, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(U, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(U, PIN_UART_TX, PIN_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    const char *hello = "hello from UART1\r\n";
    uint8_t buf[64];
    for (;;) {
        uart_write_bytes(U, hello, strlen(hello));
        int n = uart_read_bytes(U, buf, sizeof buf, pdMS_TO_TICKS(1000));
        if (n > 0) {
            uart_write_bytes(U, buf, n); /* echo */
            ESP_LOGI(TAG, "echoed %d bytes", n);
        }
    }
}
