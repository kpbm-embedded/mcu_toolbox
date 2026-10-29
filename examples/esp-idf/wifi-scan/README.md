# wifi-scan

Station-mode Wi-Fi scan, prints SSID/RSSI/channel.

Status: builds; untested on hardware.

Hardware: none (on-chip radio; ESP32-P4 has no Wi-Fi, so not a target).

## Build, flash, monitor
```
source mcu/esp-idf/env.sh
T=esp32c3        # one of: esp32, esp32s3, esp32c3, esp32c6
idf.py -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/wifi-scan/$T -DSDKCONFIG=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/wifi-scan/$T/sdkconfig -DIDF_TARGET=$T build
idf.py -B <same build dir> -p /dev/ttyACM0 flash monitor
```
Hold BOOT and reset for download mode on native-USB boards.
ESP32 WROOM-32 CH340C: console on UART0 via the CH340, port /dev/ttyUSB0 (auto-reset into download mode via DTR/RTS).

Expected: AP list every 5 s.
