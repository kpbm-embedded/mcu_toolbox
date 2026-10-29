# {{NAME}}

Rust + C firmware.

Rust: `src/lib.rs` (`#![no_std]`, `#[no_mangle] extern "C"` functions). For a Rust-only firmware with cortex-m-rt, embassy or RTIC use 32_17_embedded_rust (`rust-box new`).

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/rust-app` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare rust-app <dest> --name N --target T` any of them).
