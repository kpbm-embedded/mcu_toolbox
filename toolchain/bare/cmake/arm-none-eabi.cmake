# Shared CMake toolchain file: Arm GNU toolchain (arm-none-eabi), bare metal. Any mcu project may use it:
#   cmake -DCMAKE_TOOLCHAIN_FILE=$MCU_BARE/cmake/arm-none-eabi.cmake ...   (source bare/env.sh first, or put the
#   Arm toolchain on PATH). Per-chip flags are added by the project (bare.cmake: bare_firmware()).
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_NM arm-none-eabi-nm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
