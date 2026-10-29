# ringbuf-split

Unit-test + firmware split: lib/ring buffer tested on the host (ctest) and the same code run on Arm and RISC-V in qemu.

Chips: mps2-an385, virt-rv32 (`mcu targets bare`). `lib/` has no hardware access; `tests/` builds it with the host compiler (check.sh runs ctest); `src/` uses it on the chip.

## Build and run

```sh
templates/tools/check.sh --only ringbuf-split bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare mps2-an385 ringbuf-split          # one chip
$MCU_BARE/run/firmware.sh <build>/ringbuf-split.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
