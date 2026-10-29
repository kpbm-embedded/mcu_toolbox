cmake_minimum_required(VERSION 3.20)
# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project(k230-hello_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion

# Firmware: built by the SDK's own make (CLion Build = target "firmware").
# Code insight: target "firmware-index" holds the SDK's real compile flags (never built).
project(k230-hello C CXX)
if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../tool" ABSOLUTE)
endif()
include("${MCU_ROOT}/clion/cmake/McuProject.cmake")
set(MCU_FW_MAKE_ARGS "" CACHE STRING "extra make arguments (set per CMake profile)")
set(FW "${CMAKE_BINARY_DIR}/fw")
# RT-Smart user app, built with the K230 SDK's own .mk files (tools/mkenv.mk, .config, libs/mk/*.mk).
# The SDK tree is only read; build products go to <build dir>/fw.
mcu_firmware(
  OUTPUT_DIR "${FW}"
  BUILD "${MCU_MAKE}" -C "${CMAKE_CURRENT_SOURCE_DIR}/app" "OUT=${FW}" ${MCU_FW_MAKE_ARGS} all
  ARTIFACTS "${FW}/k230-hello"
  INDEX_DRY_RUN "${MCU_MAKE}" -C "${CMAKE_CURRENT_SOURCE_DIR}/app" "OUT=${FW}" ${MCU_FW_MAKE_ARGS} -n -B -w all
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/Makefile")
