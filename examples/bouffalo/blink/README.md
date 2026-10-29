# bouffalo / blink

Blink one GPIO LED and print a heartbeat on the console UART. Status: **builds; untested on hardware**.

Targets: bl616, bl702, bl808 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702; Sipeed M1s Dock BL808 (M0 core, legacy SDK)).

## Pins
LED_PIN from board_pins.h: bl616 GPIO27 (M0s Dock / Ai-M62), bl702 GPIO22 (M0sense), bl808 GPIO8 (M1s Dock, M0 core).
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/blink/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only blink bouffalo`.
bl808 uses the legacy SDK (`bouffalo_sdk_legacy`, `CPU_ID=m0`) and `proj.conf`; the other targets use SDK v2.3.37 and `defconfig`.

## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
LED toggles every 500 ms; console prints `blink N (LED_PIN=..)`.
