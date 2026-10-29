# i2c-scan

Scan the I2C bus and print the addresses that ACK (Arduino sketch; the folder name equals the .ino name as arduino-cli requires).

Expected: `found 0x3C` style lines or `no devices`, every 3 s (ATtiny85: 9600 baud on PB3).

Pin sources: `mcu/avr/data/packages/arduino/hardware/avr/1.8.8/variants/{standard,mega}/pins_arduino.h` (Nano: D13 = PB5 LED, A4/A5 = SDA/SCL;
Mega: D13 = PB7 LED, D20/D21 = SDA/SCL, D2 = INT4) and `mcu/avr/data/packages/ATTinyCore/hardware/avr/1.5.2/variants/tinyX5/pins_arduino.h`
(ATtiny85: pin 1 = PB1 = LED_BUILTIN and OC0B PWM, pin 2 = PB2 = INT0, pin 3 = PB3 = ADC3).

ATtiny85 (ATTinyCore): `Wire` uses the USI, SDA = PB0, SCL = PB2 (`variants/tinyX5/pins_arduino.h`: PIN_USI_SDA PINB0,
PIN_USI_SCL PINB2). The builtin software `Serial` would transmit on PB0 too, so the sketch calls `Serial.setTxBit(3)`
(ATTinyCore 1.5 ChangeLog) and prints on PB3 at 9600 baud. Connect a USB-UART RX to PB3.

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b <fqbn> --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/i2c-scan/<target> mcu/examples/avr/i2c-scan
```
FQBNs (from mcu/avr/sdk.sh): nano arduino:avr:nano:cpu=atmega328, nano-old arduino:avr:nano:cpu=atmega328old,
mega arduino:avr:mega:cpu=atmega2560, attiny85 ATTinyCore:avr:attinyx5:chip=85,clock=8internal.
Upload: `arduino-cli upload -b <fqbn> -p /dev/ttyUSB0 --input-dir <out>` (ATtiny85 needs an ISP programmer). Monitor: `mcu/mcu monitor avr /dev/ttyUSB0 115200`.

Status: builds; untested on hardware.
