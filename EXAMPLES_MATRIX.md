# MCU examples and templates matrix

Generated 2026-10-24 from the per-SDK audit (build check: `templates/tools/check.sh <sdk>`). Format of examples/templates: `templates/SPEC.md`.
Cells: **OK** = builds (status of every example stays "builds; untested on hardware" until a board test), **n/a** = the hardware does not have it (reason in the notes), **GAP** = possible but not provided (reason in the notes), **FAIL** = does not build (reason in the notes), blank = not audited. `template` = a `templates/<sdk>/*minimal` template that instantiates (`mcu new <sdk> <name> <target>`) and builds.
Extra examples beyond the standard set are listed under each SDK.

## avr

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| nano | OK | OK (blink) | OK (button-irq) | OK (serial-hello) | OK (timer-interrupt) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^avr1] | n/a[^avr2] | n/a[^avr3] | OK (sleep-wdt) | n/a[^avr4] | n/a[^avr5] |
| nano-old | OK | OK (blink) | OK (button-irq) | OK (serial-hello) | OK (timer-interrupt) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^avr1] | n/a[^avr2] | n/a[^avr3] | OK (sleep-wdt) | n/a[^avr4] | n/a[^avr5] |
| mega | OK | OK (blink) | OK (button-irq) | OK (serial-hello) | OK (timer-interrupt) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^avr1] | n/a[^avr2] | n/a[^avr3] | OK (sleep-wdt) | n/a[^avr4] | n/a[^avr5] |
| attiny85 | OK | OK (blink) | OK (button-irq) | OK (serial-hello) | OK (timer-interrupt) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^avr1] | n/a[^avr2] | n/a[^avr3] | OK (sleep-wdt) | n/a[^avr4] | n/a[^avr5] |
| ch552 | OK (ch552-minimal) | OK (ch552-blink) | OK (ch552-button-irq) | OK (ch552-uart-hello) | OK (ch552-timer) | OK (ch552-pwm-fade) | OK (ch552-adc-read) | OK (ch552-i2c-scan) | OK (ch552-usb-cdc-serial) | n/a[^avr6] | n/a[^avr7] | GAP[^avr8] | n/a[^avr4] | n/a[^avr5] |
| attiny13a | GAP[^avr9] | GAP[^avr10] | GAP[^avr10] | GAP[^avr10] | GAP[^avr10] | GAP[^avr10] | GAP[^avr10] | GAP[^avr10] |  |  |  | GAP[^avr10] |  |  |

[^avr1]: N/A: no USB device peripheral (Nano/Mega use a USB-UART bridge chip, ATtiny85 has none) (nano/usb-cdc, nano-old/usb-cdc, mega/usb-cdc, attiny85/usb-cdc)
[^avr2]: N/A: no RTOS shipped by arduino:avr/ATTinyCore; 2-8 KB SRAM (ATtiny85 512 B) (nano/rtos, nano-old/rtos, mega/rtos, attiny85/rtos)
[^avr3]: N/A: classic AVR has no DMA (nano/dma, nano-old/dma, mega/dma, attiny85/dma)
[^avr4]: N/A: no radio (nano/wireless, nano-old/wireless, mega/wireless, attiny85/wireless, ch552/wireless)
[^avr5]: N/A: single core (nano/multicore, nano-old/multicore, mega/multicore, attiny85/multicore, ch552/multicore)
[^avr6]: N/A: 8051 core with 256 B iRAM + 1 KB xRAM; CH55xDuino ships no RTOS (ch552/rtos)
[^avr7]: N/A: CH552 DMA exists only inside the USB/SPI blocks, no general-purpose DMA (ch552/dma)
[^avr8]: GAP: CH552 has sleep/wake-up (PCON PD, WAKE_CTRL) but CH55xDuino has no sleep API and USB CDC must be torn down first; not done (ch552/low-power)
[^avr9]: GAP: ATtiny13A needs MicroCore (not installed); only baremetal Makefile builds possible with MCU=attiny13a (attiny13a/template)
[^avr10]: GAP: MicroCore not installed (target not in sdk.sh) (attiny13a/blink, attiny13a/button-irq, attiny13a/uart-hello, attiny13a/timer, attiny13a/pwm-fade, attiny13a/adc-read, ...)

