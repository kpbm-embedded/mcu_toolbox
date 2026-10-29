# py32 / low-power

Enters Stop mode (deep sleep, low-power regulator) in a loop. LPTIM1 clocked from the LSI (32.768 kHz / 128, ARR 255,
one-shot, EXTI line 29) wakes it after ~1 s; a falling edge on the key (EXTI line 12) wakes it early. After each wake-up
the LED pulses for 20 ms and USART1 prints the reason. Based on Puya `LL/LPTIM/LPTIM1_Wakeup` and `LL/PWR/PWR_Stop`.
The first 3 s after reset stay awake so SWD can still attach (as in PWR_Stop).

Status: builds; untested on hardware.

## Pins
Source: Puya LL examples in IOsetting/py32f0-template (`Examples/PY32F0xx/LL/...`): LED PB5 and key PA12 (`GPIO/EXTI_Toggle`),
USART1 PA2 TX / PA3 RX (`PY32F0xx_LL_BSP/py32f0xx_bsp_printf.c`).
**No schematic is available for the owned PY32F003 / PY32F002A boards or the bare PY32F030 chips**, so these may differ
(package pin-out limits too). Edit `board.h`.

## Wiring
Push-button between PA12 and GND. Measure the supply current to see Stop mode (datasheet: a few uA in Stop).

## Build
```
mcu/templates/tools/check.sh --only low-power py32      # targets: py32f003x8, py32f002ax5, py32f030x8 -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/py32/low-power/<target>/build/app.{elf,bin,hex}
# by hand
source mcu/py32/env.sh
make TARGET=py32f003x8 BUILD=$PWD/build          # or py32f002ax5, py32f030x8
```
The Makefile reads CMSIS, the Puya LL (F07x: HAL) driver and linker scripts from `$PY32_TEMPLATE` (read only); nothing in the SDK is modified.

## Flash / monitor
`pyocd flash --pack "$PY32_PACK" -t py32f003x8 build/app.hex` (SWD), `python3 -m serial.tools.miniterm /dev/ttyUSB0 115200`.

## Expected
```
low-power: stop mode, wake by LPTIM1 (~1 s) or key
wake 1 by LPTIM
wake 2 by key
```
LSI is ~32 kHz with a wide tolerance, so the period is only roughly 1 s.
