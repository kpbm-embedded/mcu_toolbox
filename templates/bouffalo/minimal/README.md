# bouffalo / minimal (BL616, BL702)

Smallest real app on Bouffalo SDK v2.3.37: blinks `LED_PIN`, prints a counter on the console UART0. Files: `main.c`, `Makefile`
(SDK `project.build`), `CMakeLists.txt`, `defconfig`/`Kconfig` (SDK v2 config), `build.sh` (out-of-tree adapter used by `check.sh`).

Targets: `bl616` (Sipeed M0s Dock, Ai-M62; board `bl616dk`), `bl702` (Sipeed M0sense; board `bl702dk`).

Instantiate: `python3 -I mcu/templates/tools/instantiate.py bouffalo minimal ~/proj/{name} --name {name} --target bl616`

Build: `. mcu/bouffalo/env.sh && make` in the project (or `./build.sh bl616 $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/mine/{name}`, which builds a copy).
Flash: hold BOOT, tap RESET, then `make flash COMX=/dev/ttyACM0` (see `mcu/bouffalo/README.md`). Console: UART0 (bl616 TX GPIO21/RX GPIO22, bl702 TX GPIO14/RX GPIO15, from the SDK board files).

`LED_PIN` (bl616 GPIO27, bl702 GPIO22) is not verified against the Sipeed schematics; fix it if nothing blinks.