## bouffalo

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| bl616 | OK (minimal, rtos (new)) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma (new)) | OK | OK | n/a[^bouffalo1] |
| bl702 | OK (minimal, rtos (new)) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma (new)) | OK | GAP[^bouffalo2] | n/a[^bouffalo3] |
| bl808 | OK | OK (blink) | OK | OK (uart-hello) | OK (timer (target added)) | OK | OK | OK | GAP[^bouffalo4] | OK | OK (dma (new)) | GAP[^bouffalo5] | GAP[^bouffalo6] | GAP[^bouffalo7] |
| bl616cl | GAP[^bouffalo8] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] |
| bl702l | GAP[^bouffalo8] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] |
| bl808-d0 | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] | GAP[^bouffalo9] |

[^bouffalo1]: N/A: BL616 is single-core (one E907 RV32) (bl616/multicore)
[^bouffalo2]: GAP: BLE: SDK 2.3.37 ships no libblecontroller_bl702_*.a; legacy 2.2.0 only m0s1p (peripheral-only, CONFIG_BT_OBSERVER 0) so no scan; Wi-Fi N/A (no Wi-Fi radio) (bl702/wireless)
[^bouffalo3]: N/A: BL702 is single-core (one E24 RV32) (bl702/multicore)
[^bouffalo4]: GAP: legacy SDK 2.2.0 bflb_usb_v2.c only defines BL616 USB registers/IRQ; no BL808 USB device port (bl808/usb-cdc)
[^bouffalo5]: GAP: legacy SDK has only low-level bl808_hbn.h/bl808_pds.h, no pm_* API and no bl808 PMU example; hand-written HBN sequence would be unverifiable (bl808/low-power)
[^bouffalo6]: GAP: neither SDK ships BL808 Wi-Fi/BLE libraries (legacy wifi6 lib and btblecontroller libs are BL616/BL702 only) (bl808/wireless)
[^bouffalo7]: GAP: M0+D0+LP needs three images (legacy examples/bl808_triplecore: separate CPU_ID=m0/d0/lp builds, D0 is 64-bit) plus multi-image flash config; not simple, not attempted (bl808/multicore)
[^bouffalo8]: GAP: not owned (SDK-only target): not built; no LED pin in the SDK dk board file, a template would need a guessed pin (bl616cl/template, bl702l/template)
[^bouffalo9]: GAP: not owned (SDK-only target): not built (bl616cl/blink, bl616cl/button-irq, bl616cl/uart-hello, bl616cl/timer, bl616cl/pwm-fade, bl616cl/adc-read, ...)

## esp-idf

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| esp32 | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (ledc-pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^esp-idf1] | OK (rtos-tasks) | GAP[^esp-idf2] | OK (deep-sleep) | OK (wifi-scan,ble-scan) | OK (multicore) |
| esp32s3 | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (ledc-pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK (rtos-tasks) | OK (dma-memcpy) | OK (deep-sleep) | OK (wifi-scan,ble-scan) | OK (multicore) |
| esp32c3 | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (ledc-pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK (rtos-tasks) | OK (dma-memcpy) | OK (deep-sleep) | OK (wifi-scan,ble-scan) | n/a[^esp-idf3] |
| esp32c6 | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (ledc-pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK (rtos-tasks) | OK (dma-memcpy) | OK (deep-sleep) | OK (wifi-scan,ble-scan) | n/a[^esp-idf4] |
| esp32p4 | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (ledc-pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK (rtos-tasks) | OK (dma-memcpy) | OK (deep-sleep) | n/a[^esp-idf5] | OK (multicore) |

Extra examples: `led-strip` (esp32:OK, esp32s3:OK, esp32c3:OK, esp32c6:OK, esp32p4:OK); `nvs-storage` (esp32:OK, esp32s3:OK, esp32c3:OK, esp32c6:OK, esp32p4:OK)

[^esp-idf1]: N/A: classic ESP32 has no USB peripheral (USB port = CH340 bridge on UART0) (esp32/usb-cdc)
[^esp-idf2]: GAP: no GDMA/async-memcpy engine on classic ESP32; DMA only via SPI/I2S peripherals (needs wiring/loopback), not done (esp32/dma)
[^esp-idf3]: N/A: single-core RISC-V (esp32c3/multicore)
[^esp-idf4]: N/A: single-core HP RISC-V (LP core not SMP) (esp32c6/multicore)
[^esp-idf5]: N/A: ESP32-P4 has no radio (C6 companion via esp-hosted out of scope) (esp32p4/wireless)

## freertos

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| blackpill-f411 | OK (minimal, rtos-full) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma-uart) | OK | n/a[^freertos1] | n/a[^freertos2] |
| weact-g431 | OK (minimal, rtos-full) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma-uart) | OK | n/a[^freertos1] | n/a[^freertos2] |
| mini-h743 | OK (minimal, rtos-full) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma-uart) | OK | n/a[^freertos1] | n/a[^freertos2] |
| bluepill-f103 | OK (minimal, rtos-full) | OK (blink) | n/a[^freertos3] | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma-uart) | OK | n/a[^freertos1] | n/a[^freertos2] |

