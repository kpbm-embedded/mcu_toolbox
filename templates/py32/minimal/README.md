# {{NAME}} (py32 minimal, {{BOARD}})

PY32F0xx project on the Puya LL driver (PY32F071: HAL driver, the only one the template ships for F07x). Edit `main.c`;
pins are in `board.h` (Puya example pins, unconfirmed for your board: LED PB5 + USART1 PA2/PA3 on F002A/F003/F030,
LED PC13 + USART2 PA2/PA3 on F071).

    source mcu/py32/env.sh
    make TARGET={{TARGET}} BUILD=$PWD/build
    pyocd flash --pack "$PY32_PACK" -t {{TARGET}} build/app.hex

The Makefile uses CMSIS, the Puya LL/HAL driver and linker scripts from `$PY32_TEMPLATE` (read only); for py32f071x8 it
generates a 64 KB / 8 KB linker script from the template's `py32f072xb.ld`. `hal07x_it.c` and `py32f07x_hal_conf.h` are
used only by the F071 build. A `project.mk` with `RTOS := y` adds the template's FreeRTOS (see the `rtos` template).
