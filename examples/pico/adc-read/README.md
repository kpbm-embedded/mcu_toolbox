# pico example: adc-read

Status: **builds; untested on hardware**.

Prints ADC0 on GP26 and the die temperature (ADC channel 4, formula from the RP2040 datasheet / pico-examples `adc/read_temp`). Wire a potentiometer wiper to GP26, ends to 3V3 and GND.

GP26 = ADC0 on RP2040 and RP2350A (pinout in the Pico/Pico 2 datasheets; GP26 is broken out on both Zero boards per the Waveshare wiki).

## Build

```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/adc-read/<target> \
  -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/adc-read/<target> -j16
```

Targets map to PICO_BOARD like `mcu/pico/sdk.sh`: pico=pico, zero=waveshare_rp2040_zero, pico2=pico2, pico2-riscv=pico2+rp2350-riscv (toolchain `$PICO_TOOLCHAIN_PATH_RISCV`), rp2350-zero=waveshare_rp2350_zero, rp2350-zero-riscv=waveshare_rp2350_zero+rp2350-riscv.
Flash: hold BOOTSEL while plugging in, then `picotool load -x adc-read.uf2` or copy the .uf2 to the drive. Monitor: `../mcu monitor pico /dev/ttyACM0`.