Extra examples: `event-groups` (blackpill-f411:OK, weact-g431:OK, mini-h743:OK, bluepill-f103:OK); `mutex` (blackpill-f411:OK, weact-g431:OK, mini-h743:OK, bluepill-f103:OK); `stack-overflow-hook` (blackpill-f411:OK, weact-g431:OK, mini-h743:OK, bluepill-f103:OK); `sw-timers` (blackpill-f411:OK, weact-g431:OK, mini-h743:OK, bluepill-f103:OK); `uart-cli` (blackpill-f411:OK, weact-g431:OK, mini-h743:OK, bluepill-f103:OK)

[^freertos1]: N/A: no radio on STM32F1/F4/G4/H7 (blackpill-f411/wireless, weact-g431/wireless, mini-h743/wireless, bluepill-f103/wireless)
[^freertos2]: N/A: single Cortex-M core (blackpill-f411/multicore, weact-g431/multicore, mini-h743/multicore, bluepill-f103/multicore)
[^freertos3]: N/A: BluePill has no user button (board layer BOARD_HAS_BUTTON 0) (bluepill-f103/button-irq)

## k210

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| m1n | OK (minimal, rtos) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | n/a[^k2101] | OK (i2c-scan) | n/a[^k2102] | OK | OK (dma) | OK | n/a[^k2103] | OK (dual-core) |
| cm1 | OK (minimal, rtos) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | n/a[^k2101] | OK (i2c-scan) | n/a[^k2102] | OK | OK (dma) | OK | n/a[^k2103] | OK (dual-core) |

[^k2101]: N/A: K210 has no ADC peripheral (m1n/adc-read, cm1/adc-read)
[^k2102]: N/A: K210 has no USB peripheral; the USB port is a CH340/CH552 UART bridge for ISP (m1n/usb-cdc, cm1/usb-cdc)
[^k2103]: N/A: K210 has no radio and the M1n / CM-1 boards carry no Wi-Fi module (m1n/wireless, cm1/wireless)

## k230

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| k230_rtos_01studio | OK (minimal, rtos) | OK (gpio-blink) | OK (button-irq) | OK (uart) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK | OK (threads-mq) | GAP[^k2301] | OK | OK (wifi-scan) | n/a[^k2302] |
| k230d_rtos_bpi_zero | OK (minimal, rtos) | OK (gpio-blink) | OK (button-irq) | OK (uart) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK | OK (threads-mq) | GAP[^k2301] | OK | OK (wifi-scan) | n/a[^k2302] |

Extra examples: `hello` (k230_rtos_01studio:OK, k230d_rtos_bpi_zero:OK)

[^k2301]: GAP: K230 has PDMA/SDMA but rtsmart_hal exposes no user-space DMA API (kernel-internal to drivers) (k230_rtos_01studio/dma, k230d_rtos_bpi_zero/dma)
[^k2302]: N/A: little core runs Linux, big core RT-Smart; an RT-Smart user app sees one core only (k230_rtos_01studio/multicore, k230d_rtos_bpi_zero/multicore)

## pico

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| pico | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma) | OK | n/a[^pico1] | OK (multicore-fifo) |
| zero | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma) | OK | n/a[^pico1] | OK (multicore-fifo) |
| pico2 | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma) | OK | n/a[^pico1] | OK (multicore-fifo) |
| pico2-riscv | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma) | OK | n/a[^pico1] | OK (multicore-fifo) |
| rp2350-zero | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma) | OK | n/a[^pico1] | OK (multicore-fifo) |
| rp2350-zero-riscv | OK | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | OK | OK (dma) | OK | n/a[^pico1] | OK (multicore-fifo) |

