# baremetal-minimal (avr)

avr-gcc Makefile project (no Arduino core). `board.h` maps the target to LED pin and register aliases.
Targets are selected with `TARGET`: nano / nano-old (ATmega328P 16 MHz, PB5; nano-old only differs in the bootloader baud, flash with -b 57600), mega (ATmega2560 16 MHz, PB7), attiny85 (8 MHz internal, PB1).

```
. mcu/avr/env.sh
make TARGET=nano          # output $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/avr/{{NAME}}/nano/{{NAME}}.hex (override with OUT=)
make TARGET=nano flash PROG=arduino PORT=/dev/ttyUSB0
```
Edit `main.c`. Pin sources: Arduino avr core variants (standard D13 = PB5, mega D13 = PB7), ATTinyCore tinyX5 (LED_BUILTIN 1 = PB1).
