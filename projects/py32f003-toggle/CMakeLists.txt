cmake_minimum_required(VERSION 3.20)
# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project(py32f003-toggle_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion

# Firmware: built by the SDK's own make (CLion Build = target "firmware").
# Code insight: target "firmware-index" holds the SDK's real compile flags (never built).
project(py32f003-toggle C)
if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../tool" ABSOLUTE)
endif()
include("${MCU_ROOT}/clion/cmake/McuProject.cmake")
set(MCU_FW_MAKE_ARGS "" CACHE STRING "extra make arguments (set per CMake profile)")
set(FW "${CMAKE_BINARY_DIR}/fw")
# py32f0-template's Makefile uses paths relative to its own directory, so it runs in a stage dir
# inside the build dir: symlinks to every template entry plus app -> this project's app/.
# Objects and app.elf/bin/hex go to BDIR = <build dir>/fw; the template tree is only read.
set(PY32_TEMPLATE "${MCU_ROOT}/py32/py32f0-template")
set(PY32_STAGE "${CMAKE_BINARY_DIR}/py32-stage")
file(MAKE_DIRECTORY "${PY32_STAGE}")
file(GLOB _tpl LIST_DIRECTORIES true RELATIVE "${PY32_TEMPLATE}" "${PY32_TEMPLATE}/*")
foreach(_e IN LISTS _tpl)
  if(NOT _e MATCHES "^(Build|app|[.].*)$")
    file(CREATE_LINK "${PY32_TEMPLATE}/${_e}" "${PY32_STAGE}/${_e}" SYMBOLIC)
  endif()
endforeach()
file(CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/app" "${PY32_STAGE}/app" SYMBOLIC)
mcu_firmware(
  OUTPUT_DIR "${FW}"
  BUILD "${MCU_MAKE}" -C "${PY32_STAGE}" MCU_TYPE=PY32F003x8 USE_LL_LIB=y APP_DIR=app "BDIR=${FW}" "ARM_TOOLCHAIN=${MCU_ROOT}/tools/toolchains/arm-gnu-toolchain/bin" IS_GCC_ABOVE_12=1 ${MCU_FW_MAKE_ARGS} -j${MCU_JOBS}
  ARTIFACTS "${FW}/app.elf" "${FW}/app.bin" "${FW}/app.hex"
  INDEX_DRY_RUN "${MCU_MAKE}" -C "${PY32_STAGE}" MCU_TYPE=PY32F003x8 USE_LL_LIB=y APP_DIR=app "BDIR=${FW}" "ARM_TOOLCHAIN=${MCU_ROOT}/tools/toolchains/arm-gnu-toolchain/bin" IS_GCC_ABOVE_12=1 ${MCU_FW_MAKE_ARGS} -n -B -w
  INDEX_ARGS --exclude "\\.s$")
