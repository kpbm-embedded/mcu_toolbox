# size-budget

Flash/RAM budget check of a small-chip firmware: size per section against limits in budget.txt, biggest symbols listed.

Chips: ch32v003, stm32f103 (`mcu targets bare`). test.sh compares `size` of the ELF with budget.txt and fails the build when a section grows past its limit; it also prints the 5 largest symbols.

## Build and run

```sh
templates/tools/check.sh --only size-budget bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare ch32v003 size-budget          # one chip
$MCU_BARE/run/firmware.sh <build>/size-budget.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
