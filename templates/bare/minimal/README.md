# {{NAME}}

Minimal bare-metal firmware for {{TARGET}}.

Chip layer: `$MCU_BARE/chips/{{TARGET}}/` (startup, linker script, console UART). Other chips: `-DCHIP=...` (see `mcu targets bare`).

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/minimal` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare minimal <dest> --name N --target T` any of them).
