# {{NAME}} (k210 minimal, board {{BOARD}})

Bare-metal project on the Kendryte standalone SDK, used out of tree (nothing is copied into the SDK).
Start editing `main.c`; LED/key pins live in `board.h` (defaults follow the Maix family and are unconfirmed for M1n/CM-1, see `mcu/examples/k210/blink/README.md`).

    source mcu/k210/env.sh
    cmake -S . -B $K210_BUILD/{{NAME}} -DSDK=$K210_HOME/kendryte-standalone-sdk -DTOOLCHAIN=$K210_TOOLCHAIN -DBOARD={{TARGET}} -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    make -C $K210_BUILD/{{NAME}} -j16
    kflash -p /dev/ttyUSB0 -b 1500000 -B goE $K210_BUILD/{{NAME}}/{{NAME}}.bin
