# {{NAME}} (multicore template, pico)

Core 1 blinks the LED and pushes a counter to core 0 via the inter-core FIFO; core 0 prints it. Works on the RP2040 and on RP2350 ARM and Hazard3 cores.

Instantiate: `python -I mcu/templates/tools/instantiate.py pico multicore <dest> --name <name> --target <target>`.
Targets: pico, zero (RP2040-Zero), pico2, pico2-riscv, rp2350-zero, rp2350-zero-riscv. Generated for `{{TARGET}}` (LED pin {{LED_PIN}}).

## Build
```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/<project>/{{TARGET}} -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] \
  -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR   # RISC-V: PICO_TOOLCHAIN_PATH_RISCV
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/<project>/{{TARGET}} -j16
```
PICO_BOARD: pico=pico, zero=waveshare_rp2040_zero, pico2/pico2-riscv=pico2, rp2350-zero(-riscv)=waveshare_rp2350_zero.
Flash: BOOTSEL + `picotool load -x {{NAME}}.uf2`. Output is USB CDC (/dev/ttyACM0).

