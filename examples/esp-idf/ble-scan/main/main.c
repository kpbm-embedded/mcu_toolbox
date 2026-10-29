#include "nvs_flash.h"
#include "esp_log.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"

static const char *TAG = "ble-scan";

static int gap_cb(struct ble_gap_event *ev, void *arg)
{
    if (ev->type == BLE_GAP_EVENT_DISC) {
        const uint8_t *a = ev->disc.addr.val;
        ESP_LOGI(TAG, "%02x:%02x:%02x:%02x:%02x:%02x rssi %d len %d", a[5], a[4], a[3], a[2], a[1], a[0],
                 ev->disc.rssi, ev->disc.length_data);
    }
    return 0;
}

static void on_sync(void)
{
    uint8_t own;
    ESP_ERROR_CHECK(ble_hs_util_ensure_addr(0));
    ESP_ERROR_CHECK(ble_hs_id_infer_auto(0, &own));
    struct ble_gap_disc_params p = { .passive = 1, .filter_duplicates = 1 };
    int rc = ble_gap_disc(own, BLE_HS_FOREVER, &p, gap_cb, NULL);
    ESP_LOGI(TAG, "scan started rc=%d", rc);
}

static void host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void app_main(void)
{
    esp_err_t r = nvs_flash_init();
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) { ESP_ERROR_CHECK(nvs_flash_erase()); r = nvs_flash_init(); }
    ESP_ERROR_CHECK(r);
    ESP_ERROR_CHECK(nimble_port_init());
    ble_hs_cfg.sync_cb = on_sync;
    nimble_port_freertos_init(host_task);
}
