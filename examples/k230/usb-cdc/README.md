# k230 / usb-cdc

USB CDC-ACM device from user space. The RT-Smart kernel already runs a CherryUSB gadget (`src/rtsmart/rtsmart/kernel/bsp/maix3/components/canmv_usb/usbd_cdc.c`, function "CDC + MTP", enabled by `CONFIG_ENABLE_CHERRY_USB_DEVICE` in both board defconfigs) and registers its serial port as the RT-Thread device `ttyGS0`. The app opens `/dev/ttyGS0` with plain POSIX `open/poll/read/write`, greets the host and echoes every received chunk upper-cased for 60 s. There is no separate user-space USB device API in rtsmart_hal; this tty is the user-space interface.

Status: builds; untested on hardware.

## Pins
USB device (OTG) port of each board to the host PC; which of the board's USB-C connectors is the device port is **unverified** (01Studio-style board: the one not used for USB host; BPI-K230D-Zero: its single USB-C, which also powers the board). On the host it appears as `/dev/ttyACM0` (VID 0x1209, PID 0xABD1 from the kernel Kconfig defaults).

## Build
RT-Smart user-space app with its own Makefile that includes the SDK's `src/applications/mkenv.mk` and `src/rtsmart/libs/mk/*.mk`
read only (same scheme as ../hello). Needs the SDK output for the board (`./mcu build k230 <target>` once) and the SDK's global
`.config` set to that board; `check.sh` does the selection under the SDK build flock.
```
mcu/templates/tools/check.sh --only usb-cdc k230       # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k230/usb-cdc/<target>/build/app.elf
# by hand
source mcu/k230/env.sh
flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK k230d_rtos_bpi_zero_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
```

## Run
Boot an RT-Smart image (`./mcu build k230 <target>` makes `*_rtos_*.img`), copy `app.elf` to the SD card (`/sdcard/app/`), run it
from the msh console on the debug UART (UART0, 115200): `/sdcard/app/app.elf`.

## Expected
Console: `usb-cdc (<board>): echoing on /dev/ttyGS0 for 60 s`, `echoed N byte(s)` per chunk. Host: `python -m serial.tools.miniterm /dev/ttyACM0` shows `k230 usb-cdc echo: type something` (if it was open before the app started) and echoes typed text in upper case.
