# {{NAME}}

Firmware with host unit tests.

Put hardware-free logic in `lib/`, test it in `tests/` on the PC, call it from `src/` on the chip.

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S tests -B build-host && cmake --build build-host && ctest --test-dir build-host   # host
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build && $MCU_BARE/run/firmware.sh build/{{NAME}}.elf
```

Made from the mcu template `bare/fw-host-tests` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare fw-host-tests <dest> --name N --target T` any of them).
