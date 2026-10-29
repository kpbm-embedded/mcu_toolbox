# bouffalo / pwm-fade

PWM channel 0 duty ramps up and down at 1 kHz. Status: **builds; untested on hardware**.

Targets: bl616, bl702, bl808 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702; Sipeed M1s Dock BL808, M0 core, legacy SDK 2.2.0 `bouffalo_sdk_legacy`).

## Pins
board_pwm_gpio_init(): bl616 PWM_V2 CH0 = GPIO24, bl702 PWM_V1 CH0 = GPIO0, bl808 PWM_V2 (pwm_v2_0) CH0 = GPIO24 (SDK dk board files; bl808 board_pwm_gpio_init muxes GPIO24..31 to PWM0, CH0 on GPIO24 by the same pin%8 pattern as bl616 - unverified). Wire an LED + resistor to that pin.
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`; bl808: `bouffalo_sdk_legacy/bsp/board/bl808dk/board.c`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/pwm-fade/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only pwm-fade bouffalo`.


## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
Brightness of an LED on the PWM pin fades; console prints at each turnaround.
