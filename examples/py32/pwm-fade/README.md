# py32 / pwm-fade

TIM1 generates a 1 kHz PWM on the pin named in `board.h` (PA8 on F003, PA0 on F002A); the duty ramps 0..100 %..0 about every 2 s. Wire an LED + resistor to that pin (the on-board LED is PB5, which is not a TIM1 output).

Status: builds; untested on hardware.

## Pins
Source: Puya LL examples in IOsetting/py32f0-template (`Examples/PY32F0xx/LL/...`): LED PB5 and key PA12 (`GPIO/EXTI_Toggle`),
USART1 PA2 TX / PA3 RX (`PY32F0xx_LL_BSP/py32f0xx_bsp_printf.c`), ADC channel 4 = PA4 (`ADC/ADC_SingleConversion_TriggerTimer`),
PWM TIM1_CH1 on PA8 AF2 (F003, `TIM/TIM1_PWM`; F030: datasheet table 3.1) or TIM1_CH3 on PA0 AF13 (F002A, `TIM/PY32F002AF15P_TIM1_PWM`).
**No schematic is available for the owned PY32F003 / PY32F002A boards**, so these may differ (package pin-out limits too: the
F002A in TSSOP20 does not have every pin). Edit `board.h`.
PY32F030x8 (bare chips, no board): same LL code and pins as the F003 (the Puya LL examples cover F002A/F003/F030 with one
source tree; PA8 AF2 = TIM1_CH1 checked in the PY32F030 datasheet AF table 3.1; which pins exist depends on the package).

## Build
```
mcu/templates/tools/check.sh --only pwm-fade py32      # all targets -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/py32/pwm-fade/<target>/build/app.{elf,bin,hex}
# by hand
source mcu/py32/env.sh
make TARGET=py32f003x8 BUILD=$PWD/build          # or py32f002ax5, py32f030x8
```
The Makefile reads the Puya LL driver, CMSIS and linker scripts from `$PY32_TEMPLATE` (read only); nothing in the SDK is modified.

## Flash / monitor
`pyocd flash --pack "$PY32_PACK" -t py32f003x8 build/app.hex` (SWD), `python3 -m serial.tools.miniterm /dev/ttyUSB0 115200`.

## Expected
An LED on the PWM pin breathes.
