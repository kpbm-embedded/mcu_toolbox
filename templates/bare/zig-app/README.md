# {{NAME}}

Zig + C firmware.

Zig: `src/logic.zig` (`export fn` = C ABI). For Zig-only firmware with MicroZig HALs use 32_24_microzig (`microzig-box`).

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/zig-app` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare zig-app <dest> --name N --target T` any of them).
