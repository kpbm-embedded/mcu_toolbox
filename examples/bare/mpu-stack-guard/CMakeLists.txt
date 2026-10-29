cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only mpu-stack-guard   or   mcu build bare <chip> mpu-stack-guard && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(mpu-stack-guard C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(mpu-stack-guard src/main.c)
bare_firmware(mpu-stack-guard)
