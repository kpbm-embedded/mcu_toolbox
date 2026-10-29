# blink

Blink the on-board LED once per second (Arduino sketch; the folder name equals the .ino name as arduino-cli requires).

Expected: LED on 1 s / off 1 s.

Pin sources: `mcu/avr/data/packages/arduino/hardware/avr/1.8.8/variants/{standard,mega}/pins_arduino.h` (Nano: D13 = PB5 LED, A4/A5 = SDA/SCL;
Mega: D13 = PB7 LED, D20/D21 = SDA/SCL, D2 = INT4) and `mcu/avr/data/packages/ATTinyCore/hardware/avr/1.5.2/variants/tinyX5/pins_arduino.h`
(ATtiny85: pin 1 = PB1 = LED_BUILTIN and OC0B PWM, pin 2 = PB2 = INT0, pin 3 = PB3 = ADC3).

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b <fqbn> --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/blink/<target> mcu/examples/avr/blink
```
FQBNs (from mcu/avr/sdk.sh): nano arduino:avr:nano:cpu=atmega328, nano-old arduino:avr:nano:cpu=atmega328old,
mega arduino:avr:mega:cpu=atmega2560, attiny85 ATTinyCore:avr:attinyx5:chip=85,clock=8internal.
Upload: `arduino-cli upload -b <fqbn> -p /dev/ttyUSB0 --input-dir <out>` (ATtiny85 needs an ISP programmer). Monitor: `mcu/mcu monitor avr /dev/ttyUSB0 115200`.

Status: builds; untested on hardware.
