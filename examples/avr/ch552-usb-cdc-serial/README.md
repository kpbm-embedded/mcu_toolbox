# ch552-usb-cdc-serial

USB CDC serial: echo received bytes and print a heartbeat (CH55xDuino, SDCC; plain C inside the .ino).

Pins: CH55xDuino numbers pins as port*10+bit, so LED 33 = P3.3; the core's own Blink example uses 33 ("simpleCH552" LED), see
`mcu/avr/sketches-ch552/Blink/Blink.ino` and `mcu/avr/data/packages/CH55xDuino/hardware/mcs51/0.0.26/variants/ch552/pins_arduino.h`.
Your board's LED pin may differ; change LED_PIN.

## Build / flash
```
. mcu/avr/env.sh
arduino-cli compile -b CH55xDuino:mcs51:ch552 --output-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-usb-cdc-serial/ch552 mcu/examples/avr/ch552-usb-cdc-serial
arduino-cli upload  -b CH55xDuino:mcs51:ch552 --input-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/ch552-usb-cdc-serial/ch552   # BOOT held while plugging in (4348:55e0)
```
Default board options: 24 MHz internal, USB setting "Default CDC".
Expected: a /dev/ttyACM* appears; the heartbeat line prints every second and typed characters are echoed (core API USBSerial_*, see Generic_Examples 04.Communication/UsbSerialAdaptor).

Status: builds; untested on hardware.
