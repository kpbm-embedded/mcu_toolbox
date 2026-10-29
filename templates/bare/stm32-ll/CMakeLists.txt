cmake_minimum_required(VERSION 3.20)
# {{NAME}}: bare-metal firmware (mcu bare layer). Inside `mcu shell bare`:
#   cmake -S . -B build -G Ninja -DCHIP=<chip> && cmake --build build && $MCU_BARE/run/firmware.sh build/{{NAME}}.elf
set(CHIP "{{TARGET}}" CACHE STRING "chip: stm32f411 stm32f103 stm32h743")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project({{NAME}} C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
add_executable({{NAME}} src/main.c)
# Cube LL drivers (no HAL): pack, device and LL sources per chip family
if(CHIP STREQUAL "stm32f103")
  set(F F1)
  set(f f1)
  set(DEV STM32F103xB)
elseif(CHIP STREQUAL "stm32h743")
  set(F H7)
  set(f h7)
  set(DEV STM32H743xx)
else()
  set(F F4)
  set(f f4)
  set(DEV STM32F411xE)
endif()
set(CUBE ${MCU_BARE_DIR}/../stm32/STM32Cube${F}/Drivers)
target_sources({{NAME}} PRIVATE ${CUBE}/STM32${F}xx_HAL_Driver/Src/stm32${f}xx_ll_gpio.c
  ${CUBE}/STM32${F}xx_HAL_Driver/Src/stm32${f}xx_ll_usart.c ${CUBE}/STM32${F}xx_HAL_Driver/Src/stm32${f}xx_ll_rcc.c
  ${CUBE}/CMSIS/Device/ST/STM32${F}xx/Source/Templates/system_stm32${f}xx.c)   # SystemCoreClock + prescaler tables (SystemInit is not called)
target_include_directories({{NAME}} PRIVATE ${CUBE}/STM32${F}xx_HAL_Driver/Inc ${CUBE}/CMSIS/Device/ST/STM32${F}xx/Include
  ${CUBE}/CMSIS/Include ${CUBE}/CMSIS/Core/Include)
target_compile_definitions({{NAME}} PRIVATE ${DEV} USE_FULL_LL_DRIVER LL_FAMILY_H="stm32${f}xx_ll_gpio.h")
bare_firmware({{NAME}})
