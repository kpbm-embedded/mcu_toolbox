# {{NAME}}

Firmware with its own startup code and linker script.

For new chips or when the vendor startup does not fit: edit `link.ld` (memory map) and the vector table in `src/startup.c`.

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/own-startup` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare own-startup <dest> --name N --target T` any of them).
