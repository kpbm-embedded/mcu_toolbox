# bouffalo / ble-scan

BLE observer: active scan for 10 s (duplicate filter on), printing address, RSSI, advertising event type and name of each advertiser, then a 5 s pause; repeats. Controller/host bring-up as in `bouffalo_sdk/examples/btble/central` and `btble_cli` (`btble_controller_init`, `hci_driver_init`, `bt_enable`, `bt_le_scan_start`). Status: **builds; untested on hardware**.

Targets: bl616 (Sipeed M0s Dock BL616 / Ai-M62). Not bl702: SDK 2.3.37 ships no `libblecontroller_bl702_*.a`, and the legacy SDK 2.2.0 only ships `libblecontroller_bl702_m0s1p.a`, a peripheral-only controller (`CONFIG_BT_OBSERVER 0` in `ble_common.cmake`), so no scanning. Not bl808: neither SDK ships a BL808 BLE controller library.

## Pins
No pins (on-chip BLE radio; antenna on the module).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/ble-scan/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only ble-scan bouffalo`.

## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
```
BLE up, own address xx:xx:xx:xx:xx:xx
ble scan round 1
  AA:BB:CC:DD:EE:FF (random) rssi -60 evt 0 MyDevice
round 1: 7 advertisement(s)
```
`defconfig` enables `CONFIG_SHELL` only because the SDK's blestack always compiles its CLI command file; no shell is started.
