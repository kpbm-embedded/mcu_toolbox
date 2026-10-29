# ch552-button-irq

Button on P3.2 (INT0, falling edge) toggles the LED on P3.3; press count over USB CDC (CH55xDuino 0.0.26, SDCC; plain C inside the .ino).

Pins: CH55xDuino numbers pins as port*10+bit. Button between P3.2 and GND (internal pull-up enabled). INT0 = P3.2 and INT1 = P3.3 per CH55xDuino `Generic_Examples/02.Digital/AttachInterrupts`;
INT1 is the LED pin on this board, so only INT0 is used. LED 33 = P3.3 as in `mcu/avr/sketches-ch552/Blink/Blink.ino`.
The button pin is unverified for your board (many CH552 boards have only a BOOT button, usually on P3.6/UDP: do not use that).

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b CH55xDuino:mcs51:ch552 --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-button-irq/ch552 mcu/examples/avr/ch552-button-irq
arduino-cli upload  -b CH55xDuino:mcs51:ch552 --input-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-button-irq/ch552   # BOOT held while plugging in (4348:55e0)
```
Default board options: 24 MHz internal, USB setting "Default CDC". Or `mcu/templates/tools/check.sh --only ch552-button-irq avr`.

Expected: every press toggles the LED and prints `button N` on the USB CDC port (/dev/ttyACM*). No debouncing: one press may count more than once.

Status: builds; untested on hardware.
