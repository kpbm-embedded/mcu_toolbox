cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only timer-irq-stm32   or   mcu build bare <chip> timer-irq-stm32 && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "stm32f411" CACHE STRING "chip: stm32f411 stm32f103")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(timer-irq-stm32 C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(timer-irq-stm32 src/main.c)
bare_firmware(timer-irq-stm32)
