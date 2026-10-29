cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only fault-decode   or   mcu build bare <chip> fault-decode && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(fault-decode C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(fault-decode src/main.c)
bare_firmware(fault-decode)
