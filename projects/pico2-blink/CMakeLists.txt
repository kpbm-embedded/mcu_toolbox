cmake_minimum_required(VERSION 3.13)
# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project(pico2-blink_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion

# Firmware: Pico SDK (PICO_SDK_PATH, PICO_BOARD, PICO_PLATFORM come from the CMake profile).
include(pico_sdk_import.cmake)
project(pico2-blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
function(example_auto_set_url target)
endfunction()
add_subdirectory(app)   # copy of pico-examples/blink (target blink)
