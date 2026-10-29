cmake_minimum_required(VERSION 3.13)
# Out-of-tree Pico SDK app: PICO_SDK_PATH comes from mcu/pico/env.sh.
# Select the board with -DPICO_BOARD=... (and -DPICO_PLATFORM=rp2350-riscv for RISC-V).
include(pico_sdk_import.cmake)
project(pio-ws2812 C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()
add_executable(pio-ws2812 main.c)
target_link_libraries(pio-ws2812 pico_stdlib hardware_pio)
pico_enable_stdio_usb(pio-ws2812 1)
pico_enable_stdio_uart(pio-ws2812 0)
pico_generate_pio_header(pio-ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)
pico_add_extra_outputs(pio-ws2812)
