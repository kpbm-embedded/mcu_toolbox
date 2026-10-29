# bouffalo / dma

Memory-to-memory DMA on `dma0_ch0`: a 4 KiB buffer is copied through a linked-list (LLI) transfer with a completion interrupt, then compared with the source and timed against `memcpy`. D-cache clean/invalidate around the transfer as in the SDK example `examples/peripherals/dma/dma_normal` (same API in SDK 2.3 and the legacy 2.2.0). Status: **builds; untested on hardware**.

Targets: bl616, bl702, bl808 (Sipeed M0s Dock BL616 / Ai-M62; Sipeed M0sense BL702; Sipeed M1s Dock BL808, M0 core, legacy SDK 2.2.0 `bouffalo_sdk_legacy`).

## Pins
No pins.

## Build
`. mcu/bouffalo/env.sh; ./build.sh <target> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/bouffalo/dma/<target>` (copies the example to `<out>/work`, runs the SDK `make CHIP=.. BOARD=..`; bl808 uses the legacy SDK with `CPU_ID=m0`).
Or `mcu/templates/tools/check.sh --only dma bouffalo`.

## Flash / monitor
Hold BOOT, tap RESET, `make flash CHIP=<chip> BOARD=<chip>dk COMX=/dev/ttyACM0` in `<out>/work` (see `mcu/bouffalo/README.md`), or `mcu flash bouffalo`. Monitor the console UART (115200 8N1 unless the SDK board file says otherwise; not verified on hardware).

## Expected output
`round 1: DMA 4096 bytes in NN us (ok), memcpy NN us` every 2 s.
