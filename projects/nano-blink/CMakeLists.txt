cmake_minimum_required(VERSION 3.20)
# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project(nano-blink_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion

# Firmware: built by the SDK's own make (CLion Build = target "firmware").
# Code insight: target "firmware-index" holds the SDK's real compile flags (never built).
project(nano-blink C CXX)
if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../tool" ABSOLUTE)
endif()
include("${MCU_ROOT}/clion/cmake/McuProject.cmake")
set(MCU_FW_MAKE_ARGS "" CACHE STRING "extra make arguments (set per CMake profile)")
set(FW "${CMAKE_BINARY_DIR}/fw")
# arduino-cli compiles the sketch; --only-compilation-database gives the index flags at configure time.
mcu_firmware(
  OUTPUT_DIR "${FW}"
  BUILD "${MCU_ROOT}/avr/bin/arduino-cli" compile -b arduino:avr:nano:cpu=atmega328 --build-path "${CMAKE_BINARY_DIR}/arduino-build" "${CMAKE_CURRENT_SOURCE_DIR}/Blink" -j ${MCU_JOBS} --output-dir "${FW}"
  ARTIFACTS "${FW}/Blink.ino.hex" "${FW}/Blink.ino.elf"
  INDEX_PREPARE "${MCU_ROOT}/avr/bin/arduino-cli" compile -b arduino:avr:nano:cpu=atmega328 --build-path "${CMAKE_BINARY_DIR}/arduino-build" "${CMAKE_CURRENT_SOURCE_DIR}/Blink" --only-compilation-database
  INDEX_COMPDB "${CMAKE_BINARY_DIR}/arduino-build/compile_commands.json"
  INDEX_ARGS --map "Blink\\.ino\\.cpp$=${CMAKE_CURRENT_SOURCE_DIR}/Blink/Blink.ino"
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Blink/Blink.ino")
