# Direct use:  . {{MCU_ROOT}}/bouffalo/env.sh && make          (CHIP/BOARD default to the target chosen at instantiation)
SDK_DEMO_PATH ?= $(abspath .)
BL_SDK_BASE ?= {{MCU_ROOT}}/bouffalo/bouffalo_sdk
export BL_SDK_BASE

CHIP ?= {{TARGET}}
BOARD ?= {{TARGET}}dk
CROSS_COMPILE ?= riscv64-unknown-elf-

include $(BL_SDK_BASE)/project.build
