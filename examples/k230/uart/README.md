# k230 / uart

Configures the board's UART2 pins through the FPIOA and does a TX-to-RX loopback five times (jumper the two pins).

Status: builds; untested on hardware.

## How it is built (and why not in the SDK tree)
RT-Smart user apps in `k230_rtos_sdk` normally live in `sdk/src/applications/<app>/` and are enabled with a Kconfig switch. This
project instead has its **own Makefile** that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
(read only) and writes only to the build dir, so nothing is added to or edited in the SDK tree. Requirements: the SDK's output for the
chosen board must already exist (`./mcu build k230 <target>`: that builds the rtsmart HAL libraries the app links against), and the
SDK's single global `.config` must select that board; `check.sh` does both selection steps under the SDK's build flock.

```
mcu/templates/tools/check.sh --only uart k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/uart/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Pins
Per board, from the SDK's board files (see the comment in `board.h` for exact paths):
| | k230_rtos_01studio | k230d_rtos_bpi_zero |
|---|---|---|
| LED | IO52 (dtsi comment `LED_GPIO52`) | none in the dts: external LED on JP1-15 = IO2 (dts `jp1-pin15`) |
| key | IO21 (dtsi `KEY_GPIO21`) | SW3: IO0 (dts `SW3 KEY` above IO0/IO1; IO1 is BOOT1) |
| UART2 TX/RX | IO11/IO12 (dtsi `GPIO11/12`; SDK uart test) | IO44/IO45 (dts `UART2`, pinmux_config.c) |

**Unverified**: LED/key active levels (no schematic in the SDK; LED assumed active high, key active low with pull-up), the
IO0-vs-IO1 choice for SW3, and that the owned "Jia Nan" board matches the SDK's 01Studio board files. Override with
`EXTRA_CFLAGS="-DLED_GPIO=.. -DLED_ACTIVE_LOW=1"`. The board is picked from the SDK's `generated/autoconf.h`.

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` onto the SD card (`/sdcard/app/`) or the
image's `sdcard/app` overlay, then run it from the msh console on the debug UART (115200).

## Expected
`rx(17): hello k230 uart` five times with the jumper fitted, `no data ...` without. UART2 pins: 01studio IO11/12 (as in the SDK's own peripheral test), BPI zero IO44/45 (its dts UART2); check they reach a header on your board.
