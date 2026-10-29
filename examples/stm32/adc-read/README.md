# stm32 / adc-read

Polled ADC: PA1, internal temperature sensor and VREFINT, printed each second.

**Status: builds; untested on hardware.**

| Target | PA1 channel | Temp / VREFINT | ADC clock | Conversion to mV / degC |
|---|---|---|---|---|
| blackpill-f411 | ADC1 IN1 | ADC1 IN18 / IN17 | PCLK2/4 = 25 MHz | VDDA assumed 3.3 V; datasheet typical V25 0.76 V, +2.5 mV/C |
| bluepill-f103 | ADC1 IN1 | ADC1 IN16 / IN17 | PCLK2/6 = 12 MHz, calibrated | VDDA assumed 3.3 V; datasheet typical V25 1.43 V, -4.3 mV/C |
| weact-g431 | ADC1 IN2 | ADC1 temp / VREFINT | HCLK/4 = 36 MHz, calibrated | VDDA from VREFINT_CAL, temperature from TS_CAL1/TS_CAL2 (`__HAL_ADC_CALC_*`) |
| mini-h743 | ADC1 INP17 (16-bit) | ADC3 temp / VREFINT (internal channels exist only on ADC3) | per_ck = HSI 64 MHz /4, offset-calibrated | same, factory calibration values |

PA1 channel numbers: Zephyr hal_stm32 pinctrl (`modules/hal/stm32/dts/st/<family>/<part>-pinctrl.dtsi` in the zephyr workspace) (`adc1_in1_pa1` F103, `adc1_in2_pa1` G431, `adc1_inp17_pa1` H743); F411 IN1 as before. PA1 is unused by the board files of all four boards.
Temperatures on F411/F103 are rough (typical datasheet slope, no calibration); G431/H743 use the factory calibration words.

Expected output (values vary):
```
PA1 raw= 2048 (1650 mV)  temp raw=  940 (~31.2 C)  vrefint raw= 1500
```

## Build
```
. mcu/stm32/env.sh
cmake -S mcu/examples/stm32/adc-read -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/adc-read/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/adc-read/blackpill-f411 -j16        # -> adc-read.{elf,bin,hex}
```
Other boards: `-DBOARD=<target>` with its own build dir, or `MCU_JOBS=4 bash mcu/templates/tools/check.sh --only adc-read stm32`.
Flash with any probe (see `mcu/stm32/README.md`): `st-flash --reset write adc-read.bin 0x08000000`.
Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` on a USB-UART wired to PA9/PA10 (115200 8N1).

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `mcu/stm32/boards/<board>/` cites them. They were cross-checked against the
Zephyr board files in this workspace (`blackpill_f411ce.dts`, `weact_stm32g431_core.dts`, `mini_stm32h743.dts`, `stm32_min_dev` blue overlay);
the vendor schematics were not re-read. Clocks: F411 100 MHz (HSE 25), G431 144 MHz (HSE 8), H743 400 MHz (HSE 25), F103 72 MHz (HSE 8).
