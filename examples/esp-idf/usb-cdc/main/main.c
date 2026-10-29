#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"

void app_main(void)
{
    usb_serial_jtag_driver_config_t cfg = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&cfg));
    const char *hi = "usb-cdc echo ready\r\n";
    usb_serial_jtag_write_bytes(hi, strlen(hi), pdMS_TO_TICKS(100));
    uint8_t buf[64];
    for (;;) {
        int n = usb_serial_jtag_read_bytes(buf, sizeof buf, pdMS_TO_TICKS(100));
        if (n > 0) usb_serial_jtag_write_bytes(buf, n, pdMS_TO_TICKS(100));
    }
}
