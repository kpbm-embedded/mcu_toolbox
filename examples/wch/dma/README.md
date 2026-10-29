# wch / dma

DMA1 channel 1 copies a buffer memory-to-memory (MEM2MEM, 32-bit words, very high priority; "peripheral" address = source),
the CPU polls the transfer-complete flag, compares the buffers with `memcmp` and prints the result once per second; the LED
toggles each round. Buffer: 1 KB (V003: 256 B, it has 2 KB SRAM). Register use follows ch32fun `examples/dma_gpio_mem2mem`.
H417: DMA1 is enabled with `RCC->HBPCENR |= RCC_HBPeriph_DMA1`; the channel registers are the same names in
`ch32h41xhw.h`, but this path is **unverified** (no ch32fun DMA example for H41x).

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
mcu/templates/tools/check.sh --only dma wch      # all targets -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/wch/dma/<target>/build/
# by hand (build in a copy: ch32fun writes products next to the sources)
source mcu/wch/env.sh
B=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/mywork/dma; mkdir -p $B && cp -r . $B && make -C $B TARGET_NAME=ch32v203
```
The Makefile includes `$CH32FUN_REPO/ch32fun/ch32fun.mk` (read-only use; the generated linker script goes to the build copy).

## Flash / monitor
`minichlink -w main.bin flash -b` (WCH-LinkE/LinkW), `wchisp flash main.bin` for the USB bootloader on V203/V307.
printf goes to USART1 TX at 115200 (`funconfig.h`): V003 PD5, V203/V307 PA9, H417 PA9 (ch32fun SetupUART).

## Expected
`dma (CH32V203): DMA1 channel 1 mem2mem`, then `round N: copy OK` every second.
