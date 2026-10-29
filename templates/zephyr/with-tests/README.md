# {{NAME}} (with-tests)

Zephyr app template (out of tree, Zephyr v4.4.2). Placeholders already filled: name `{{NAME}}`, target `{{TARGET}}`, west board `{{BOARD}}`.

App in `src/` (logic in `blink_logic.c`, hardware in `main.c`). Unit tests in `tests/` use ztest and run on the host with `native_sim`:
```
west build -p auto -b native_sim/native/64 -d build-test tests && ./build-test/zephyr/zephyr.exe
```
or `west twister -T tests -p native_sim/native/64`. The board build above compiles only the app.

## Build / flash
```
. $BOX/30_39_tools/32_embedded/32_03_zephyr/toolchain/zephyr/env.sh
west build -p auto -b {{BOARD}} -d build .
west flash -d build        # promicro_nrf52840: copy build/zephyr/zephyr.uf2 to the bootloader drive; rpi_pico2: uf2 or openocd
```
Other board targets: blackpill_f411ce, weact_g431, weact_h562, rpi_pico2, promicro_nrf52840 (board names in `../mcu/zephyr/sdk.sh`). Per-board tweaks go in `boards/<board>.overlay` / `boards/<board>.conf`, never in the Zephyr tree.
Console: board console UART at 115200 (see mcu/zephyr/README.md for the pins). Status: builds; untested on hardware.
