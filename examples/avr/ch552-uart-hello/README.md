# ch552-uart-hello

UART0 (Serial0) TX P3.1 / RX P3.0 at 115200: greeting, echo and a counter per second (CH55xDuino 0.0.26, SDCC; plain C inside the .ino).

Pins: CH55xDuino numbers pins as port*10+bit. UART0 = P3.0 RXD / P3.1 TXD (`variants/ch552/include/ch5xx.h`: `SBIT(TXD, 0xB0, 1)`, `SBIT(RXD, 0xB0, 0)`, 0xB0 = P3; API `Serial0_*` in `cores/ch55xduino/HardwareSerial.h`).
The USB connector stays the CH55xDuino default CDC (uploads/auto-reset); the UART needs a 3.3/5 V USB-UART adapter.

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b CH55xDuino:mcs51:ch552 --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-uart-hello/ch552 mcu/examples/avr/ch552-uart-hello
arduino-cli upload  -b CH55xDuino:mcs51:ch552 --input-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-uart-hello/ch552   # BOOT held while plugging in (4348:55e0)
```
Default board options: 24 MHz internal, USB setting "Default CDC". Or `mcu/templates/tools/check.sh --only ch552-uart-hello avr`.

Expected: `hello from CH552 UART0`, then `alive N` every second at 115200 8N1; typed characters come back as `echo: x`.

Status: builds; untested on hardware.
