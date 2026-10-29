cmake_minimum_required(VERSION 3.20)
# Two images in one ELF: the app (linked at 0x10100, app.ld) gets a 256-byte header (magic, size, CRC-32;
# mkimage.py) and is embedded at 0x10000 into the bootloader (boot.ld, .incbin). Run: firmware.sh {{NAME}}.elf
set(CHIP "{{TARGET}}" CACHE STRING "chip: mps2-an385")
include($ENV{MCU_BARE}/cmake/bare_chip.cmake)
project({{NAME}} C ASM)
include($ENV{MCU_BARE}/cmake/bare.cmake)
find_program(PYTHON3 python3 REQUIRED)

add_executable(app src/app.c)
bare_firmware(app LD ${CMAKE_CURRENT_SOURCE_DIR}/app.ld)

add_custom_command(OUTPUT app.img
  COMMAND ${PYTHON3} -I ${CMAKE_CURRENT_SOURCE_DIR}/mkimage.py app.bin app.img
  DEPENDS app ${CMAKE_CURRENT_SOURCE_DIR}/mkimage.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(app_image DEPENDS app.img)

add_executable({{NAME}} src/boot.c src/app_blob.S)
add_dependencies({{NAME}} app_image)
set_source_files_properties(src/app_blob.S PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/app.img)
target_include_directories({{NAME}} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
bare_firmware({{NAME}} LD ${CMAKE_CURRENT_SOURCE_DIR}/boot.ld)
