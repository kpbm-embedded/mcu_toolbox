# noinit-reboot

Linker sections at work: .noinit survives a software reset (boot counter), code placed in RAM (.ramfunc), .data/.bss.

Chips: mps2-an385 (`mcu targets bare`). The section layout comes from chips/common/cm.ld.in; test.sh checks the addresses in the ELF.

## Build and run

```sh
templates/tools/check.sh --only noinit-reboot bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare mps2-an385 noinit-reboot          # one chip
$MCU_BARE/run/firmware.sh <build>/noinit-reboot.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
