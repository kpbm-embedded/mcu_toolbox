# bouffalo / adc-read

Read one external ADC channel once a second. Status: **builds; untested on hardware**.

Targets: bl616, bl702, bl808 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702; Sipeed M1s Dock BL808, M0 core, legacy SDK 2.2.0 `bouffalo_sdk_legacy`).

## Pins
bl616 ADC_CH3 = GPIO3 (bsp/board/bl616dk/board_gpio.c); bl702 ADC_CH0, its GPIO is not stated by the SDK (datasheet) - unverified; bl808 ADC_CH3 = GPIO11 (bouffalo_sdk_legacy/bsp/board/bl808dk/board.c board_adc_gpio_init).
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`; bl808: `bouffalo_sdk_legacy/bsp/board/bl808dk/board.c`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/adc-read/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`).
Or `mcu/templates/tools/check.sh --only adc-read bouffalo`.


## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
`adc raw=0x.. N mV` per second; tie the pin to 3V3/GND to check.
