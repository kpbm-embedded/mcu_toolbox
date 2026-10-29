cmake_minimum_required(VERSION 3.20)
# Needs: . mcu/stm32/env.sh   (Arm GNU toolchain, cmake, ninja, STM32_CUBE_* paths)
if(NOT DEFINED ENV{STM32_CUBE_F4})
  message(FATAL_ERROR "source mcu/stm32/env.sh first")
endif()
set(STM32_BOARDS "$ENV{STM32_CUBE_F4}/../boards" CACHE PATH "mcu/stm32/boards (shared board layer)")
set(CMAKE_TOOLCHAIN_FILE ${STM32_BOARDS}/arm-none-eabi.cmake)
set(BOARD "{{TARGET}}" CACHE STRING "blackpill-f411 | weact-g431 | mini-h743 | bluepill-f103")
project({{NAME}} C ASM)
include(${STM32_BOARDS}/board.cmake)
add_executable({{NAME}} src/main.c src/logic.c)
target_include_directories({{NAME}} PRIVATE src)
stm32_board({{NAME}})

# Host unit tests (plain host CMake project in tests/, host gcc from mcu/host): cmake --build <dir> --target host-tests
add_custom_target(host-tests
  COMMAND ${CMAKE_COMMAND} -G Ninja -DCMAKE_TOOLCHAIN_FILE= -S ${CMAKE_SOURCE_DIR}/tests -B ${CMAKE_BINARY_DIR}/host-tests
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/host-tests
  COMMAND ctest --test-dir ${CMAKE_BINARY_DIR}/host-tests --output-on-failure
  USES_TERMINAL)
