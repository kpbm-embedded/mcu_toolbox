cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only systick-irq   or   mcu build bare <chip> systick-irq && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385 stm32f411 nrf52840 stm32f103")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(systick-irq C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(systick-irq src/main.c)
bare_firmware(systick-irq)
