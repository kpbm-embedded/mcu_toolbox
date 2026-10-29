# bouffalo / rtos-bl808 (BL808 M0 core, legacy SDK, FreeRTOS)

BL808 is not supported by Bouffalo SDK v2.3.37; this uses `mcu/bouffalo/bouffalo_sdk_legacy` (branch master_legacy, SDK 2.2.0), building the M0 core (`CPU_ID=m0`).
FreeRTOS skeleton (kernel V10.6.2 shipped in `components/os/freertos`, `set(CONFIG_FREERTOS 1)` in `proj.conf`): a blink task toggles `LED_PIN`, a main task prints a counter on the console UART. `FreeRTOSConfig.h` carries the CLINT mtime addresses the legacy RISC-V port needs. Config is `proj.conf` (the legacy SDK ignores `defconfig`/`Kconfig`).

Instantiate: `python3 -I mcu/templates/tools/instantiate.py bouffalo rtos-bl808 ~/proj/{name} --name {name}`
Build: `. mcu/bouffalo/env.sh && make` (needs a `flash_prog_cfg.ini` next to the Makefile for `make flash`; copy it from `bouffalo_sdk_legacy/examples/helloworld`).
Flash: via the M1s Dock's onboard BL702 USB-UART or BOOT+UART, see `mcu/bouffalo/README.md`.
`LED_PIN` (GPIO8) is not verified against the M1s Dock schematic.
