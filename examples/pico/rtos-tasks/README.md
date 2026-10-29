# pico example: rtos-tasks

Status: **builds; untested on hardware**.

FreeRTOS SMP on both cores (Cortex-M0+, Cortex-M33 and Hazard3 RISC-V): a blink task toggles the LED every 250 ms, a producer task sends a counter into a queue once per second, a consumer task receives it, and a mutex serialises `printf` on UART0. Stack-overflow and malloc-failed hooks call `panic()`.

| Board (target) | LED | notes |
|---|---|---|
| Pico (`pico`), Pico 2 (`pico2`, `pico2-riscv`) | GPIO25 on-board LED | `PICO_DEFAULT_LED_PIN` in `pico-sdk/src/boards/include/boards/pico.h` / `pico2.h` |
| RP2040-Zero (`zero`), RP2350-Zero (`rp2350-zero`, `rp2350-zero-riscv`) | none; external LED+resistor on GP15 | board headers `waveshare_rp2040_zero.h` / `waveshare_rp2350_zero.h` define no plain LED; the only on-board LED is the WS2812 on GPIO16 (Waveshare wiki: https://www.waveshare.com/wiki/RP2040-Zero and https://www.waveshare.com/wiki/RP2350-Zero) |

Console: UART0 TX GP0 / RX GP1, 115200 8N1 (`PICO_DEFAULT_UART_*` in the board headers).

FreeRTOS kernel: the out-of-tree SMP ports in FreeRTOS-Kernel `portable/ThirdParty/GCC/RP2040` and `portable/ThirdParty/Community-Supported-Ports/GCC/RP2350_ARM_NTZ` / `RP2350_RISC-V`, located via `FreeRTOS_Kernel_import.cmake` (copied from `pico-examples/freertos`). `CMakeLists.txt` looks for the kernel in `-DFREERTOS_KERNEL_PATH`, then `$FREERTOS_KERNEL_PATH`, then `$MCU_ROOT/freertos/FreeRTOS-Kernel`, then `../../../tool/freertos/FreeRTOS-Kernel` (the shared kernel in 32_04_freertos; read only). `FreeRTOSConfig.h` is trimmed from `pico-examples/freertos/FreeRTOSConfig_examples_common.h`; set `configNUMBER_OF_CORES` to 1 for single-core.

Expected:

```
rtos-tasks: FreeRTOS V11.1.0+, 2 core(s)
producer: sent 0 (core 0)
consumer: got 0 (core 1), free heap 6xxxx
```

## Build

```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/rtos-tasks/<target> \
  -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/rtos-tasks/<target> -j16
```

Targets map to PICO_BOARD like `mcu/pico/sdk.sh`: pico=pico, zero=waveshare_rp2040_zero, pico2=pico2, pico2-riscv=pico2+rp2350-riscv (toolchain `$PICO_TOOLCHAIN_PATH_RISCV`), rp2350-zero=waveshare_rp2350_zero, rp2350-zero-riscv=waveshare_rp2350_zero+rp2350-riscv.
Flash: hold BOOTSEL while plugging in, then `picotool load -x rtos-tasks.uf2` or copy the .uf2 to the drive. Monitor (UART0 via a USB-UART adapter on GP0/GP1): `picocom -b 115200 /dev/ttyUSB0`.
