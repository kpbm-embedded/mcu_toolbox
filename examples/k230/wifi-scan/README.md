# k230 / wifi-scan

Wi-Fi station scan through the RT-Smart network manager (`hal_netmgmt.h`, `netmgmt_wlan_sta_scan`) and prints SSID, RSSI, channel and BSSID of up to 64 access points.

Status: builds; untested on hardware.

## Pins
No GPIO. The Wi-Fi module and its pins are owned by the kernel driver (from the kernel defconfigs in `src/rtsmart/rtsmart/kernel/bsp/maix3/configs/`): 01Studio `CONFIG_RT_USING_AIC8800_WIFI` on SDIO with REG_ON pin 53 (`CONFIG_BSP_WIFI_SDIO_REG_ON_PIN=53`); BPI-K230D-Zero `CONFIG_RT_USING_REALTEK` on SDIO host 0 (`CONFIG_BSP_WIFI_SDIO_HOST_0`). Whether the owned boards carry exactly these modules is **unverified**.

## Build
RT-Smart user-space app with its own Makefile that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
read only (same scheme as ../hello). Needs the SDK output for the board (`./mcu build k230 <target>` once) and the SDK's global
`.config` set to that board; `check.sh` does the selection under the SDK build flock.
```
mcu/templates/tools/check.sh --only wifi-scan k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/wifi-scan/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` to the SD card (`/sdcard/app/`), run it
from the msh console on the debug UART (UART0, 115200): `/sdcard/app/app.elf`.

## Expected
`wifi-scan (<board>): scanning...`, `N access point(s)` and one line per AP: `MyWifi  rssi  -52  ch   6  aa:bb:cc:dd:ee:ff`. `scan failed` if the Wi-Fi driver did not come up.
