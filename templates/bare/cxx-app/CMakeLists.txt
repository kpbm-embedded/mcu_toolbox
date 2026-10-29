cmake_minimum_required(VERSION 3.20)
# {{NAME}}: bare-metal firmware (mcu bare layer). Inside `mcu shell bare`:
#   cmake -S . -B build -G Ninja -DCHIP=<chip> && cmake --build build && $MCU_BARE/run/firmware.sh build/{{NAME}}.elf
set(CHIP "{{TARGET}}" CACHE STRING "chip: mps2-an385 virt-rv32 stm32f411")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project({{NAME}} C CXX ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable({{NAME}} src/main.cpp)
target_compile_options({{NAME}} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-threadsafe-statics>)
bare_firmware({{NAME}})
