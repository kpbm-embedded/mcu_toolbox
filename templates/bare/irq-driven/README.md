# {{NAME}}

Interrupt-driven skeleton (SysTick + PendSV + WFI).

Core peripherals only, so it is the same on every Cortex-M chip; CORE_HZ is the reset clock per chip.

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/irq-driven` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare irq-driven <dest> --name N --target T` any of them).
