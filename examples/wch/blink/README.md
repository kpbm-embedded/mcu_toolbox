# wch / blink

Blinks the user LED at 2 Hz.

Status: builds; untested on hardware.

## Pins (see `board.h`)
| Target | LED | Key | Source |
|---|---|---|---|
| ch32v003 | PC0 (active high) | PC3 (to GND) | **unconfirmed** default: no WeAct F4U6 / EVT schematic found; avoids SWIO (PD1) and NRST (PD7) |
| ch32v203 | PB2 (active high) | PA0 (active high) | WeAct BluePill Plus CH32V203, Zephyr `boards/weact/bluepillplus_ch32v203.dts` |
| ch32v307 | PD0 (assumed active low, jumper LED1 to PD0) | PA0 (assumed active high) | WCH CH32V307V-R1 EVT, Zephyr `boards/wch/ch32v307v_evt_r1` doc (LEDs not wired to the SoC) |
| ch32h417 | PE2 | PA0 (to GND, pull-up; arbitrary) | ch32fun `examples_h41x` use PE2; no board file for the owned CH32H417, both **unconfirmed** |


## Build
```
mcu/templates/tools/check.sh --only blink wch      # all targets -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/wch/blink/<target>/build/
# by hand (build in a copy: ch32fun writes products next to the sources)
source mcu/wch/env.sh
B=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/mywork/blink; mkdir -p $B && cp -r . $B && make -C $B TARGET_NAME=ch32v203
```
The Makefile includes `$CH32FUN_REPO/ch32fun/ch32fun.mk` (read-only use; the generated linker script goes to the build copy).

## Flash / monitor
`minichlink -w main.bin flash -b` (WCH-LinkE/LinkW; V003 also via wlink), `wchisp flash main.bin` for the USB bootloader on V203/V307. printf: `minichlink -T` (SDI) or a USB-serial adapter on USART1 when the example says so.

## Expected
LED blinks twice per second.
