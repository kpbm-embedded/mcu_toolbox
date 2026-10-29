# cxx-bare

C++ without an OS: static constructors (init_array), virtual functions, templates and placement new; no exceptions/RTTI/heap.

Chips: mps2-an385, virt-rv32, stm32f411 (`mcu targets bare`). Built with -fno-exceptions -fno-rtti; the Arm startup runs constructors via __libc_init_array, the RISC-V startup walks __init_array_start..end itself.

## Build and run

```sh
templates/tools/check.sh --only cxx-bare bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare mps2-an385 cxx-bare          # one chip
$MCU_BARE/run/firmware.sh <build>/cxx-bare.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
