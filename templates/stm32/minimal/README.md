# stm32 / minimal

Blinks the board LED ({{LED_PIN}} on the default target) and prints a heartbeat on USART1 (PA9 TX, PA10 RX, 115200 8N1) at {{SYSCLK_MHZ}} MHz SYSCLK.
The clock tree, LED/button pins, linker script and HAL configuration come from the shared board layer `mcu/stm32/boards/<board>/` (pin and clock sources are cited in each `board.cmake`).

## Targets
`blackpill-f411` (WeAct BlackPill F411CEU6), `weact-g431` (WeAct G431CBT6 core), `mini-h743` (H743VIT6 min-system), `bluepill-f103` (F103C8T6).

## Build
```
. mcu/stm32/env.sh
cmake -S . -B build -G Ninja -DBOARD={{TARGET}}
cmake --build build -j16          # -> build/{{NAME}}.{elf,bin,hex}
```
Flash: `st-flash --reset write build/{{NAME}}.bin 0x08000000` (or openocd / pyocd / probe-rs, see mcu/stm32/README.md).
Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` with a USB-UART on PA9/PA10.

## Where to start editing
`src/main.c`. Board API: `board_init()`, `board_led_set/toggle()`, `board_button_*()` (where the board has a button), `printf()`.
Extra HAL modules (adc, i2c, tim, ...): `stm32_board({{NAME}} MODULES adc tim)` in CMakeLists.txt.
