# ch552-minimal (avr)

CH55xDuino (SDCC) sketch for the CH552T. Folder name must equal NAME (arduino-cli rule). LED pin 33 = P3.3 as in the core's Blink example; adjust for your board.
```
. mcu/avr/env.sh
arduino-cli compile -b CH55xDuino:mcs51:ch552 --output-dir <out> <dest>
```
Upload with BOOT held while plugging in (USB bootloader 4348:55e0). USB setting "Default CDC" (board default) provides `USBSerial_*`.