Extra examples: `flash-storage` (pico:OK, zero:OK, pico2:OK, pico2-riscv:OK, rp2350-zero:OK, rp2350-zero-riscv:OK); `pio-ws2812` (pico:OK, zero:OK, pico2:OK, pico2-riscv:OK, rp2350-zero:OK, rp2350-zero-riscv:OK)

[^pico1]: N/A: RP2040/RP2350 have no radio (only Pico W/2 W with CYW43, not owned) (pico/wireless, zero/wireless, pico2/wireless, pico2-riscv/wireless, rp2350-zero/wireless, rp2350-zero-riscv/wireless)

## py32

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| py32f003x8 | OK (minimal) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^py321] | OK | OK (dma) | OK (low-power) | n/a[^py322] | n/a[^py323] |
| py32f002ax5 | OK (minimal) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^py321] | n/a[^py324] | n/a[^py325] | OK (low-power) | n/a[^py322] | n/a[^py323] |
| py32f030x8 | OK (minimal) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^py321] | OK | OK (dma) | OK (low-power) | n/a[^py322] | n/a[^py323] |
| py32f071x8 | OK (minimal) | OK (blink) | GAP[^py326] | OK (uart-hello) | GAP[^py326] | GAP[^py326] | GAP[^py326] | GAP[^py326] | GAP[^py327] | OK | GAP[^py326] | GAP[^py326] | n/a[^py322] | n/a[^py323] |

Extra examples: `template-rtos` (py32f003x8:OK, py32f030x8:OK, py32f071x8:OK)

[^py321]: N/A: no USB peripheral on PY32F002A/F003/F030 (datasheets) (py32f003x8/usb-cdc, py32f002ax5/usb-cdc, py32f030x8/usb-cdc)
[^py322]: N/A: no radio (py32f003x8/wireless, py32f002ax5/wireless, py32f030x8/wireless, py32f071x8/wireless)
[^py323]: N/A: single Cortex-M0+ core (py32f003x8/multicore, py32f002ax5/multicore, py32f030x8/multicore, py32f071x8/multicore)
[^py324]: N/A: 3 KB SRAM: the 3-task + queue + mutex demo overflows RAM by 640 B (kernel alone would fit with ~1.5 KB heap) (py32f002ax5/rtos)
[^py325]: N/A: PY32F002A has no DMA controller (no DMA1 in py32f002ax5.h) (py32f002ax5/dma)
[^py326]: GAP: PY32F071 has only the HAL driver in py32f0-template; this example is LL-only and was not ported to HAL in this pass (py32f071x8/button-irq, py32f071x8/timer, py32f071x8/pwm-fade, py32f071x8/adc-read, py32f071x8/i2c-scan, py32f071x8/dma, ...)
[^py327]: GAP: PY32F071 HAS a USB 2.0 FS device (datasheet V0.8 table 1-1, USBD in py32f071x8.h) but py32f0-template ships no USB stack/HAL driver (py32f071x8/usb-cdc)

## stm32

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| blackpill-f411 | OK (minimal, with-tests) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | n/a[^stm321] | OK (dma-uart) | OK (low-power) | n/a[^stm322] | n/a[^stm323] |
| weact-g431 | OK (minimal, with-tests) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | n/a[^stm321] | OK (dma-uart) | OK (low-power) | n/a[^stm322] | n/a[^stm323] |
| mini-h743 | OK (minimal, with-tests) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | n/a[^stm321] | OK (dma-uart) | OK (low-power) | n/a[^stm322] | n/a[^stm324] |
| bluepill-f103 | OK (minimal, with-tests) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | n/a[^stm321] | OK (dma-uart) | OK (low-power) | n/a[^stm322] | n/a[^stm325] |
| weact-g031 | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | n/a[^stm327] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | n/a[^stm322] | n/a[^stm328] |
| h562-minsys | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | GAP[^stm326] | n/a[^stm322] | n/a[^stm328] |

[^stm321]: N/A: see freertos SDK (rtos-tasks there); this SDK is bare HAL (blackpill-f411/rtos, weact-g431/rtos, mini-h743/rtos, bluepill-f103/rtos)
[^stm322]: N/A: no radio on the chip (blackpill-f411/wireless, weact-g431/wireless, mini-h743/wireless, bluepill-f103/wireless, weact-g031/wireless, h562-minsys/wireless)
[^stm323]: N/A: single core (Cortex-M4) (blackpill-f411/multicore, weact-g431/multicore)
[^stm324]: N/A: single core (Cortex-M7) (mini-h743/multicore)
[^stm325]: N/A: single core (Cortex-M3) (bluepill-f103/multicore)
[^stm326]: GAP: no Cube pack G0/H5 installed; covered by zephyr (weact-g031/template, weact-g031/blink, weact-g031/button-irq, weact-g031/uart-hello, weact-g031/timer, weact-g031/pwm-fade, ...)
[^stm327]: N/A: STM32G031 has no USB peripheral (weact-g031/usb-cdc)
[^stm328]: N/A: single core (weact-g031/multicore, h562-minsys/multicore)

