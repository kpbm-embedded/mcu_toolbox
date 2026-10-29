cmake_minimum_required(VERSION 3.20)
# Firmware index math (C) checked against a host-side Bend 2 reference (test.sh, pinned Bend 2: mcu setup install bend).
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(bend-host-check C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(bend-host-check src/main.c)
bare_firmware(bend-host-check)
