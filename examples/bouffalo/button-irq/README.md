# bouffalo / button-irq

Falling-edge GPIO interrupt from the BOOT key toggles the LED. Status: **builds; untested on hardware**.

Targets: bl616, bl702, bl808 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702; Sipeed M1s Dock BL808, M0 core, legacy SDK 2.2.0 `bouffalo_sdk_legacy`).

## Pins
BTN_PIN (BOOT key, active low) GPIO2 on bl616 and bl702 (unverified for bl702); bl808 BTN_PIN GPIO22 (board_pins.h, unverified against the M1s Dock schematic); LED_PIN as in blink.
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`; bl808: `bouffalo_sdk_legacy/bsp/board/bl808dk/board.c`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/button-irq/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only button-irq bouffalo`.


## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
Each key press toggles the LED and prints `button presses: N`.
