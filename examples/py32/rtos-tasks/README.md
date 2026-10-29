# py32 / rtos-tasks

Three FreeRTOS tasks: `blink` toggles the LED every 250 ms, `producer` sends a counter to a queue every second,
`consumer` prints it (and the free heap) under a mutex. The kernel is the template's `Libraries/FreeRTOS` (port GCC/ARM_CM0,
heap_4), built when `project.mk` sets `RTOS := y`; `FreeRTOSConfig.h` is Puya's `FreeRTOS/Tasks/LL_Blink` one with a
4 KB heap and `configUSE_MUTEXES 1`. PY32F071x8 uses the HAL branch in `main.c` (SysTick belongs to the FreeRTOS port).
Not for the PY32F002A: with 3 KB SRAM this demo overflows RAM (by 640 B with a 2 KB heap).

Status: builds; untested on hardware.

## Pins
Source: Puya LL examples in IOsetting/py32f0-template (`Examples/PY32F0xx/LL/...`): LED PB5 and key PA12 (`GPIO/EXTI_Toggle`),
USART1 PA2 TX / PA3 RX (`PY32F0xx_LL_BSP/py32f0xx_bsp_printf.c`); PY32F071x8: LED PC13 (Puya `PY32F07x/HAL/GPIO/GPIO_Toggle`, unverified), USART2 PA2/PA3 AF1 (`PY32F07x_HAL_BSP`).
**No schematic is available for the owned PY32F003 / PY32F002A boards or the bare PY32F030 chips**, so these may differ
(package pin-out limits too). Edit `board.h`.

## Wiring
Only the LED and the USB-UART.

## Build
```
mcu/templates/tools/check.sh --only rtos-tasks py32      # targets: py32f003x8, py32f030x8, py32f071x8 -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/py32/rtos-tasks/<target>/build/app.{elf,bin,hex}
# by hand
source mcu/py32/env.sh
make TARGET=py32f003x8 BUILD=$PWD/build          # or py32f030x8, py32f071x8
```
The Makefile reads CMSIS, the Puya LL (F07x: HAL) driver and linker scripts from `$PY32_TEMPLATE` (read only); nothing in the SDK is modified.

## Flash / monitor
`pyocd flash --pack "$PY32_PACK" -t py32f003x8 build/app.hex` (SWD), `python3 -m serial.tools.miniterm /dev/ttyUSB0 115200`.

## Expected
```
rtos-tasks: FreeRTOS V10.x
consumer got 0, free heap NNNN
consumer got 1, free heap NNNN
```
and the LED blinks at 2 Hz.
