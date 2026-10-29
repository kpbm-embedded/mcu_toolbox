# k210 / uart-hello

Prints a greeting and the CPU clock, echoes received bytes and prints an alive counter every 2 s. Uses the UARTHS (IO4/IO5, the same port kflash uses), so no extra wiring.

Status: builds; untested on hardware.

## Pins
Source: Sipeed Maix family convention (Maix Bit V2.0 schematic: BOOT = IO16, ISP download on IO4/IO5; MaixDuino pin table:
IO12 = LED_B, IO13 = LED_R, IO14 = LED_G). **Not confirmed for the M1n Maix Nano or the CM-1 CanMV board**; the Nano datasheet only
states "1 reset button, 1 user button" without a pin table. All pins are `#define`s in `board.h` and can be overridden with
`add_definitions(-DPIN_LED_R=..)`. Check the real LED/key wiring before relying on it.
Expected: `hello from K210, cpu 400 MHz` (value depends on the PLL setting), then `alive N s`; typed characters are echoed as `echo: x (0x78)`.

## Build
```
mcu/templates/tools/check.sh --only uart-hello k210        # builds both targets into $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k210/uart-hello/<target>/
# by hand:
source mcu/k210/env.sh
cmake -S . -B $K210_BUILD/uart-hello -DSDK=$K210_HOME/kendryte-standalone-sdk -DTOOLCHAIN=$K210_TOOLCHAIN -DBOARD=m1n -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -C $K210_BUILD/uart-hello -j16
```
The SDK is used in place through its external-project mode (`cmake/common.cmake` + `cmake/executable.cmake`); nothing is copied into or edited in the SDK tree.

## Flash / monitor
`kflash -p /dev/ttyUSB0 -b 1500000 -B goE <build>/uart-hello.bin` then `python -m serial.tools.miniterm /dev/ttyUSB0 115200` (kflash board id `goE` is a guess for these boards, see mcu/k210/README.md).

