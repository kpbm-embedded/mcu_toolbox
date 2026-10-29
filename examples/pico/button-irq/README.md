# pico example: button-irq

Status: **builds; untested on hardware**.

A falling edge on GP14 (button to GND) toggles the LED from the GPIO interrupt handler; the main loop prints the press count.

| Board (target) | LED | notes |
|---|---|---|
| Pico (`pico`), Pico 2 (`pico2`, `pico2-riscv`) | GPIO25 on-board LED | `PICO_DEFAULT_LED_PIN` in `pico-sdk/src/boards/include/boards/pico.h` / `pico2.h` |
| RP2040-Zero (`zero`), RP2350-Zero (`rp2350-zero`, `rp2350-zero-riscv`) | none; external LED+resistor on GP15 | board headers `waveshare_rp2040_zero.h` / `waveshare_rp2350_zero.h` define no plain LED; the only on-board LED is the WS2812 on GPIO16 (Waveshare wiki: https://www.waveshare.com/wiki/RP2040-Zero and https://www.waveshare.com/wiki/RP2350-Zero) |

Button: external switch GP14 to GND (board.h `BUTTON_PIN`); neither Zero board exposes BOOT as a GPIO.

## Build

```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/button-irq/<target> \
  -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/button-irq/<target> -j16
```

Targets map to PICO_BOARD like `mcu/pico/sdk.sh`: pico=pico, zero=waveshare_rp2040_zero, pico2=pico2, pico2-riscv=pico2+rp2350-riscv (toolchain `$PICO_TOOLCHAIN_PATH_RISCV`), rp2350-zero=waveshare_rp2350_zero, rp2350-zero-riscv=waveshare_rp2350_zero+rp2350-riscv.
Flash: hold BOOTSEL while plugging in, then `picotool load -x button-irq.uf2` or copy the .uf2 to the drive. Monitor: `../mcu monitor pico /dev/ttyACM0`.
