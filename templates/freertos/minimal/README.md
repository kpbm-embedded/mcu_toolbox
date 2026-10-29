# freertos / minimal

Two FreeRTOS tasks on an STM32 board: an LED blinker ({{LED_PIN}}) and a periodic logger. Heap: {{HEAP_KB}} KiB (heap_4).

Kernel FreeRTOS-Kernel V11.3.2 (mcu/freertos), shared config `mcu/freertos/common/FreeRTOSConfig.h`, board layer `mcu/stm32/boards/{{TARGET}}`
(pin/clock sources cited there). Ports: ARM_CM4F (F411, G431), ARM_CM7/r0p1 (H743), ARM_CM3 (F103).

## Build
```
. mcu/freertos/env.sh
cmake -S . -B build -G Ninja -DBOARD={{TARGET}} && cmake --build build -j16     # -> build/{{NAME}}.{elf,bin,hex}
```
Flash: `st-flash --reset write build/{{NAME}}.bin 0x08000000`. Console: USART1 PA9 TX / PA10 RX, 115200 8N1.

## Where to start editing
`src/main.c`. Console output from tasks: `rtos_log()` / `rtos_print()` (mutex-protected). Kernel options: copy `FreeRTOSConfig.h` next to your sources
and put its directory first in the include path to override the shared one.
Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103. Status of the M7/M3 ports: builds only; untested on hardware.
