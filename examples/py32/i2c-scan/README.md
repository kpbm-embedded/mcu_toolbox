# py32 / i2c-scan

Scans the 7-bit I2C address range with START + address + STOP and prints every address that ACKs. Polled LL I2C
(the PY32F0 I2C is the SR1/SR2 "F1-style" peripheral), 100 kHz, timeouts so a stuck bus does not hang the loop.

Status: builds; untested on hardware.

## Pins
Source: Puya LL examples in IOsetting/py32f0-template (`Examples/PY32F0xx/LL/...`): LED PB5 and key PA12 (`GPIO/EXTI_Toggle`),
USART1 PA2 TX / PA3 RX (`PY32F0xx_LL_BSP/py32f0xx_bsp_printf.c`), I2C1 SDA PF0 / SCL PF1 AF12 (`I2C/AT24C32`, `I2C/SSD1306_128x64OLED`; PF0/PF1 AF12 = I2C_SDA/I2C_SCL in the PY32F030
datasheet table 3.3). PF0/PF1 are the HSE pins: they are free when the 24 MHz HSI is used (as here). Alternative per the
SSD1306 example comment: SCL PA9 / SDA PA10 AF6.
**No schematic is available for the owned PY32F003 / PY32F002A boards or the bare PY32F030 chips**, so these may differ
(package pin-out limits too). Edit `board.h`.

## Wiring
Wire the I2C device to PF0 (SDA) and PF1 (SCL) with 4.7k pull-ups to 3.3 V (the internal pull-ups are enabled but weak).

## Build
```
mcu/templates/tools/check.sh --only i2c-scan py32      # targets: py32f003x8, py32f002ax5, py32f030x8 -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/py32/i2c-scan/<target>/build/app.{elf,bin,hex}
# by hand
source mcu/py32/env.sh
make TARGET=py32f003x8 BUILD=$PWD/build          # or py32f002ax5, py32f030x8
```
The Makefile reads CMSIS, the Puya LL (F07x: HAL) driver and linker scripts from `$PY32_TEMPLATE` (read only); nothing in the SDK is modified.

## Flash / monitor
`pyocd flash --pack "$PY32_PACK" -t py32f003x8 build/app.hex` (SWD), `python3 -m serial.tools.miniterm /dev/ttyUSB0 115200`.

## Expected
```
i2c-scan: I2C1 SDA/SCL, 100 kHz
  found 0x3c
1 device(s)
```
(with an SSD1306 OLED; nothing connected -> `0 device(s)`)
