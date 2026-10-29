cmake_minimum_required(VERSION 3.20)
# {{NAME}}: bare-metal firmware (mcu bare layer). Inside `mcu shell bare`:
#   cmake -S . -B build -G Ninja -DCHIP=<chip> && cmake --build build && $MCU_BARE/run/firmware.sh build/{{NAME}}.elf
set(CHIP "{{TARGET}}" CACHE STRING "chip: nrf52840")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project({{NAME}} C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable({{NAME}} src/main.c)
set(CHIP_RUN "none")   # linked behind the SoftDevice: not runnable in Renode without it
bare_firmware({{NAME}} LD ${CMAKE_CURRENT_SOURCE_DIR}/link.ld)
find_program(PYTHON3 python3 REQUIRED)
add_custom_command(TARGET {{NAME}} POST_BUILD COMMAND ${PYTHON3} -I ${CMAKE_CURRENT_SOURCE_DIR}/uf2conv.py {{NAME}}.bin 0x26000 {{NAME}}.uf2 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
