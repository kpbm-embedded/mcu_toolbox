# Bare-metal firmware for one CHIP (after project(); bare_chip.cmake was included before project()).
#   bare_firmware(<tgt> [LD <linker script>] [NO_COMMON] [SPECS nano|none] [OPT -Os])
# Adds the chip's startup and console (chips/<chip>/), the common test helpers (chips/common/bare.c, unless
# NO_COMMON), flags and the linker script; makes <tgt>.elf/.bin/.hex and <tgt>.map and prints the size.
# The run method of the chip (qemu/renode/none) is written to <build>/<tgt>.run for run/firmware.sh.
include_guard(GLOBAL)
function(bare_firmware tgt)
  cmake_parse_arguments(A "NO_COMMON" "LD;SPECS;OPT" "" ${ARGN})
  if(NOT A_LD)
    set(A_LD ${CHIP_LD})
  endif()
  if(NOT A_OPT)
    set(A_OPT -Os)
  endif()
  set(specs --specs=nano.specs)
  if(A_SPECS STREQUAL "none")
    set(specs -nostdlib)
  endif()
  if(CHIP_ARCH STREQUAL "riscv")
    list(APPEND specs -nostartfiles)   # rv_start.S is the entry; newlib's crt0 is not used
  endif()
  target_sources(${tgt} PRIVATE ${CHIP_SOURCES})
  if(NOT A_NO_COMMON)
    target_sources(${tgt} PRIVATE ${MCU_BARE_DIR}/chips/common/bare.c)
  endif()
  target_include_directories(${tgt} PRIVATE ${MCU_BARE_DIR}/chips/common ${MCU_BARE_DIR}/chips/${CHIP})
  target_compile_definitions(${tgt} PRIVATE ${CHIP_DEFINES} BARE_CHIP="${CHIP}")
  target_compile_options(${tgt} PRIVATE ${CHIP_FLAGS} ${A_OPT} -g3 -ffunction-sections -fdata-sections -Wall -Wextra)
  target_link_options(${tgt} PRIVATE ${CHIP_FLAGS} -L${MCU_BARE_DIR}/chips/common -T${A_LD} ${specs} -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${tgt}.map -Wl,--no-warn-rwx-segments -Wl,--print-memory-usage)
  set_target_properties(${tgt} PROPERTIES SUFFIX .elf)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${tgt}.run "${CHIP_RUN}\n")
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${tgt}> ${tgt}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${tgt}> ${tgt}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${tgt}>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endfunction()
