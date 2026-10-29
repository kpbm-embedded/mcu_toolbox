# timer-interrupt

Timer0 CTC 1 kHz interrupt toggles the LED every 500 ms (bare metal, avr-gcc, no Arduino core).

| target | MCU / clock | LED |
|---|---|---|
| nano | ATmega328P, 16 MHz | PB5 (D13) |
| nano-old | ATmega328P, 16 MHz (old bootloader: flash with -b 57600) | PB5 (D13) |
| mega | ATmega2560, 16 MHz | PB7 (D13) |
| attiny85 | ATtiny85, 8 MHz internal | PB1 |

Pin sources: Arduino core variants `standard/pins_arduino.h` (D13 = PB5) and `mega/pins_arduino.h` (D13 = PB7) in
`mcu/avr/data/packages/arduino/hardware/avr/1.8.8/variants/`; ATTinyCore `variants/tinyX5/pins_arduino.h` (`LED_BUILTIN 1` = PB1) in
`mcu/avr/data/packages/ATTinyCore/hardware/avr/1.5.2/`. The ATtiny85 needs a fuse setting of 8 MHz internal (CKDIV8 cleared).

## Build / flash
```
. mcu/avr/env.sh
make TARGET=nano        # or nano-old, mega, attiny85; output $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/timer-interrupt/<target>/
make TARGET=nano flash PROG=arduino PORT=/dev/ttyUSB0
```
Expected: LED blinks at 1 Hz, driven entirely from the TIMER0_COMPA ISR.

Status: builds; untested on hardware.
