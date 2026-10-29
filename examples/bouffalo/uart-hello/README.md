# bouffalo / uart-hello

Console UART0 banner, periodic hello and character echo. Status: **builds; untested on hardware**.

Targets: bl616, bl702, bl808 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702; Sipeed M1s Dock BL808 (M0 core, legacy SDK)).

## Pins
Console UART0: bl616 TX GPIO21 RX GPIO22, bl702 TX GPIO14 RX GPIO15 (SDK board.c); bl808 per bsp/board/bl808dk/board.c.
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/uart-hello/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only uart-hello bouffalo`.
bl808 uses the legacy SDK (`bouffalo_sdk_legacy`, `CPU_ID=m0`) and `proj.conf`; the other targets use SDK v2.3.37 and `defconfig`.

## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
Banner, `hello N` every 2 s, typed characters echoed.
