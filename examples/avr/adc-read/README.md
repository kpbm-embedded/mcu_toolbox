# adc-read

Read A0 and print the raw value and millivolts; ATmega328P also prints the internal temperature sensor raw value (Arduino sketch; the folder name equals the .ino name as arduino-cli requires).

Expected: `A0=512 2502 mV` lines (ATtiny85: `A3=...` at 9600 baud), plus `temp_raw=...` on Nano (uncalibrated: T ~ (raw-324.31)/1.22 C).

Pin sources: `mcu/avr/data/packages/arduino/hardware/avr/1.8.8/variants/{standard,mega}/pins_arduino.h` (Nano: D13 = PB5 LED, A4/A5 = SDA/SCL;
Mega: D13 = PB7 LED, D20/D21 = SDA/SCL, D2 = INT4) and `mcu/avr/data/packages/ATTinyCore/hardware/avr/1.5.2/variants/tinyX5/pins_arduino.h`
(ATtiny85: pin 1 = PB1 = LED_BUILTIN and OC0B PWM, pin 2 = PB2 = INT0, pin 3 = PB3 = ADC3).

ATtiny85 (ATTinyCore): A0 is PB5 = RESET, so the sketch reads A3 = PB3 (ADC3) and prints with the core's builtin software
`Serial` (no UART on the chip): TX = AIN0 = PB0, 9600 baud (half duplex; source: ATTinyCore 1.5.2 README "TX is AIN0, RX is
AIN1" and `variants/tinyX5/pins_arduino.h`). Connect a USB-UART RX to PB0.

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b <fqbn> --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/adc-read/<target> mcu/examples/avr/adc-read
```
FQBNs (from mcu/avr/sdk.sh): nano arduino:avr:nano:cpu=atmega328, nano-old arduino:avr:nano:cpu=atmega328old,
mega arduino:avr:mega:cpu=atmega2560, attiny85 ATTinyCore:avr:attinyx5:chip=85,clock=8internal.
Upload: `arduino-cli upload -b <fqbn> -p /dev/ttyUSB0 --input-dir <out>` (ATtiny85 needs an ISP programmer). Monitor: `mcu/mcu monitor avr /dev/ttyUSB0 115200`.

Status: builds; untested on hardware.
