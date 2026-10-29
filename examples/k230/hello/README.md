# k230 / hello

Prints a greeting and three one-second ticks, then exits back to msh.

Status: builds; untested on hardware.

## How it is built (and why not in the SDK tree)
RT-Smart user apps in `k230_rtos_sdk` normally live in `sdk/src/applications/<app>/` and are enabled with a Kconfig switch. This
project instead has its **own Makefile** that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
(read only) and writes only to the build dir, so nothing is added to or edited in the SDK tree. Requirements: the SDK's output for the
chosen board must already exist (`./mcu build k230 <target>`: that builds the rtsmart HAL libraries the app links against), and the
SDK's single global `.config` must select that board; `check.sh` does both selection steps under the SDK's build flock.

```
mcu/templates/tools/check.sh --only hello k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/hello/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Pins
No LED/key table exists in the SDK tree for these boards. Defaults in `board.h` (LED GPIO52, key GPIO21) are **unconfirmed** (recalled from
the CanMV-K230 / 01Studio MicroPython docs; not checked against the owned boards' schematics). They are plain defines; override with
`EXTRA_CFLAGS`.

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` onto the SD card (`/sdcard/app/`) or the
image's `sdcard/app` overlay, then run it from the msh console on the debug UART (115200).

## Expected
`hello from K230 RT-Smart user space` then `tick 0..2` on the console.
