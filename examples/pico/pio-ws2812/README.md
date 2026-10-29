# pico example: pio-ws2812

Status: **builds; untested on hardware**.

A PIO state machine clocks out WS2812 data on GPIO16 and cycles a dimmed colour wheel on the on-board RGB LED (Zero boards) or on the first pixel of an external strip (Pico / Pico 2).

Pin source: `PICO_DEFAULT_WS2812_PIN 16` in `pico-sdk/src/boards/include/boards/waveshare_rp2040_zero.h` and `waveshare_rp2350_zero.h`; Waveshare wikis (RP2040-Zero, RP2350-Zero) show the on-board WS2812 on GP16.

| Board (target) | WS2812 data | source |
|---|---|---|
| RP2040-Zero (`zero`), RP2350-Zero (`rp2350-zero`, `rp2350-zero-riscv`) | on-board, GPIO16 | `PICO_DEFAULT_WS2812_PIN` in the board header |
| Pico (`pico`), Pico 2 (`pico2`, `pico2-riscv`) | external strip DIN on GP16, strip GND to Pico GND, strip power from VBUS (5 V) or 3V3 | `pico.h` / `pico2.h` define no WS2812 pin; GP16 is the fallback `#define` in `main.c` (free GPIO on the Pico pinout). Many 5 V strips accept 3.3 V data; if not, add a level shifter (unverified per strip) |

Override the pin with `-DPICO_DEFAULT_WS2812_PIN=<n>` (add it to the cmake line, or `add_compile_definitions`).

## Build

```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/pio-ws2812/<target> \
  -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/pio-ws2812/<target> -j16
```

Targets map to PICO_BOARD like `mcu/pico/sdk.sh`: pico=pico, zero=waveshare_rp2040_zero, pico2=pico2, pico2-riscv=pico2+rp2350-riscv (toolchain `$PICO_TOOLCHAIN_PATH_RISCV`), rp2350-zero=waveshare_rp2350_zero, rp2350-zero-riscv=waveshare_rp2350_zero+rp2350-riscv.
Flash: hold BOOTSEL while plugging in, then `picotool load -x pio-ws2812.uf2` or copy the .uf2 to the drive. Monitor: `../mcu monitor pico /dev/ttyACM0`.
