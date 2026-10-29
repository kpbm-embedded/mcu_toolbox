# {{NAME}} (k210 rtos, board {{BOARD}})

Kendryte FreeRTOS SDK (kendryte-freertos-sdk) used out of tree. `main()` runs in a FreeRTOS task; add tasks in `main.c`. Drivers are opened by name (`io_open("/dev/uart1")`, `/dev/gpio0` = high-speed GPIO, `/dev/i2c0`, `/dev/pwm0`, ...).

    source mcu/k210/env.sh
    cmake -S . -B $K210_BUILD/{{NAME}} -DSDK=$K210_HOME/kendryte-freertos-sdk -DTOOLCHAIN=$K210_TOOLCHAIN -DBOARD={{TARGET}} -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    make -C $K210_BUILD/{{NAME}} -j16
    kflash -p /dev/ttyUSB0 -b 1500000 -B goE $K210_BUILD/{{NAME}}/{{NAME}}.bin
