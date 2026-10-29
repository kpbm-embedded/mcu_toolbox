# ch552-pwm-fade

Fade an external LED with analogWrite() on P3.4 (PWM2) (CH55xDuino 0.0.26, SDCC; plain C inside the .ino).

Pins: CH55xDuino numbers pins as port*10+bit. PWM pins from `mcu/avr/data/packages/CH55xDuino/hardware/mcs51/0.0.26/variants/ch552/pins_arduino.h` (`digital_pin_to_pwm`): pin 15 = P1.5 PWM1 (alt. 30 = P3.0), pin 34 = P3.4 PWM2 (alt. 31 = P3.1);
the core's `Generic_Examples/01.Basics/Fade` also uses 34. The board LED P3.3 has no PWM, so wire an external LED to P3.4.

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b CH55xDuino:mcs51:ch552 --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-pwm-fade/ch552 mcu/examples/avr/ch552-pwm-fade
arduino-cli upload  -b CH55xDuino:mcs51:ch552 --input-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-pwm-fade/ch552   # BOOT held while plugging in (4348:55e0)
```
Default board options: 24 MHz internal, USB setting "Default CDC". Or `mcu/templates/tools/check.sh --only ch552-pwm-fade avr`.

Expected: the LED on P3.4 fades up and down (~10 s cycle).

Status: builds; untested on hardware.
