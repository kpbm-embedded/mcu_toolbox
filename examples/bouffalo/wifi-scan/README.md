# bouffalo / wifi-scan

Wi-Fi station scan every 10 s (SDK fhost/wifi_mgmr), prints SSID, channel, RSSI, BSSID. Status: **builds; untested on hardware**.

Targets: bl616 (Sipeed M0s Dock BL616 / Ai-M62).

## Pins
No pins (on-chip Wi-Fi; antenna on the module).
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/wifi-scan/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only wifi-scan bouffalo`.


## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
Lists nearby APs. Links with -fno-lto because the SDK prebuilt libmacsw_bl616.a was built with a newer GCC than the installed Xuantie 10.4.0 (see CMakeLists.txt).
