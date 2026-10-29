# bouffalo / low-power

Hibernate (HBN level 0) with RTC wake-up using the SDK 2.3 PM API (`pm_hbn_mode_enter`, `drivers/soc/<chip>/std/include/<chip>_pm.h`), pattern from `bouffalo_sdk/examples/pmu/<chip>/hbn_rtc`. Each boot blinks the LED 3 times, prints a line, then hibernates 5 s; the RTC wake-up resets the chip, so `main()` runs again (RAM is lost). Status: **builds; untested on hardware**.

Targets: bl616, bl702 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702). bl808 is not built: the legacy SDK 2.2.0 has only the low-level `bl808_hbn.h`/`bl808_pds.h` drivers, no `pm_*` API and no bl808 PMU example.

## Pins
LED_PIN from `board_pins.h` as in blink: bl616 GPIO27, bl702 GPIO22 (unverified against Sipeed schematics). To see the sleep, measure the current into the module (the Sipeed boards' USB-UART / LED keep drawing power).
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`; bl808: `bouffalo_sdk_legacy/bsp/board/bl808dk/board.c`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/low-power/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only low-power bouffalo`.

## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
`awake; entering HBN level 0 for 5 s (RTC wake-up = reset)` every ~6 s, preceded by the SDK boot banner. For GPIO wake-up or PDS (RAM-retaining) sleep see `bouffalo_sdk/examples/pmu/<chip>/pds_io_wakeup`, `pds_rtc`, `hbn_io_wakeup` (bl616).
