# pico example: i2c-scan

Status: **builds; untested on hardware**.

Probes addresses 0x08-0x77 on I2C0 every 3 s.

Pins: SDA GP4, SCL GP5 (I2C0 function; same as the Pico default I2C pins `PICO_DEFAULT_I2C_SDA_PIN` = 4 in pico.h/pico2.h). The Zero boards' own default I2C is GP6/GP7 (I2C1) but GP4/GP5 are also I2C0-capable and broken out, so one binary pinout is used for all boards. Add external pull-ups if the module has none.

## Build

```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/i2c-scan/<target> \
  -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/i2c-scan/<target> -j16
```

Targets map to PICO_BOARD like `mcu/pico/sdk.sh`: pico=pico, zero=waveshare_rp2040_zero, pico2=pico2, pico2-riscv=pico2+rp2350-riscv (toolchain `$PICO_TOOLCHAIN_PATH_RISCV`), rp2350-zero=waveshare_rp2350_zero, rp2350-zero-riscv=waveshare_rp2350_zero+rp2350-riscv.
Flash: hold BOOTSEL while plugging in, then `picotool load -x i2c-scan.uf2` or copy the .uf2 to the drive. Monitor: `../mcu monitor pico /dev/ttyACM0`.
