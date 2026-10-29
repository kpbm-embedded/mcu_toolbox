# py32 / dma

Fills a 1 KB buffer, copies it with DMA1 channel 1 (memory-to-memory, word size, TC/TE interrupts) and compares the
result with `memcmp`, once per second. PY32F003 and PY32F030 have a 3-channel DMA (datasheets); the PY32F002A has no DMA
(no `DMA1` in `py32f002ax5.h`), so it is not a target.

Status: builds; untested on hardware.

## Pins
Source: Puya LL examples in IOsetting/py32f0-template (`Examples/PY32F0xx/LL/...`): LED PB5 and key PA12 (`GPIO/EXTI_Toggle`),
USART1 PA2 TX / PA3 RX (`PY32F0xx_LL_BSP/py32f0xx_bsp_printf.c`).
**No schematic is available for the owned PY32F003 / PY32F002A boards or the bare PY32F030 chips**, so these may differ
(package pin-out limits too). Edit `board.h`.

## Wiring
Nothing to wire besides the USB-UART on PA2/PA3.

## Build
```
mcu/templates/tools/check.sh --only dma py32      # targets: py32f003x8, py32f030x8 -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/py32/dma/<target>/build/app.{elf,bin,hex}
# by hand
source mcu/py32/env.sh
make TARGET=py32f003x8 BUILD=$PWD/build          # or py32f030x8
```
The Makefile reads CMSIS, the Puya LL (F07x: HAL) driver and linker scripts from `$PY32_TEMPLATE` (read only); nothing in the SDK is modified.

## Flash / monitor
`pyocd flash --pack "$PY32_PACK" -t py32f003x8 build/app.hex` (SWD), `python3 -m serial.tools.miniterm /dev/ttyUSB0 115200`.

## Expected
```
dma round 1: complete, buffers match
dma round 2: complete, buffers match
```
