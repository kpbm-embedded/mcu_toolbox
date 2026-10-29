/* BL616 Wi-Fi station scan: bring up the Wi-Fi firmware task, scan every 10 s, print SSID/channel/RSSI/BSSID.
 * Structure follows the SDK example examples/wifi/sta/wifi_tcp (Apache-2.0); only the scan path is kept. */
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <lwip/tcpip.h>
#include "wifi_mgmr_ext.h"
#include "fhost_api.h"
#include "wifi_mgmr.h"
#include "bflb_uart.h"
#include "rfparam_adapter.h"
#include "async_event.h"
#include "board.h"
#include "shell.h"

#define DBG_TAG "SCAN"
#include "log.h"

extern void shell_init_with_task(struct bflb_device_s *shell);
static volatile bool mgmr_ready;

static void print_scan_results(void)
{
    uint32_t num = wifi_mgmr_sta_scanlist_nums_get();
    printf("scan done: %u AP(s)\r\n", (unsigned)num);
    if (num == 0) {
        return;
    }
    wifi_mgmr_scan_item_t *res = pvPortMalloc(num * sizeof(*res));
    if (res == NULL) {
        return;
    }
    num = wifi_mgmr_sta_scanlist_dump(res, num);
    for (uint32_t i = 0; i < num; i++) {
        printf("%2u ch%-3u %4d dBm %02x:%02x:%02x:%02x:%02x:%02x  %.*s\r\n", (unsigned)i, res[i].channel, res[i].rssi,
               res[i].bssid[0], res[i].bssid[1], res[i].bssid[2], res[i].bssid[3], res[i].bssid[4], res[i].bssid[5],
               res[i].ssid_len, res[i].ssid);
    }
    vPortFree(res);
}

static void wifi_event_handler(async_input_event_t ev, void *priv)
{
    switch (ev->code) {
        case CODE_WIFI_ON_INIT_DONE:
            wifi_mgmr_task_start();
            wifi_mgmr_scan_filter_hidden_ssid(0);
            break;
        case CODE_WIFI_ON_MGMR_DONE:
            mgmr_ready = true;
            break;
        case CODE_WIFI_ON_SCAN_DONE:
            print_scan_results();
            break;
        default:
            break;
    }
}

static void wifi_start_firmware_task(void *param)
{
    async_register_event_filter(EV_WIFI, wifi_event_handler, NULL);
    wifi_task_create();
    fhost_init();
    vTaskDelete(NULL);
}

static void scan_task(void *param)
{
    while (!mgmr_ready) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    while (1) {
        wifi_mgmr_scan_params_t config;
        memset(&config, 0, sizeof(config));
        printf("scanning...\r\n");
        if (wifi_mgmr_sta_scan(&config) < 0) {
            printf("scan start failed\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

int main(void)
{
    board_init();

    struct bflb_device_s *uart0 = bflb_device_get_by_name("uart0");
    shell_init_with_task(uart0);

    if (0 != rfparam_init(0, NULL, 0)) {
        LOG_E("PHY RF init failed\r\n");
        return 0;
    }
    tcpip_init(NULL, NULL);
    xTaskCreate(wifi_start_firmware_task, "wifi init", 1024, NULL, 10, NULL);
    xTaskCreate(scan_task, "scan", 1024, NULL, 8, NULL);
    vTaskStartScheduler();
    while (1) {
    }
}
