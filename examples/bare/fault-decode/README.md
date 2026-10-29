# fault-decode

UsageFault decoding: divide-by-zero and unaligned-access traps, CFSR bits and the stacked PC, then resume.

Chips: mps2-an385 (`mcu targets bare`). qemu models DIV_0_TRP and UNALIGN_TRP of the Cortex-M3; the same handler works on any ARMv7-M chip.

## Build and run

```sh
templates/tools/check.sh --only fault-decode bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare mps2-an385 fault-decode          # one chip
$MCU_BARE/run/firmware.sh <build>/fault-decode.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
