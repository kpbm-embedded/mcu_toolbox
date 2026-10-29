cmake_minimum_required(VERSION 3.20)
# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project(bl616-hello_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion

# Firmware: built by the SDK's own make (CLion Build = target "firmware").
# Code insight: target "firmware-index" holds the SDK's real compile flags (never built).
project(bl616-hello C)
if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../tool" ABSOLUTE)
endif()
include("${MCU_ROOT}/clion/cmake/McuProject.cmake")
set(MCU_FW_MAKE_ARGS "" CACHE STRING "extra make arguments (set per CMake profile)")
set(FW "${CMAKE_BINARY_DIR}/fw")
# bouffalo_sdk's project.build (Kconfig seed + inner CMake) runs in app/ with BUILD_DIR = <build dir>/fw.
# Its inner CMake writes compile_commands.json, which feeds the index. The SDK joins SDK_DEMO_PATH and
# BUILD_DIR, so BUILD_DIR is passed relative to app/.
file(RELATIVE_PATH FW_REL "${CMAKE_CURRENT_SOURCE_DIR}/app" "${FW}")
mcu_firmware(
  OUTPUT_DIR "${FW}"
  BUILD "${CMAKE_COMMAND}" -E env CMAKE_EXPORT_COMPILE_COMMANDS=ON "${MCU_MAKE}" -C "${CMAKE_CURRENT_SOURCE_DIR}/app" CHIP=bl616 BOARD=bl616dk "BUILD_DIR=${FW_REL}" ${MCU_FW_MAKE_ARGS}
  ARTIFACTS "${FW}/build_out/helloworld_bl616.bin" "${FW}/build_out/helloworld_bl616.elf"
  INDEX_PREPARE "${CMAKE_COMMAND}" -E env CMAKE_EXPORT_COMPILE_COMMANDS=ON "${MCU_MAKE}" -C "${CMAKE_CURRENT_SOURCE_DIR}/app" CHIP=bl616 BOARD=bl616dk "BUILD_DIR=${FW_REL}" ${MCU_FW_MAKE_ARGS} cmake_cache
  INDEX_COMPDB "${FW}/compile_commands.json"
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/defconfig" "${CMAKE_CURRENT_SOURCE_DIR}/app/CMakeLists.txt")
