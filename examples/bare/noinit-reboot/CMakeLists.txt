cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only noinit-reboot   or   mcu build bare <chip> noinit-reboot && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(noinit-reboot C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(noinit-reboot src/main.c)
bare_firmware(noinit-reboot)
