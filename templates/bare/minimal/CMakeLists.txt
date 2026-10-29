cmake_minimum_required(VERSION 3.20)
# {{NAME}}: bare-metal firmware (mcu bare layer). Inside `mcu shell bare`:
#   cmake -S . -B build -G Ninja -DCHIP=<chip> && cmake --build build && $MCU_BARE/run/firmware.sh build/{{NAME}}.elf
set(CHIP "{{TARGET}}" CACHE STRING "chip: mps2-an385 stm32f411 stm32h743 stm32f103 nrf52840 virt-rv32 ch32v003")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project({{NAME}} C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable({{NAME}} src/main.c)
bare_firmware({{NAME}})
