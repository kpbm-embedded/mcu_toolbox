# bootloader-app

Two images in one flash: a bootloader at 0x0 checks the CRC-32 of an app at 0x10000, then sets VTOR/MSP and jumps.

Chips: mps2-an385 (`mcu targets bare`). The app is linked at 0x10000 (app.ld), its .bin gets a header (size, CRC) and is embedded into the bootloader image with .incbin, so one ELF holds both.

## Build and run

```sh
templates/tools/check.sh --only bootloader-app bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare mps2-an385 bootloader-app          # one chip
$MCU_BARE/run/firmware.sh <build>/bootloader-app.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
