# Legacy SDK (SDK 2.2.0, branch master_legacy) for BL808; this builds the M0 (32-bit RISC-V) core image.
# Direct use:  . {{MCU_ROOT}}/bouffalo/env.sh && make
SDK_DEMO_PATH ?= $(abspath .)
BL_SDK_BASE ?= {{MCU_ROOT}}/bouffalo/bouffalo_sdk_legacy
export BL_SDK_BASE

CHIP ?= bl808
BOARD ?= bl808dk
CPU_ID ?= m0
CROSS_COMPILE ?= riscv64-unknown-elf-

include $(BL_SDK_BASE)/project.build
