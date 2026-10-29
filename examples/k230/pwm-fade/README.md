# k230 / pwm-fade

Routes PWM channel 0 to a pin through the FPIOA and ramps the duty 0..100 % and back at 1 kHz (`drv_pwm_set_duty`, 2 % steps every 20 ms): five 2-second breaths, then the channel is disabled.

Status: builds; untested on hardware.

## Pins
| | k230_rtos_01studio | k230d_rtos_bpi_zero |
|---|---|---|
| PWM0 | IO42 (SDK `src/rtsmart/examples/peripheral/pwm/test_pwm.c` maps PWM0 to pin 42; dtsi `GPIO42`) | IO60 = JP1-33 (dts `JP1-PIN33, pwm`; pinmux_config.c `PWM0`) |

Wire an LED with a ~330 R resistor from the pin to GND. Which header position IO42 has on the 01Studio-style board is **unverified**. Override with `EXTRA_CFLAGS="-DPWM_GPIO=.. -DPWM_CH=.."`.

## Build
RT-Smart user-space app with its own Makefile that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
read only (same scheme as ../hello). Needs the SDK output for the board (`./mcu build k230 <target>` once) and the SDK's global
`.config` set to that board; `check.sh` does the selection under the SDK build flock.
```
mcu/templates/tools/check.sh --only pwm-fade k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/pwm-fade/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` to the SD card (`/sdcard/app/`), run it
from the msh console on the debug UART (UART0, 115200): `/sdcard/app/app.elf`.

## Expected
`pwm-fade (<board>): PWM0 on IO42, 1 kHz, 5 breaths`, then `breath 1` ... `breath 5`; the LED fades up and down.
