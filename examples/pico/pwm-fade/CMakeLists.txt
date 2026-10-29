cmake_minimum_required(VERSION 3.13)
# Out-of-tree Pico SDK app: PICO_SDK_PATH comes from mcu/pico/env.sh.
# Select the board with -DPICO_BOARD=... (and -DPICO_PLATFORM=rp2350-riscv for RISC-V).
include(pico_sdk_import.cmake)
project(pwm-fade C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()
add_executable(pwm-fade main.c)
target_link_libraries(pwm-fade pico_stdlib hardware_pwm)
pico_enable_stdio_usb(pwm-fade 1)
pico_enable_stdio_uart(pwm-fade 0)

pico_add_extra_outputs(pwm-fade)
