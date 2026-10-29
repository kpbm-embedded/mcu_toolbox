# {{NAME}}

nice!nano app for the UF2 bootloader.

Double-tap reset on the board, then copy `build/{{NAME}}.uf2` to the NICENANO drive (not tested here: no board). Linked at 0x26000 because SoftDevice S140 sits below.

## Build and run (inside `mcu shell bare`, or `. <mcu>/toolchain/bare/env.sh`)

```sh
cmake -S . -B build -G Ninja -DCHIP=nrf52840 && cmake --build build   # -> build/{{NAME}}.uf2
bash test.sh nrf52840 build                                             # UF2 header check
```

Made from the mcu template `bare/nrf52840-uf2` (`mcu new bare {{NAME}} {{TARGET}}` gives the `minimal` one;
`python3 -I templates/tools/instantiate.py bare nrf52840-uf2 <dest> --name N --target T` any of them).
