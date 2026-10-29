#!/bin/bash
# usage: build.sh <board> [platform] ; builds blink+hello_usb from pico-examples
. "$(dirname "$0")/env.sh"
b=$1; p=${2:-}; n=$b${p:+-$p}
extra=(); [ -n "$p" ] && extra+=(-DPICO_PLATFORM=$p)
[ "$p" = rp2350-riscv ] && extra+=(-DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH_RISCV)
cmake -S "$PICO_EXAMPLES_PATH" -B "$PICO_BUILD_ROOT/$n" -DPICO_BOARD=$b "${extra[@]}" -Dpicotool_DIR=$picotool_DIR >"$PICO_BUILD_ROOT/$n.cfg.log" 2>&1 || { echo cfg fail $n; exit 1; }
cmake --build "$PICO_BUILD_ROOT/$n" -j16 --target blink hello_usb >"$PICO_BUILD_ROOT/$n.log" 2>&1 && echo ok $n || echo FAIL $n
