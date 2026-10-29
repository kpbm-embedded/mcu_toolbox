# k230 / low-power

CPU dynamic voltage/frequency scaling through the K230 power-management API (`mpi_pm_api.h`, kernel `CONFIG_RT_USING_PM=y` in both board defconfigs): lists the CPU domain's frequency/voltage profiles, selects the manual governor and the lowest-frequency profile, sleeps 10 s (the RT-Smart idle thread runs WFI), then restores the previous governor and profile. Deeper modes are outside an RT-Smart app: the PMU (`drv_pmu.h`) can only power the whole board off and schedule an RTC power cycle.

Status: builds; untested on hardware.

## Pins
No pins.

## Build
RT-Smart user-space app with its own Makefile that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
read only (same scheme as ../hello). Needs the SDK output for the board (`./mcu build k230 <target>` once) and the SDK's global
`.config` set to that board; `check.sh` does the selection under the SDK build flock.
```
mcu/templates/tools/check.sh --only low-power k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/low-power/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` to the SD card (`/sdcard/app/`), run it
from the msh console on the debug UART (UART0, 115200): `/sdcard/app/app.elf`.

## Expected
`low-power (<board>): governor N, profile M of K`, the profile table (`[0] 1600000000 Hz, ... uV` ...), `CPU at profile ... for 10 s`, `restored governor ...`.
