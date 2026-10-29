cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only ringbuf-split   or   mcu build bare <chip> ringbuf-split && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385 virt-rv32")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(ringbuf-split C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(ringbuf-split src/main.c lib/ringbuf.c)
target_include_directories(ringbuf-split PRIVATE lib)
bare_firmware(ringbuf-split)
