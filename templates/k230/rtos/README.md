# {{NAME}} (k230 rtos, {{BOARD}})

RT-Smart user-space app built out of tree against `k230_rtos_sdk` (the SDK tree is only read). Needs the SDK output of the board
(`./mcu build k230 {{TARGET}}`) and its defconfig selected; see the Makefile header. Edit `main.c`; GPIO/UART pins in `board.h` are
unconfirmed defaults.

    source mcu/k230/env.sh
    flock $K230_SDK/.mcu-build.lock bash -c 'make -C $K230_SDK {{TARGET}}_defconfig && make -C $K230_SDK .autoconf && make K230_SDK=$K230_SDK OUT=$PWD/build'
    # copy build/app.elf to the SD card /sdcard/app/ of a {{TARGET}} image and run it from msh
