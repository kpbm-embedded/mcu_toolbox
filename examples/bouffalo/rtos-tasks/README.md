# bouffalo / rtos-tasks

FreeRTOS as shipped in the SDK (`components/os/freertos`): a blink task (250 ms), a producer that queues a counter every second, a consumer that prints it, and a mutex around `printf`. Stack-overflow and malloc-failed hooks print and halt. Status: **builds; untested on hardware**.

Targets: bl616, bl702, bl808 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702; Sipeed M1s Dock BL808, M0 core, legacy SDK 2.2.0 `bouffalo_sdk_legacy`).
`FreeRTOSConfig.h` is the one from `bouffalo_sdk_legacy/examples/freertos` (it carries the CLINT mtime addresses the legacy RISC-V port needs; SDK 2.3 ignores them) plus `INCLUDE_uxTaskGetStackHighWaterMark`. The SDK heap is its own allocator (no `heap_x.c`), so the consumer prints its stack headroom instead of free heap.

## Pins
LED_PIN from `board_pins.h` as in blink: bl616 GPIO27, bl702 GPIO22, bl808 GPIO8 (unverified against Sipeed schematics).
Pin sources: SDK board files (`bouffalo_sdk/bsp/board/<chip>dk/`; bl808: `bouffalo_sdk_legacy/bsp/board/bl808dk/board.c`), not Sipeed schematics. The dk-board pin numbers are valid on the
Sipeed boards only where the GPIO is broken out; `LED_PIN`/`BTN_PIN` in `board_pins.h` are unverified guesses (override if wrong).

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/rtos-tasks/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`; bl808 uses the legacy SDK with `CPU_ID=m0`).
Or `mcu/templates/tools/check.sh --only rtos-tasks bouffalo`.

## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
```
rtos-tasks: FreeRTOS V10.6.2
producer: sent 0
consumer: got 0, stack headroom NNN words
```
and the LED blinks at 2 Hz.
