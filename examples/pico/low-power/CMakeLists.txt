cmake_minimum_required(VERSION 3.13)
# Out-of-tree Pico SDK app: PICO_SDK_PATH comes from mcu/pico/env.sh.
# Select the board with -DPICO_BOARD=... (and -DPICO_PLATFORM=rp2350-riscv for RISC-V).
include(pico_sdk_import.cmake)
project(low-power C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()
add_executable(low-power main.c)
target_link_libraries(low-power pico_stdlib pico_low_power)   # pico_low_power: pico-sdk >= 2.3
pico_enable_stdio_usb(low-power 0)    # USB is stopped in dormant: console on UART0
pico_enable_stdio_uart(low-power 1)
pico_add_extra_outputs(low-power)
