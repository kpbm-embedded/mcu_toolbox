cmake_minimum_required(VERSION 3.20)
# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project(ch32v003-blink_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion

# Firmware: built by the SDK's own make (CLion Build = target "firmware").
# Code insight: target "firmware-index" holds the SDK's real compile flags (never built).
project(ch32v003-blink C)
if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../tool" ABSOLUTE)
endif()
include("${MCU_ROOT}/clion/cmake/McuProject.cmake")
set(MCU_FW_MAKE_ARGS "" CACHE STRING "extra make arguments (set per CMake profile)")
set(FW "${CMAKE_BINARY_DIR}/fw")
mcu_firmware(
  OUTPUT_DIR "${FW}"
  BUILD "${MCU_MAKE}" -C "${FW}" -f "${CMAKE_CURRENT_SOURCE_DIR}/app/Makefile" TARGET_MCU=CH32V003 TARGET_MCU_PACKAGE=CH32V003 ${MCU_FW_MAKE_ARGS} build -j${MCU_JOBS}
  ARTIFACTS "${FW}/blink.elf" "${FW}/blink.bin" "${FW}/blink.hex"
  INDEX_DRY_RUN "${MCU_MAKE}" -C "${FW}" -f "${CMAKE_CURRENT_SOURCE_DIR}/app/Makefile" TARGET_MCU=CH32V003 TARGET_MCU_PACKAGE=CH32V003 ${MCU_FW_MAKE_ARGS} -n -B -w build
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/Makefile")
