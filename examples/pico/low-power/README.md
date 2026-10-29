# pico example: low-power

Status: **builds; untested on hardware**.

Uses `pico_low_power` (new in pico-sdk 2.3; pico-extras is not needed and not installed). Each round: LED on and awake for 2 s, then **sleep** for 5 s (clocks gated, woken by the system timer via `low_power_sleep_for_ms`), then **dormant** (crystal/ring oscillators stopped) until the button on GP14 is pressed (`low_power_dormant_until_gpio_pin_state`, falling edge). Dormant clock source as in `pico-examples/low_power/low_power_dormant`: XOSC on RP2040, LPOSC on RP2350. USB is stopped in dormant, so the console is UART0.

| Board (target) | LED | notes |
|---|---|---|
| Pico (`pico`), Pico 2 (`pico2`, `pico2-riscv`) | GPIO25 on-board LED | `PICO_DEFAULT_LED_PIN` in `pico-sdk/src/boards/include/boards/pico.h` / `pico2.h` |
| RP2040-Zero (`zero`), RP2350-Zero (`rp2350-zero`, `rp2350-zero-riscv`) | none; external LED+resistor on GP15 | board headers `waveshare_rp2040_zero.h` / `waveshare_rp2350_zero.h` define no plain LED; the only on-board LED is the WS2812 on GPIO16 (Waveshare wiki: https://www.waveshare.com/wiki/RP2040-Zero and https://www.waveshare.com/wiki/RP2350-Zero) |

Button: external switch from GP14 to GND (board.h `BUTTON_PIN`, internal pull-up; the pull-up stays on, the SDK only removes pulls in `low_power_set_pins_low_leakage*`, which is not called). Console: UART0 TX GP0 / RX GP1, 115200 8N1.

Rough currents from `pico_low_power/include/pico/low_power.h` (SDK measurements, VSYS): sleep 7.3 mA (Pico) / 5.9 mA (Pico 2), dormant 0.95 mA / 3.3 mA. The RP2350 power-state (Pstate) mode (~0.2 mA, reboots on wake) is not used here; see `pico-examples/low_power/low_power_pstate`.

Expected:

```
run 1: awake 2 s
sleep 5 s
woke from sleep (rc=0)
dormant until GP14 goes low (press the button)
woke from dormant (rc=0)
run 2: awake 2 s
```

## Build

```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/low-power/<target> \
  -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/low-power/<target> -j16
```

Targets map to PICO_BOARD like `mcu/pico/sdk.sh`: pico=pico, zero=waveshare_rp2040_zero, pico2=pico2, pico2-riscv=pico2+rp2350-riscv (toolchain `$PICO_TOOLCHAIN_PATH_RISCV`), rp2350-zero=waveshare_rp2350_zero, rp2350-zero-riscv=waveshare_rp2350_zero+rp2350-riscv.
Flash: hold BOOTSEL while plugging in, then `picotool load -x low-power.uf2` or copy the .uf2 to the drive. Monitor (UART0 via a USB-UART adapter on GP0/GP1): `picocom -b 115200 /dev/ttyUSB0`.
