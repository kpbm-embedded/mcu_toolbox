# stm32 / i2c-scan

Scan I2C1 for 7-bit devices and print an address table.

**Status: builds; untested on hardware.**

| Target | SCL | SDA | Source |
|---|---|---|---|
| blackpill-f411 | PB8 (AF4) | PB9 (AF4) | Zephyr `blackpill_f411ce.dts` (i2c1_scl_pb8, i2c1_sda_pb9) |
| bluepill-f103 | PB6 | PB7 | I2C1 default mapping, no remap (RM0008); Zephyr hal_stm32 pinctrl (`modules/hal/stm32/dts/st/<family>/<part>-pinctrl.dtsi` in the zephyr workspace) |
| weact-g431 | PA15 (AF4) | PB7 (AF4) | Zephyr hal_stm32 pinctrl (`modules/hal/stm32/dts/st/<family>/<part>-pinctrl.dtsi` in the zephyr workspace): `i2c1_scl_pa15`, `i2c1_sda_pb7`. PB8/PB9 avoided: PB8 is BOOT0 on this board (Zephyr `weact_stm32g431_core.dts` button1 "Boot0") |
| mini-h743 | PB8 (AF4) | PB9 (AF4) | Zephyr `mini_stm32h743.dts` (camera I2C: i2c1_scl_pb8, i2c1_sda_pb9) |

100 kHz. F1/F4 use the legacy HAL I2C (ClockSpeed). G4/H7 have the timing-register I2C: kernel clock HSI (16 MHz on G4, 64 MHz on H7) and
TIMINGR from the reference-manual 100 kHz example (PRESC so that tPRESC = 250 ns, SCLL 0x13, SCLH 0x0F, SDADEL 2, SCLDEL 4): G4 `0x30420F13`, H7 `0xF0420F13`; analog filter on.
The internal pull-ups are enabled on F4/G4/H7 but are too weak for reliable operation: fit external 4.7k.

Expected output: a 16-column table with `--` for absent addresses and `NN` for ACKs, then `N device(s)`.

## Build
```
. mcu/stm32/env.sh
cmake -S mcu/examples/stm32/i2c-scan -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/i2c-scan/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/i2c-scan/blackpill-f411 -j16        # -> i2c-scan.{elf,bin,hex}
```
Other boards: `-DBOARD=<target>` with its own build dir, or `MCU_JOBS=4 bash mcu/templates/tools/check.sh --only i2c-scan stm32`.
Flash with any probe (see `mcu/stm32/README.md`): `st-flash --reset write i2c-scan.bin 0x08000000`.
Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` on a USB-UART wired to PA9/PA10 (115200 8N1).

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `mcu/stm32/boards/<board>/` cites them. They were cross-checked against the
Zephyr board files in this workspace (`blackpill_f411ce.dts`, `weact_stm32g431_core.dts`, `mini_stm32h743.dts`, `stm32_min_dev` blue overlay);
the vendor schematics were not re-read. Clocks: F411 100 MHz (HSE 25), G431 144 MHz (HSE 8), H743 400 MHz (HSE 25), F103 72 MHz (HSE 8).
