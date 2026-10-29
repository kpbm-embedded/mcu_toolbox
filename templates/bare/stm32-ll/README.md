# {{NAME}}

STM32 LL (Low-Layer) drivers on the bare chip layer.

LL sources and CMSIS device headers come from the installed Cube pack (`toolchain/stm32/STM32Cube<F>`); no HAL, no generated code. Renode runs it (STM32F412/F103/H743 models).

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/stm32-ll` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare stm32-ll <dest> --name N --target T` any of them).
