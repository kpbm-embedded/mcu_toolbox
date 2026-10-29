# {{NAME}} (wch minimal / ch32fun, {{BOARD}})

Edit `main.c`; pins and board strings are in `board.h` (LED/key defaults are unconfirmed for CH32V003, see
`mcu/examples/wch/blink/README.md`). Add peripherals with the ch32fun register API (`TIM2->`, `ADC1->`, `funAnalogRead`, ...).

    source mcu/wch/env.sh
    B=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/{{NAME}}; mkdir -p $B && cp -r . $B     # ch32fun writes products next to the sources
    make -C $B TARGET_NAME={{TARGET}}
    minichlink -w $B/main.bin flash -b          # or: wchisp flash $B/main.bin

The Makefile uses `$CH32FUN_REPO` read-only; the generated linker script lands in the build copy.
For per-example extra sources put `ADDITIONAL_C_FILES += ...` in an `example.mk` next to the Makefile.
