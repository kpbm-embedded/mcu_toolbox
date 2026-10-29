# arduino-minimal (avr)

Sketch skeleton for Nano (ATmega328P, new/old bootloader), Mega 2560 and ATtiny85. Edit `{{NAME}}.ino`.
Keep the destination folder name equal to NAME: arduino-cli requires folder == .ino name.

Placeholders: `{{NAME}}`, `{{TARGET}}`, `{{LED_PIN}}` (13 on Nano/Mega, 1 = PB1 on ATtiny85, ATTinyCore `LED_BUILTIN`).

```
. mcu/avr/env.sh
arduino-cli compile -b arduino:avr:nano:cpu=atmega328 --output-dir <out> <dest>     # see mcu/avr/sdk.sh sdk_targets for the other FQBNs
mcu/mcu build avr nano <dest>                                                          # same, via the plugin (dest = sketch dir)
```
