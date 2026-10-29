# ch32v003-blink: ch32fun example 'blink' built with ch32fun's own ch32fun.mk, out of the SDK tree.
# Every output (elf/bin/hex/map/lst, generated linker script) goes to the current directory:
#   make -C <outdir> -f <project>/app/Makefile build        (CLion: <build dir>/fw)
# Makefile.example is the untouched original from wch/ch32fun/examples/blink.
APP_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
MCU_ROOT ?= $(abspath $(APP_DIR)/../../../tool)
CH32FUN_REPO ?= $(MCU_ROOT)/wch/ch32fun
TARGET := blink
TARGET_EXT := c
TARGET_MCU ?= CH32V003
# ch32fun.mk regenerates its linker script on every build; keep it here instead of in the SDK
override GENERATED_LD_FILE := $(CURDIR)/generated_$(TARGET_MCU).ld
vpath %.c $(APP_DIR)
vpath %.cpp $(APP_DIR)
include $(CH32FUN_REPO)/ch32fun/ch32fun.mk
FILES_TO_COMPILE := $(SYSTEM_C) $(APP_DIR)/$(TARGET).$(TARGET_EXT) $(ADDITIONAL_C_FILES)
CFLAGS += -I$(APP_DIR)
clean : cv_clean
