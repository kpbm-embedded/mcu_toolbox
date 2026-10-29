/* BLE observer: active scan for 10 s, print every advertiser (address, RSSI, name), pause 5 s, repeat.
 * Controller/host bring-up as in bouffalo_sdk examples/btble/central and btble_cli (main.c). */
#include <string.h>
#include <FreeRTOS.h>
#include "task.h"
#include "board.h"
#include "bluetooth.h"
#include "hci_driver.h"
#include "hci_core.h"
#include "btble_lib_api.h"
#include "rfparam_adapter.h"

static volatile int seen;

static bool name_cb(struct bt_data *data, void *user)
{
    char *name = user;
    if (data->type == BT_DATA_NAME_COMPLETE || data->type == BT_DATA_NAME_SHORTENED) {
        size_t n = data->data_len < 29 ? data->data_len : 29;
        memcpy(name, data->data, n);
        name[n] = 0;
        return false;
    }
    return true;
}

static void device_found(const bt_addr_le_t *addr, s8_t rssi, u8_t evtype, struct net_buf_simple *buf)
{
    char a[BT_ADDR_LE_STR_LEN], name[30] = "";
    bt_data_parse(buf, name_cb, name);
    bt_addr_le_to_str(addr, a, sizeof(a));
    printf("  %s rssi %d evt %u %s\r\n", a, rssi, evtype, name);
    seen++;
}

static void scan_task(void *arg)
{
    struct bt_le_scan_param p = {
        .type = BT_HCI_LE_SCAN_ACTIVE,
        .filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_ENABLE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
    };
    for (unsigned round = 1;; round++) {
        seen = 0;
        printf("ble scan round %u\r\n", round);
        int err = bt_le_scan_start(&p, device_found);
        if (err) {
            printf("bt_le_scan_start failed: %d\r\n", err);
        } else {
            vTaskDelay(pdMS_TO_TICKS(10000));
            bt_le_scan_stop();
        }
        printf("round %u: %d advertisement(s)\r\n", round, seen);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void bt_enable_cb(int err)
{
    if (err) {
        printf("bt_enable failed: %d\r\n", err);
        return;
    }
    bt_addr_le_t own;
    bt_get_local_public_address(&own);
    printf("BLE up, own address %02x:%02x:%02x:%02x:%02x:%02x\r\n", own.a.val[5], own.a.val[4], own.a.val[3],
           own.a.val[2], own.a.val[1], own.a.val[0]);
    xTaskCreate(scan_task, "scan", 1024, NULL, 3, NULL);
}

static void app_start_task(void *arg)
{
    btble_controller_init(configMAX_PRIORITIES - 1);
    hci_driver_init();
    bt_enable(bt_enable_cb);
    vTaskDelete(NULL);
}

int main(void)
{
    board_init();
    if (rfparam_init(0, NULL, 0) != 0) {
        printf("PHY RF init failed\r\n");
        return 0;
    }
    xTaskCreate(app_start_task, "app_start", 1024, NULL, configMAX_PRIORITIES - 2, NULL);
    vTaskStartScheduler();
    while (1) {
    }
}
