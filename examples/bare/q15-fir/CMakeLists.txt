cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only q15-fir   or   mcu build bare <chip> q15-fir && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385 stm32f411 virt-rv32")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(q15-fir C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(q15-fir src/main.c)
target_link_libraries(q15-fir PRIVATE m)
bare_firmware(q15-fir)
