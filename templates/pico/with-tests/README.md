# {{NAME}} (with-tests template, pico)

Firmware in `src/`; hardware-independent logic (`logic.c`) is unit-tested on the host with Unity (vendored in `tests/unity`, MIT, from the pico-sdk tinyusb tree).

Instantiate: `python -I mcu/templates/tools/instantiate.py pico with-tests <dest> --name <name> --target <target>`.
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
## Host tests
```sh
source mcu/pico/env.sh
cmake -G Ninja -S tests -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/<project>/host-tests && cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/<project>/host-tests -j16 && ctest --test-dir $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/<project>/host-tests --output-on-failure
```
