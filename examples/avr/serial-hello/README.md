# serial-hello

Print a counter over the hardware UART at 115200 baud (Arduino sketch; the folder name equals the .ino name as arduino-cli requires).

Expected: `hello 0`, `hello 1`, ... once per second (ATtiny85: 9600 baud).

Pin sources: `mcu/avr/data/packages/arduino/hardware/avr/1.8.8/variants/{standard,mega}/pins_arduino.h` (Nano: D13 = PB5 LED, A4/A5 = SDA/SCL;
Mega: D13 = PB7 LED, D20/D21 = SDA/SCL, D2 = INT4) and `mcu/avr/data/packages/ATTinyCore/hardware/avr/1.5.2/variants/tinyX5/pins_arduino.h`
(ATtiny85: pin 1 = PB1 = LED_BUILTIN and OC0B PWM, pin 2 = PB2 = INT0, pin 3 = PB3 = ADC3).

ATtiny85 (ATTinyCore): no UART; the core's builtin software `Serial` sends on AIN0 = PB0 (RX AIN1 = PB1), 9600 baud,
half duplex (ATTinyCore 1.5.2 README). Connect a USB-UART RX to PB0. Note PB1 is also LED_BUILTIN.

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b <fqbn> --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/serial-hello/<target> mcu/examples/avr/serial-hello
```
FQBNs (from mcu/avr/sdk.sh): nano arduino:avr:nano:cpu=atmega328, nano-old arduino:avr:nano:cpu=atmega328old,
mega arduino:avr:mega:cpu=atmega2560, attiny85 ATTinyCore:avr:attinyx5:chip=85,clock=8internal.
Upload: `arduino-cli upload -b <fqbn> -p /dev/ttyUSB0 --input-dir <out>` (ATtiny85 needs an ISP programmer). Monitor: `mcu/mcu monitor avr /dev/ttyUSB0 115200`.

Status: builds; untested on hardware.
