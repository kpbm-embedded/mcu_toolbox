cmake_minimum_required(VERSION 3.20)
# Build + run: check.sh bare --only cxx-bare   or   mcu build bare <chip> cxx-bare && $MCU_BARE/run/firmware.sh <elf>
set(CHIP "mps2-an385" CACHE STRING "chip: mps2-an385 virt-rv32 stm32f411")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project(cxx-bare C CXX ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable(cxx-bare src/main.cpp)
target_compile_options(cxx-bare PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-threadsafe-statics>)
bare_firmware(cxx-bare)
