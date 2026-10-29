cmake_minimum_required(VERSION 3.20)
# {{NAME}}: everything below the C library is in this project: startup.c, link.ld. Only the shared toolchain file
# ($MCU_BARE/cmake/arm-none-eabi.cmake) comes from mcu. Target: qemu mps2-an385 (Cortex-M3).
set(CMAKE_TOOLCHAIN_FILE $ENV{MCU_BARE}/cmake/arm-none-eabi.cmake)
project({{NAME}} C)
set(CPU -mcpu=cortex-m3 -mthumb)
add_executable({{NAME}} src/startup.c src/main.c)
target_compile_options({{NAME}} PRIVATE ${CPU} -Os -g -ffunction-sections -fdata-sections -Wall -Wextra)
target_link_options({{NAME}} PRIVATE ${CPU} -T${CMAKE_CURRENT_SOURCE_DIR}/link.ld --specs=nano.specs --specs=nosys.specs
  -Wl,--gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/{{NAME}}.map -Wl,--no-warn-rwx-segments)
set_target_properties({{NAME}} PROPERTIES SUFFIX .elf)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/{{NAME}}.run "qemu-arm mps2-an385\n")   # for $MCU_BARE/run/firmware.sh
add_custom_command(TARGET {{NAME}} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:{{NAME}}> {{NAME}}.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:{{NAME}}> WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
