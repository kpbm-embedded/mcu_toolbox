# usb-cdc

USB-Serial-JTAG as a CDC serial port: echo via the usb_serial_jtag driver.

Status: builds; untested on hardware.

Uses the fixed USB-Serial-JTAG CDC function of the chip, not TinyUSB (TinyUSB is a managed component and would need the component registry).

Hardware: Native USB connector (S3-Zero, C3 SuperMini/Zero, C6 and P4 USB-Serial-JTAG pins); appears as /dev/ttyACM*.

## Build, flash, monitor
```
source mcu/esp-idf/env.sh
T=esp32c3        # one of: esp32s3, esp32c3, esp32c6, esp32p4
idf.py -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/usb-cdc/$T -DSDKCONFIG=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/usb-cdc/$T/sdkconfig -DIDF_TARGET=$T build
idf.py -B <same build dir> -p /dev/ttyACM0 flash monitor
```
Hold BOOT and reset for download mode on native-USB boards.
Not for classic ESP32 (WROOM-32): it has no USB peripheral; its USB port is the CH340 bridge on UART0 (use uart-hello / the console).

Expected: Open the ttyACM port: banner, then your typing is echoed.
