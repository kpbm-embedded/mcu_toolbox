# stm32 / low-power

Stop mode with EXTI wakeup on the user button; clock tree restored after wake.

**Status: builds; untested on hardware.**

Blinks 3 times, prints, waits for the last UART byte, suspends SysTick and enters **Stop mode** (`HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI)`:
all clocks of the core domain stopped, SRAM and registers kept; G4 = Stop 1, H7 = all domains in Stop). The user button EXTI line wakes it up.
After wakeup the MCU runs from HSI with PLL/HSE off, so `board_clock_init()` restores the normal clock tree before the console is used again.

| Board (target) | LED | User button | Console USART1 |
|---|---|---|---|
| blackpill-f411 (WeAct BlackPill F411CEU6) | PC13, active low | KEY PA0, active low (pull-up) | PA9 TX / PA10 RX |
| weact-g431 (WeAct G431CBT6 core) | PC6, active high | PC13, active high (pull-down) | PA9 TX / PA10 RX |
| mini-h743 (H743VIT6 min-system) | PE3, active high | PC13, active high (pull-down) | PA9 TX / PA10 RX |
| bluepill-f103 (F103C8T6) | PC13, active low | none in the board layer; examples that need one use PA0 (see below) | PA9 TX / PA10 RX |

bluepill-f103: the example configures PA0 as wake button itself (internal pull-down, EXTI0 rising edge); plain BluePill: button from PA0 to 3V3;
BluePill+ on-board KEY = PA0 active high per Zephyr `bluepillplus_ch32v203` (CH32V203 variant of the board), **unverified** for the F103 board.

Notes: a connected SWD debugger loses the core in Stop (enable `DBGMCU` stop debugging if you need it, at the cost of current).
Standby (lowest current, reset on wake via WKUP pin / RTC) is not shown: the button pins are WKUP pins on F411 (PA0 = WKUP1) and H743 (PC13 = WKUP4),
but the F411 KEY is active low while WKUP needs a rising edge, so Stop + EXTI is the portable choice here. Current figures were not measured.

Expected output:
```
low-power on blackpill-f411: Stop mode, wake with the user button
cycle 1: entering Stop (SYSCLK 100000000 Hz)
woke up (button events 1), SYSCLK 100000000 Hz
```

## Build
```
. mcu/stm32/env.sh
cmake -S mcu/examples/stm32/low-power -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/low-power/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/low-power/blackpill-f411 -j16        # -> low-power.{elf,bin,hex}
```
Other boards: `-DBOARD=<target>` with its own build dir, or `MCU_JOBS=4 bash mcu/templates/tools/check.sh --only low-power stm32`.
Flash with any probe (see `mcu/stm32/README.md`): `st-flash --reset write low-power.bin 0x08000000`.
Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` on a USB-UART wired to PA9/PA10 (115200 8N1).

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `mcu/stm32/boards/<board>/` cites them. They were cross-checked against the
Zephyr board files in this workspace (`blackpill_f411ce.dts`, `weact_stm32g431_core.dts`, `mini_stm32h743.dts`, `stm32_min_dev` blue overlay);
the vendor schematics were not re-read. Clocks: F411 100 MHz (HSE 25), G431 144 MHz (HSE 8), H743 400 MHz (HSE 25), F103 72 MHz (HSE 8).
