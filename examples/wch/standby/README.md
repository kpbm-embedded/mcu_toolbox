# wch / standby

V003: puts all pins to pulled inputs, enables the LSI + auto-wake-up (AWU) timer and enters standby with `__WFE`; after each wake-up it restores the clock and prints a counter (SDI printf, `minichlink -T`). The first 5 s are a grace delay to reprogram the chip before it sleeps (standby disables the debug interface).

Status: builds; untested on hardware.

## Pins (see `board.h`)
| Target | LED | Key | Source |
|---|---|---|---|
| ch32v003 | PC0 (active high) | PC3 (to GND) | **unconfirmed** default: no WeAct F4U6 / EVT schematic found; avoids SWIO (PD1) and NRST (PD7) |
| ch32v203 | PB2 (active high) | PA0 (active high) | WeAct BluePill Plus CH32V203, Zephyr `boards/weact/bluepillplus_ch32v203.dts` |
| ch32v307 | PD0 (assumed active low, jumper LED1 to PD0) | PA0 (assumed active high) | WCH CH32V307V-R1 EVT, Zephyr `boards/wch/ch32v307v_evt_r1` doc (LEDs not wired to the SoC) |
| ch32h417 | PE2 | PA0 (to GND, pull-up; arbitrary) | ch32fun `examples_h41x` use PE2; no board file for the owned CH32H417, both **unconfirmed** |
The V003-specific AWU registers (`PWR->AWUCSR` etc.) are not on V203/V307, so those targets use their own source
(`ch32v203/main.c`, `ch32v307/main.c` includes it; the Makefile picks `<target>/main.c`): standby (PDDS + SLEEPDEEP), woken
by a rising edge on the WKUP pin PA0 (`PWR_CSR.EWUP`; PA0 is the user key on the BluePill Plus and the WAKEUP key on the
V307 EVT, see the table) or by the IWDG after ~5 s (LSI ~40 kHz / 128, reload 1561, as in ch32fun
`examples_v30x/lowpower_standbyMode_test`). Standby wakes through a reset; the cause (PWR_CSR.SBF, RCC IWDGRSTF) is printed
with the SDI printf (`minichlink -T`). The 3 s pause after every boot is the window to reflash.

## Build
```
mcu/templates/tools/check.sh --only standby wch      # all targets -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/wch/standby/<target>/build/
# by hand (build in a copy: ch32fun writes products next to the sources)
source mcu/wch/env.sh
B=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/mywork/standby; mkdir -p $B && cp -r . $B && make -C $B TARGET_NAME=ch32v203
```
The Makefile includes `$CH32FUN_REPO/ch32fun/ch32fun.mk` (read-only use; the generated linker script goes to the build copy).

## Flash / monitor
`minichlink -w main.bin flash -b` (WCH-LinkE/LinkW; V003 also via wlink), `wchisp flash main.bin` for the USB bootloader on V203/V307. printf: `minichlink -T` (SDI) or a USB-serial adapter on USART1 when the example says so.

## Expected
After 5 s the chip sleeps; the monitor shows `awake, N` about once a second. Measure the supply current to see the standby draw (no figure measured here). To reprogram, power cycle and flash within the first 5 s.
V203/V307: `standby (CH32V203): cold start / reset pin`, LED flickers 3 times, `entering standby`; then every ~5 s
`woke from standby by the IWDG (~5 s)`, or `... by WKUP pin PA0` when the key is pressed.
