# {{NAME}}

ESP-IDF project created from the `minimal` template of the mcu workspace. Target: `{{TARGET}}` (board `{{BOARD}}`).


## Build, flash, monitor
```
source <mcu>/esp-idf/env.sh
idf.py -B build -DIDF_TARGET={{TARGET}} build
idf.py -B build -p /dev/ttyACM0 flash monitor
```
Keep the build dir outside the source tree if you prefer: `-B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/projects/{{NAME}}` with `-DSDKCONFIG=<that>/sdkconfig`.
`sdkconfig.defaults(.<target>)` hold your config; the generated `sdkconfig` is ignored. On native-USB boards (S3-Zero, C3 SuperMini/Zero) the console is USB-Serial-JTAG.

Start editing in `main/main.c`. The LED pin is a per-target default ({{LED_PIN}}); verify it for your board (pins for WT9932P4-TINY unknown). ESP32 WROOM-32 CH340C: GPIO2 = onboard LED on most CH340 clones (unverified); console on UART0 via the CH340 (/dev/ttyUSB0).
