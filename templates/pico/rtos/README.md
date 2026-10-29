# {{NAME}} (rtos template, pico)

FreeRTOS SMP skeleton: a blink task (LED pin {{LED_PIN}}) and a main task printing a tick on UART0 (GP0 TX / GP1 RX, 115200). Edit `main.c`; kernel settings in `FreeRTOSConfig.h` (both cores by default; set `configNUMBER_OF_CORES` 1 for one).

Instantiate: `python -I mcu/templates/tools/instantiate.py pico rtos <dest> --name <name> --target <target>`.
Targets: pico, zero (RP2040-Zero), pico2, pico2-riscv, rp2350-zero, rp2350-zero-riscv. Generated for `{{TARGET}}`.
LED pin source: `PICO_DEFAULT_LED_PIN` 25 in the SDK `pico.h` / `pico2.h`; the Zero boards have no plain LED (only a WS2812 on GPIO16), so 15 means an external LED + resistor on GP15.

FreeRTOS kernel: needs FreeRTOS-Kernel with `portable/ThirdParty/GCC/RP2040` and `Community-Supported-Ports/GCC/RP2350_*` (the shared one is `mcu/freertos/FreeRTOS-Kernel`). Located via `-DFREERTOS_KERNEL_PATH`, `$FREERTOS_KERNEL_PATH`, or `$MCU_ROOT/freertos/FreeRTOS-Kernel`.

## Build
```sh
source mcu/pico/env.sh
export FREERTOS_KERNEL_PATH=$PWD/mcu/freertos/FreeRTOS-Kernel   # or MCU_ROOT=$PWD/mcu
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/<project>/{{TARGET}} -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] \
  -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR   # RISC-V: PICO_TOOLCHAIN_PATH_RISCV
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/<project>/{{TARGET}} -j16
```
PICO_BOARD: pico=pico, zero=waveshare_rp2040_zero, pico2/pico2-riscv=pico2, rp2350-zero(-riscv)=waveshare_rp2350_zero.
Flash: BOOTSEL + `picotool load -x {{NAME}}.uf2`. Console: UART0 via a USB-UART adapter (`picocom -b 115200 /dev/ttyUSB0`).
