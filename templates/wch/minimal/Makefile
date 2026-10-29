# ch32fun project (out of tree). Build in a COPY of this directory so products do not land in the source tree:
#   source mcu/wch/env.sh
#   cp -r <this dir> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/mywork && make -C $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/mywork TARGET_NAME=ch32v203
# TARGET_NAME: ch32v003 | ch32v203 | ch32v307 | ch32h417
TARGET_NAME ?= ch32v003
CH32FUN     ?= $(CH32FUN_REPO)/ch32fun

ifeq ($(TARGET_NAME),ch32v003)
  TARGET_MCU := CH32V003
  TARGET_MCU_PACKAGE := CH32V003
else ifeq ($(TARGET_NAME),ch32v203)
  TARGET_MCU := CH32V203
  TARGET_MCU_PACKAGE := CH32V203C8T6
else ifeq ($(TARGET_NAME),ch32v307)
  TARGET_MCU := CH32V307
  TARGET_MCU_PACKAGE := CH32V307VCT6
else ifeq ($(TARGET_NAME),ch32h417)
  TARGET_MCU := CH32H417
else
  $(error unknown TARGET_NAME $(TARGET_NAME))
endif

# per-target sources: <target>/main.c if present (used by usb-cdc), else ./main.c
TARGET := $(if $(wildcard $(TARGET_NAME)/main.c),$(TARGET_NAME)/main,main)
EXTRA_CFLAGS += -DBOARD_$(shell echo $(TARGET_NAME) | tr a-z A-Z) $(if $(wildcard $(TARGET_NAME)),-I$(TARGET_NAME))
EXTRA_CFLAGS += -I$(CH32FUN)/../extralibs
# keep the generated linker script in the build dir, not in the SDK checkout
LINKER_SCRIPT := $(CURDIR)/link.ld
-include example.mk   # optional per-example additions (extra sources, flags)

all: $(TARGET).bin
include $(CH32FUN)/ch32fun.mk
$(LINKER_SCRIPT):
	$(PREFIX)-gcc -E -P -x c -DTARGET_MCU=$(TARGET_MCU) -DMCU_PACKAGE=$(MCU_PACKAGE) -DTARGET_MCU_LD=$(TARGET_MCU_LD) -DTARGET_MCU_MEMORY_SPLIT=$(TARGET_MCU_MEMORY_SPLIT) $(CH32FUN)/ch32fun.ld > $@
flash: cv_flash
clean: cv_clean
