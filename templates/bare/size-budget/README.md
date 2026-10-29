# {{NAME}}

Firmware with a size budget.

Run `bash test.sh <chip> build` in CI: it fails when text or data+bss exceed budget.txt.

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/size-budget` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare size-budget <dest> --name N --target T` any of them).
