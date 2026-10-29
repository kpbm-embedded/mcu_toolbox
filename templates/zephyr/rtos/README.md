# {{NAME}} (rtos)

Zephyr app template (out of tree, Zephyr v4.4.2). Placeholders already filled: name `{{NAME}}`, target `{{TARGET}}`, west board `{{BOARD}}`.

Producer and consumer threads (`K_THREAD_DEFINE`) exchange samples over a `K_MSGQ`; a `K_MUTEX` guards a shared counter; the consumer toggles `led0`.

## Build / flash
```
. $BOX/30_39_tools/32_embedded/32_03_zephyr/toolchain/zephyr/env.sh
west build -p auto -b {{BOARD}} -d build .
west flash -d build        # promicro_nrf52840: copy build/zephyr/zephyr.uf2 to the bootloader drive; rpi_pico2: uf2 or openocd
```
Other board targets: blackpill_f411ce, weact_g431, weact_h562, rpi_pico2, promicro_nrf52840 (board names in `../mcu/zephyr/sdk.sh`). Per-board tweaks go in `boards/<board>.overlay` / `boards/<board>.conf`, never in the Zephyr tree.
Console: board console UART at 115200 (see mcu/zephyr/README.md for the pins). Status: builds; untested on hardware.
