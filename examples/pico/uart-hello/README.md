# pico example: uart-hello

Status: **builds; untested on hardware**.

Prints a counter once per second on UART0. Connect a USB-UART adapter: adapter RX to GP0, TX to GP1, GND to GND.

Pins: UART0 TX GP0, RX GP1 (`PICO_DEFAULT_UART_*` in the SDK board headers, identical for pico, pico2 and both Zero boards).

Expected: `hello from UART0, n=0` ... at 115200 8N1 (`picocom -b 115200 /dev/ttyUSB0`).

## Build

```sh
source mcu/pico/env.sh
cmake -G Ninja -S . -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/uart-hello/<target> \
  -DPICO_BOARD=<board> [-DPICO_PLATFORM=rp2350-riscv] -DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH -Dpicotool_DIR=$picotool_DIR
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/pico/uart-hello/<target> -j16
```

Targets map to PICO_BOARD like `mcu/pico/sdk.sh`: pico=pico, zero=waveshare_rp2040_zero, pico2=pico2, pico2-riscv=pico2+rp2350-riscv (toolchain `$PICO_TOOLCHAIN_PATH_RISCV`), rp2350-zero=waveshare_rp2350_zero, rp2350-zero-riscv=waveshare_rp2350_zero+rp2350-riscv.
Flash: hold BOOTSEL while plugging in, then `picotool load -x uart-hello.uf2` or copy the .uf2 to the drive. Monitor: `../mcu monitor pico /dev/ttyACM0`.
