# k230 / i2c-scan

Routes the board's I2C SCL/SDA through the FPIOA, opens the hardware controller at 100 kHz (`drv_i2c_inst_create`) and probes 0x08..0x77 with a 1-byte read; addresses whose transfer ioctl returns 0 (RT_EOK) ACKed. (The DesignWare controller cannot send an address-only zero-length write.)

Status: builds; untested on hardware.

## Pins
| | k230_rtos_01studio | k230d_rtos_bpi_zero |
|---|---|---|
| bus | I2C0: SCL IO48, SDA IO49 (dtsi `iic0`; pinmux_config.c `IIC0_SCL/SDA`) | I2C2: SCL IO11, SDA IO12 = JP1-3/JP1-5 (dts `jp1-pin3,5 iic2`; pinmux_config.c) |

Pull-ups: the pinmux configs enable the internal pull-up on these pins; add 4.7 k external pull-ups for reliable 100 kHz. Header position of IO48/49 on the 01Studio-style board is **unverified**. Override with `EXTRA_CFLAGS="-DI2C_ID=.. -DI2C_SCL_GPIO=.. -DI2C_SDA_GPIO=.."`.

## Build
RT-Smart user-space app with its own Makefile that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
read only (same scheme as ../hello). Needs the SDK output for the board (`./mcu build k230 <target>` once) and the SDK's global
`.config` set to that board; `check.sh` does the selection under the SDK build flock.
```
mcu/templates/tools/check.sh --only i2c-scan k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/i2c-scan/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` to the SD card (`/sdcard/app/`), run it
from the msh console on the debug UART (UART0, 115200): `/sdcard/app/app.elf`.

## Expected
`i2c-scan (<board>): I2C0 SCL IO48 SDA IO49`, `  found 0x3c` for each device, `N device(s)`.
