# k230 / button-irq

Registers a GPIO edge interrupt (`drv_gpio_register_irq`, falling edge, 20 ms debounce) on the board key. The callback toggles the LED and bumps an atomic counter; main prints the count when it changes and exits after 30 s.

Status: builds; untested on hardware.

## Pins
| | k230_rtos_01studio | k230d_rtos_bpi_zero |
|---|---|---|
| key | IO21 (dtsi `KEY_GPIO21`) | SW3 on IO0 (dts `SW3 KEY` above IO0/IO1) |
| LED | IO52 (dtsi `LED_GPIO52`) | none in the dts: external LED + resistor on JP1-15 = IO2 (dts `jp1-pin15`) |

Only IO0..IO63 have GPIO interrupts (`GPIO_IRQ_MAX_NUM`). **Unverified**: LED/key active levels (no schematic in the SDK; LED assumed active high, key active low with pull-up), the IO0-vs-IO1 choice for SW3 on the BPI zero, and that the owned "Jia Nan" board matches the SDK's 01Studio board files. All pins are defines in `board.h` (paths of the source files in its header comment); override with `EXTRA_CFLAGS="-DKEY_GPIO=.."`.

## Build
RT-Smart user-space app with its own Makefile that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
read only (same scheme as ../hello). Needs the SDK output for the board (`./mcu build k230 <target>` once) and the SDK's global
`.config` set to that board; `check.sh` does the selection under the SDK build flock.
```
mcu/templates/tools/check.sh --only button-irq k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/button-irq/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` to the SD card (`/sdcard/app/`), run it
from the msh console on the debug UART (UART0, 115200): `/sdcard/app/app.elf`.

## Expected
`button-irq (<board>): press the key on GPIO21, LED on GPIO52; exits after 30 s` then `presses: 1`, `presses: 2`, ... and the LED toggles per press.
