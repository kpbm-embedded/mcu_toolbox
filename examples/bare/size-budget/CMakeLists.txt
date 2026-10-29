cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only size-budget   or   mcu build bare <chip> size-budget && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "ch32v003" CACHE STRING "chip: ch32v003 stm32f103")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(size-budget C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(size-budget src/main.c)
bare_firmware(size-budget)
