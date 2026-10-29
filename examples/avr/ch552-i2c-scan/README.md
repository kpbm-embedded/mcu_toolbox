# ch552-i2c-scan

Bit-banged I2C scan of 0x08..0x77 every 3 s with the core's SoftI2C library (the CH552 has no I2C peripheral), results
on the USB CDC port (CH55xDuino 0.0.26, SDCC; plain C inside the .ino).

Pins: CH55xDuino numbers pins as port*10+bit. SCL 30 = P3.0, SDA 31 = P3.1 as in
`mcu/avr/data/packages/CH55xDuino/hardware/mcs51/0.0.26/libraries/SoftI2C/examples/i2cScanner`; any GPIO works (change
SCL_PIN/SDA_PIN). These are also the UART0 pins, so do not combine with `ch552-uart-hello`. Use 4.7k pull-ups.

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b CH55xDuino:mcs51:ch552 --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-i2c-scan/ch552 mcu/examples/avr/ch552-i2c-scan
arduino-cli upload  -b CH55xDuino:mcs51:ch552 --input-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-i2c-scan/ch552   # BOOT held while plugging in (4348:55e0)
```
Default board options: 24 MHz internal, USB setting "Default CDC". Or `mcu/templates/tools/check.sh --only ch552-i2c-scan avr`.

Expected: `i2c-scan:`, `  found 0x3C`, `1 device(s)` on /dev/ttyACM*.

Status: builds; untested on hardware.
