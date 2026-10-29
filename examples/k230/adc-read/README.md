# k230 / adc-read

Initialises the K230 SAR ADC (`drv_adc_init`) and prints raw counts and millivolts of all six channels once a second for 10 s, using the driver's default 1.8 V reference (`DRV_ADC_DEFAULT_REF_UV`).

Status: builds; untested on hardware.

## Pins
The six ADC inputs are dedicated analog balls of the chip, not FPIOA pins, so nothing is routed in software. Input range 0..1.8 V: do not apply 3.3 V. Which header pins of the 01Studio-style board and the BPI-K230D-Zero reach ADC0..5 is **unverified** (no ADC mention in either board's dts or pinmux file).

## Build
RT-Smart user-space app with its own Makefile that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
read only (same scheme as ../hello). Needs the SDK output for the board (`./mcu build k230 <target>` once) and the SDK's global
`.config` set to that board; `check.sh` does the selection under the SDK build flock.
```
mcu/templates/tools/check.sh --only adc-read k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/adc-read/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` to the SD card (`/sdcard/app/`), run it
from the msh console on the debug UART (UART0, 115200): `/sdcard/app/app.elf`.

## Expected
`adc-read (<board>): 6 channels, 12 bit, ref 1800000 uV`, then ten lines like `ch0: 2047 ( 899 mV) ch1: ...`; a floating input reads noise.
