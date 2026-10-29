cmake_minimum_required(VERSION 3.20)
# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project(stm32-blinky-f411_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion

# Firmware: STM32F411CEU6 + STM32CubeF4 HAL from mcu/stm32 (CUBE comes from the CMake profile).
if(NOT CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi.cmake)
endif()
project(stm32-blinky-f411 C ASM)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/stm32f411.cmake)
add_executable(stm32-blinky-f411 src/main.c)
target_link_libraries(stm32-blinky-f411 stm32f411_hal)
stm32f411_finish(stm32-blinky-f411)
