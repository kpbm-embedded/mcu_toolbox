cmake_minimum_required(VERSION 3.5)
# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project(k210-hello_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion

# Firmware: kendryte-standalone-sdk's own CMake (common.cmake, lib/, executable.cmake), used out of tree.
# SDK_ROOT and TOOLCHAIN come from the CMake profile.
set(PROJ k210-hello)
set(BUILDING_SDK "yes" CACHE INTERNAL "")
include(${SDK_ROOT}/cmake/common.cmake)
project(${PROJ} C CXX ASM)
include(${SDK_ROOT}/cmake/macros.internal.cmake)
header_directories(${SDK_ROOT}/lib)
header_directories(${CMAKE_CURRENT_LIST_DIR}/src)
add_subdirectory(${SDK_ROOT}/lib kendryte)
add_source_files(src/*.c src/*.s src/*.S src/*.cpp)
include(${SDK_ROOT}/cmake/executable.cmake)
