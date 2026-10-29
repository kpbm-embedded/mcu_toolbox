# deep-sleep

Deep sleep with timer wake and an RTC-retained counter.

Status: builds; untested on hardware.

Hardware: none; measure current on the 3V3/5V rail to see sleep current.

## Build, flash, monitor
```
source mcu/esp-idf/env.sh
T=esp32c3        # one of: esp32, esp32s3, esp32c3, esp32c6, esp32p4
idf.py -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/deep-sleep/$T -DSDKCONFIG=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/deep-sleep/$T/sdkconfig -DIDF_TARGET=$T build
idf.py -B <same build dir> -p /dev/ttyACM0 flash monitor
```
Hold BOOT and reset for download mode on native-USB boards.
ESP32 WROOM-32 CH340C: console on UART0 via the CH340, port /dev/ttyUSB0 (auto-reset into download mode via DTR/RTS).

Expected: Device reboots every ~5 s, counter increments (USB port re-enumerates on native-USB boards).
