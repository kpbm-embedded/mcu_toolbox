# bouffalo / usb-cdc

USB CDC-ACM echo device (CherryUSB). Status: **builds; untested on hardware**.

Targets: bl616, bl702 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702).

## Pins
USB: native USB pins of the module (bl616 HS OTG, bl702 FS); no GPIO config needed on the dk boards.
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/usb-cdc/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only usb-cdc bouffalo`.


## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
Host sees /dev/ttyACMx (VID:PID FFFF:FFFF demo placeholder); typed text is echoed.
