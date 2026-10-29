# ch552-timer

Timer2 auto-reload interrupt every 10 ms toggles the LED every 500 ms; counter over USB CDC (CH55xDuino 0.0.26, SDCC; plain C inside the .ino).

Pins: CH55xDuino numbers pins as port*10+bit. LED 33 = P3.3. Timer2 registers (T2CON, T2MOD.bT2_CLK, RCAP2L/H, ET2, TF2) from `variants/ch552/include/ch5xx.h`; Timer0 stays
with the core (millis). The sketch overrides the core's empty `Timer2Interrupt()` (`cores/ch55xduino/weakOverrideTimer2.c`).

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b CH55xDuino:mcs51:ch552 --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-timer/ch552 mcu/examples/avr/ch552-timer
arduino-cli upload  -b CH55xDuino:mcs51:ch552 --input-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-timer/ch552   # BOOT held while plugging in (4348:55e0)
```
Default board options: 24 MHz internal, USB setting "Default CDC". Or `mcu/templates/tools/check.sh --only ch552-timer avr`.

Expected: LED toggles every 500 ms; `half-seconds N` on the USB CDC port.

Status: builds; untested on hardware.
