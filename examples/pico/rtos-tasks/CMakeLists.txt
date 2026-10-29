cmake_minimum_required(VERSION 3.13)
# Out-of-tree Pico SDK + FreeRTOS SMP app: PICO_SDK_PATH comes from mcu/pico/env.sh.
# Select the board with -DPICO_BOARD=... (and -DPICO_PLATFORM=rp2350-riscv for RISC-V).
# FreeRTOS-Kernel (with the RP2040 / RP2350_ARM_NTZ / RP2350_RISC-V ports) is taken from, in order:
# -DFREERTOS_KERNEL_PATH, $FREERTOS_KERNEL_PATH, $MCU_ROOT/freertos/FreeRTOS-Kernel, ../../../tool/freertos/FreeRTOS-Kernel.
include(pico_sdk_import.cmake)
if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    if (DEFINED ENV{MCU_ROOT} AND EXISTS "$ENV{MCU_ROOT}/freertos/FreeRTOS-Kernel/tasks.c")
        set(FREERTOS_KERNEL_PATH "$ENV{MCU_ROOT}/freertos/FreeRTOS-Kernel")
    else()
        get_filename_component(_here "${CMAKE_CURRENT_LIST_DIR}" REALPATH)
        set(FREERTOS_KERNEL_PATH "${_here}/../../../tool/freertos/FreeRTOS-Kernel")
    endif()
endif()
project(rtos-tasks C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()
include(FreeRTOS_Kernel_import.cmake)   # copy of pico-examples/freertos/FreeRTOS_Kernel_import.cmake
add_executable(rtos-tasks main.c)
target_include_directories(rtos-tasks PRIVATE ${CMAKE_CURRENT_LIST_DIR})   # FreeRTOSConfig.h
target_link_libraries(rtos-tasks pico_stdlib FreeRTOS-Kernel-Heap4)
pico_enable_stdio_usb(rtos-tasks 0)
pico_enable_stdio_uart(rtos-tasks 1)
pico_add_extra_outputs(rtos-tasks)
