# stm32 / button-irq

External interrupt on the user button toggles the LED (EXTI, NVIC, debounce).

**Status: builds; untested on hardware.**

The board layer configures the button pin as EXTI on the press edge and calls the weak `board_button_hook()` from the ISR; this app overrides it, debounces with SysTick (50 ms), toggles the LED and sets a flag that `main` prints.

| Board (target) | LED | User button | Console USART1 |
|---|---|---|---|
| blackpill-f411 (WeAct BlackPill F411CEU6) | PC13, active low | KEY PA0, active low (pull-up) | PA9 TX / PA10 RX |
| weact-g431 (WeAct G431CBT6 core) | PC6, active high | PC13, active high (pull-down) | PA9 TX / PA10 RX |
| mini-h743 (H743VIT6 min-system) | PE3, active high | PC13, active high (pull-down) | PA9 TX / PA10 RX |
| bluepill-f103 (F103C8T6) | PC13, active low | none in the board layer; examples that need one use PA0 (see below) | PA9 TX / PA10 RX |

bluepill-f103: the board layer has no button (`BOARD_HAS_BUTTON 0`), so the example configures **PA0 itself: input with internal pull-down, EXTI0 on the rising edge** (active high).
Plain BluePill: wire a push button from PA0 to 3V3. WeAct BluePill+ : the on-board KEY is PA0 active high according to Zephyr `boards/weact/bluepillplus_ch32v203` (the CH32V203 variant of the same WeAct board);
**unverified** for the F103 variant (no F103 BluePill+ schematic in this workspace).

Expected: each press toggles the LED and prints `press #N`.

## Build
```
. mcu/stm32/env.sh
cmake -S mcu/examples/stm32/button-irq -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/button-irq/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/button-irq/blackpill-f411 -j16        # -> button-irq.{elf,bin,hex}
```
Other boards: `-DBOARD=<target>` with its own build dir, or `MCU_JOBS=4 bash mcu/templates/tools/check.sh --only button-irq stm32`.
Flash with any probe (see `mcu/stm32/README.md`): `st-flash --reset write button-irq.bin 0x08000000`.
Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` on a USB-UART wired to PA9/PA10 (115200 8N1).

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `mcu/stm32/boards/<board>/` cites them. They were cross-checked against the
Zephyr board files in this workspace (`blackpill_f411ce.dts`, `weact_stm32g431_core.dts`, `mini_stm32h743.dts`, `stm32_min_dev` blue overlay);
the vendor schematics were not re-read. Clocks: F411 100 MHz (HSE 25), G431 144 MHz (HSE 8), H743 400 MHz (HSE 25), F103 72 MHz (HSE 8).
