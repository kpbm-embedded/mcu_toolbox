# py32 / blink

Toggles the LED on PB5 every 500 ms.

Status: builds; untested on hardware.

## Pins
Source: Puya LL examples in IOsetting/py32f0-template (`Examples/PY32F0xx/LL/...`): LED PB5 and key PA12 (`GPIO/EXTI_Toggle`),
USART1 PA2 TX / PA3 RX (`PY32F0xx_LL_BSP/py32f0xx_bsp_printf.c`), ADC channel 4 = PA4 (`ADC/ADC_SingleConversion_TriggerTimer`),
PWM TIM1_CH1 on PA8 AF2 (F003, `TIM/TIM1_PWM`; F030: datasheet table 3.1) or TIM1_CH3 on PA0 AF13 (F002A, `TIM/PY32F002AF15P_TIM1_PWM`).
**No schematic is available for the owned PY32F003 / PY32F002A boards**, so these may differ (package pin-out limits too: the
F002A in TSSOP20 does not have every pin). Edit `board.h`.
PY32F030x8 (bare chips, no board): same LL code and pins as the F003 (the Puya LL examples cover F002A/F003/F030 with one
source tree; PA8 AF2 = TIM1_CH1 checked in the PY32F030 datasheet AF table 3.1; which pins exist depends on the package).
PY32F071x8 (bare chips, HAL only in the template): `main.c` has a HAL branch; LED PC13 from Puya `PY32F07x/HAL/GPIO/GPIO_Toggle`
(unverified, no board), debug UART USART2 PA2 TX / PA3 RX AF1 from `PY32F07x_HAL_BSP/Inc/py32f07x_bsp_printf.h`. The
Makefile generates a 64 KB / 8 KB linker script from the template's `py32f072xb.ld` (sizes: PY32F071 datasheet table 1-1).

## Build
```
mcu/templates/tools/check.sh --only blink py32      # all targets -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/py32/blink/<target>/build/app.{elf,bin,hex}
# by hand
source mcu/py32/env.sh
make TARGET=py32f003x8 BUILD=$PWD/build          # or py32f002ax5, py32f030x8, py32f071x8
```
The Makefile reads the Puya LL driver, CMSIS and linker scripts from `$PY32_TEMPLATE` (read only); nothing in the SDK is modified.

## Flash / monitor
`pyocd flash --pack "$PY32_PACK" -t py32f003x8 build/app.hex` (SWD), `python3 -m serial.tools.miniterm /dev/ttyUSB0 115200`.

## Expected
LED blinks at 1 Hz (PY32F071x8: PC13).
