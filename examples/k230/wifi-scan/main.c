/* Wi-Fi station scan through the RT-Smart network manager (hal_netmgmt): list SSID, RSSI, channel, BSSID. */
#include <stdio.h>
#include "hal_netmgmt.h"
#include "board.h"

static struct rt_wlan_info_t aps[RT_WLAN_STA_SCAN_MAX_AP];

int main(void)
{
    int n = 0;
    printf("wifi-scan (%s): scanning...\n", BOARD_NAME);
    if (netmgmt_wlan_sta_scan(&n, aps) != 0) { printf("scan failed (is the Wi-Fi module up? see README)\n"); return 1; }
    printf("%d access point(s)\n", n);
    for (int i = 0; i < n; i++) {
        const uint8_t *b = aps[i].bssid;
        printf("  %-32.32s rssi %4d  ch %3d  %02x:%02x:%02x:%02x:%02x:%02x%s\n", (const char *)aps[i].ssid.val, aps[i].rssi,
               aps[i].channel, b[0], b[1], b[2], b[3], b[4], b[5], aps[i].hidden ? "  (hidden)" : "");
    }
    return 0;
}
