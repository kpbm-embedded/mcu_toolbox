# pico example: flash-storage

Status: **builds; untested on hardware**.

Stores and increments a boot counter in the last 4 KiB flash sector using `flash_safe_execute`. WARNING: erases the last sector of the board's flash (it is outside the program image). Reset the board to see the count rise.

No external hardware. Flash size comes from `PICO_FLASH_SIZE_BYTES` in the board header, so the last sector is always inside the chip.

## Build

```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/flash-storage/<target> \
  -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/flash-storage/<target> -j16
```

Targets map to PICO_BOARD like `mcu/pico/sdk.sh`: pico=pico, zero=waveshare_rp2040_zero, pico2=pico2, pico2-riscv=pico2+rp2350-riscv (toolchain `$PICO_TOOLCHAIN_PATH_RISCV`), rp2350-zero=waveshare_rp2350_zero, rp2350-zero-riscv=waveshare_rp2350_zero+rp2350-riscv.
Flash: hold BOOTSEL while plugging in, then `picotool load -x flash-storage.uf2` or copy the .uf2 to the drive. Monitor: `../mcu monitor pico /dev/ttyACM0`.
