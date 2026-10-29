# {{NAME}} (py32 rtos, {{BOARD}})

FreeRTOS project: `project.mk` sets `RTOS := y`, so the Makefile adds the kernel from `$PY32_TEMPLATE/Libraries/FreeRTOS`
(port GCC/ARM_CM0, heap_4). `FreeRTOSConfig.h` is Puya's `FreeRTOS/Tasks/LL_Blink` config with a 4 KB heap and mutexes on.
`main.c` starts three tasks (LED, queue producer, consumer printing under a mutex); replace them with yours.
PY32F002A (3 KB SRAM) is not a target: this layout overflows its RAM. Pins in `board.h` (Puya example pins, unconfirmed).

    source mcu/py32/env.sh
    make TARGET={{TARGET}} BUILD=$PWD/build
    pyocd flash --pack "$PY32_PACK" -t {{TARGET}} build/app.hex
