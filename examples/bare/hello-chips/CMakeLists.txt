cmake_minimum_required(VERSION 3.20)
# Build: mcu build bare <chip> hello-chips   (or check.sh bare). Needs bare/env.sh (MCU_BARE).
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385 stm32f411 stm32h743 stm32f103 nrf52840 virt-rv32 ch32v003")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(hello-chips C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(hello-chips src/main.c)
bare_firmware(hello-chips)
