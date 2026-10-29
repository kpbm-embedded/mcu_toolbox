# {{NAME}}

Two-image layout: bootloader at 0x0, application slot at 0x10000.

Change the slot address in boot.ld, app.ld and src/boot.c together. On a real chip the slot is a flash page boundary.

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/bootloader-app` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare bootloader-app <dest> --name N --target T` any of them).
