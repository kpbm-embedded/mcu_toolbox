# ch552-adc-read

Read the 8-bit ADC on P1.1 (AIN0) and print raw and mV over USB CDC (CH55xDuino 0.0.26, SDCC; plain C inside the .ino).

Pins: CH55xDuino numbers pins as port*10+bit. ADC pins from `mcu/avr/data/packages/CH55xDuino/hardware/mcs51/0.0.26/variants/ch552/pins_arduino.h` (`digital_pin_to_analog`): 11 = P1.1 AIN0, 14 = P1.4 AIN1, 15 = P1.5 AIN2, 32 = P3.2 AIN3. The CH552
ADC is 8-bit with VDD as reference; mV assumes 5 V (the 24 MHz clock option requires 5 V, boards.txt).

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b CH55xDuino:mcs51:ch552 --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-adc-read/ch552 mcu/examples/avr/ch552-adc-read
arduino-cli upload  -b CH55xDuino:mcs51:ch552 --input-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-adc-read/ch552   # BOOT held while plugging in (4348:55e0)
```
Default board options: 24 MHz internal, USB setting "Default CDC". Or `mcu/templates/tools/check.sh --only ch552-adc-read avr`.

Expected: `P1.1 raw 128 = 2509 mV` lines twice a second on the USB CDC port.

Status: builds; untested on hardware.
