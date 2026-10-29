# {{NAME}}

C++ on bare metal.

Constructors run before main (Arm: __libc_init_array; RISC-V: the startup walks .init_array). Avoid virtual destructors and `new` unless you add a heap.

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP={{TARGET}} && cmake --build build
$MCU_BARE/run/firmware.sh build/{{NAME}}.elf      # qemu / Renode run, passes on RESULT: PASS
```

Made from the mcu template `bare/cxx-app` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare cxx-app <dest> --name N --target T` any of them).