## wch

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| ch32v003 | OK (minimal) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | n/a[^wch1] | n/a[^wch2] | OK (dma) | OK (standby) | n/a[^wch3] | n/a[^wch4] |
| ch32v203 | OK (minimal, evt-minimal) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | GAP[^wch5] | OK (dma) | OK (standby) | n/a[^wch3] | n/a[^wch4] |
| ch32v307 | OK (minimal) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | OK (usb-cdc) | GAP[^wch5] | OK (dma) | OK (standby) | n/a[^wch3] | n/a[^wch4] |
| ch32h417 | OK (minimal) | OK (blink) | OK (button-irq) | OK (uart-hello) | OK (timer) | OK (pwm-fade) | OK (adc-read) | OK (i2c-scan) | GAP[^wch6] | n/a[^wch7] | OK (dma) | GAP[^wch8] | n/a[^wch3] | OK (dual-core) |

Extra examples: `template-evt` (ch32v307:GAP)

[^wch1]: N/A: CH32V003 has no USB peripheral (ch32v003/usb-cdc)
[^wch2]: N/A: ch32fun ships no RTOS; 2 KB SRAM (ch32v003/rtos)
[^wch3]: N/A: no radio (ch32v003/wireless, ch32v203/wireless, ch32v307/wireless, ch32h417/wireless)
[^wch4]: N/A: single core (ch32v003/multicore, ch32v203/multicore, ch32v307/multicore)
[^wch5]: GAP: openwch EVT ships FreeRTOS/RT-Thread (EVT/EXAM/FreeRTOS); EVT FreeRTOS build not wrapped (needs the EVT port + WCH fast-interrupt setup) (ch32v203/rtos, ch32v307/rtos)
[^wch6]: GAP: CH32H417 has USBFS/USBHS, but ch32fun fsusb.c has no CH32H41x clock setup and hsusb.h (CH32H4xx) has no CDC class; not ported (ch32h417/usb-cdc)
[^wch7]: N/A: ch32fun ships no RTOS and no openwch EVT for H417 in the tree (ch32h417/rtos)
[^wch8]: GAP: no ch32fun H41x low-power example; PWR/standby registers differ from V2/V3, not ported (ch32h417/low-power)

## zephyr

| target | template | blink | button-irq | uart-hello | timer | pwm-fade | adc-read | i2c-scan | usb-cdc | rtos | dma | low-power | wireless | multicore |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| blackpill_f411ce | OK | OK | OK | OK | OK | OK | OK | OK | OK | OK (threads-msgq) |  |  |  |  |
| ch32v003 |  |  |  | OK |  |  | OK |  |  |  |  |  |  |  |
| esp32c3 |  | OK |  | OK | OK |  |  | OK |  | OK (threads-msgq) |  |  | OK (ble-beacon) |  |
| esp32s3 |  |  |  | OK |  |  |  | OK |  | OK (threads-msgq) |  |  |  |  |
| promicro_nrf52840 | OK | OK |  | OK | OK | OK |  | OK | OK | OK (threads-msgq) |  |  | OK (ble-beacon) |  |
| rpi_pico |  | OK |  |  |  | OK | OK | OK | OK |  |  |  |  |  |
| rpi_pico2 | OK | OK |  | OK | OK | OK | OK | OK | OK | OK (threads-msgq) |  |  |  |  |
| weact_g431 | OK | OK | OK | OK | OK |  |  |  | OK |  |  |  |  |  |
| weact_h562 | OK | OK | OK |  | OK |  |  |  |  |  |  |  |  |  |
| weact_h743 |  | OK | OK |  |  |  |  |  | OK |  |  |  |  |  |

Extra examples: `settings` (esp32c3:OK, promicro_nrf52840:OK, weact_g431:OK); `shell` (blackpill_f411ce:OK, esp32c3:OK, promicro_nrf52840:OK, rpi_pico2:OK, weact_g431:OK)

