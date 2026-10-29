# k230 / timer

Hardware timer 0 (`drv_hard_timer`, periodic, 500 ms) toggles the LED from its callback; a software timer (`drv_soft_timer`, 100 ms) counts ticks. Prints both counts once a second for 10 s. The K230 has 6 hardware timers (`KD_TIMER_MAX_NUM`); the clock is set to the timer's minimum frequency so that 500 ms fits in its counter.

Status: builds; untested on hardware.

## Pins
LED only: 01studio IO52 (dtsi `LED_GPIO52`); BPI zero external LED on JP1-15 = IO2 (dts `jp1-pin15`). **Unverified**: LED/key active levels (no schematic in the SDK; LED assumed active high, key active low with pull-up), the IO0-vs-IO1 choice for SW3 on the BPI zero, and that the owned "Jia Nan" board matches the SDK's 01Studio board files. All pins are defines in `board.h` (paths of the source files in its header comment); override with `EXTRA_CFLAGS="-DKEY_GPIO=.."`.

## Build
RT-Smart user-space app with its own Makefile that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
read only (same scheme as ../hello). Needs the SDK output for the board (`./mcu build k230 <target>` once) and the SDK's global
`.config` set to that board; `check.sh` does the selection under the SDK build flock.
```
mcu/templates/tools/check.sh --only timer k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/timer/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` to the SD card (`/sdcard/app/`), run it
from the msh console on the debug UART (UART0, 115200): `/sdcard/app/app.elf`.

## Expected
`timer0: freq ... Hz, maxcnt ...`, then `timer (<board>): hw 2 x 500 ms, sw 10 x 100 ms`, `hw 4 ..., sw 20 ...` and so on; LED blinks at 1 Hz.
