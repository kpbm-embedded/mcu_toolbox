# Out-of-tree build for Puya PY32F0xx / PY32F07x using CMSIS, drivers and linker scripts of IOsetting/py32f0-template
# (read only; PY32 = that checkout, `source mcu/py32/env.sh` exports PY32_TEMPLATE).
#   make TARGET=py32f003x8|py32f002ax5|py32f030x8|py32f071x8 [BUILD=build] [PY32=/path/to/py32f0-template]
# F002A/F003/F030 use the LL driver; F071 only has a HAL driver in the template (PY32F07x_HAL_*).
# An optional project.mk next to this file may set RTOS := y (FreeRTOS from the template's Libraries/FreeRTOS).
-include project.mk
PROJECT ?= app
TARGET  ?= py32f003x8
BUILD   ?= build
PY32    ?= $(PY32_TEMPLATE)
CROSS   ?= arm-none-eabi-
RTOS    ?= n

LIB   := $(PY32)/Libraries
FAM   := f0xx
ifeq ($(TARGET),py32f003x8)
  DEVDEF := PY32F003x8
  STARTUP := startup_py32f003.s
else ifeq ($(TARGET),py32f002ax5)
  DEVDEF := PY32F002Ax5
  STARTUP := startup_py32f002a.s
else ifeq ($(TARGET),py32f030x8)
  DEVDEF := PY32F030x8
  STARTUP := startup_py32f030.s
else ifeq ($(TARGET),py32f071x8)
  DEVDEF := PY32F071x8
  STARTUP := startup_py32f072.s
  FAM := f07x
else
  $(error unknown TARGET $(TARGET))
endif

ifeq ($(FAM),f07x)
  # 64 KB flash / 8 KB SRAM (PY32F071x8, datasheet table 1-1); the template only ships py32f072xb.ld (128K/16K),
  # so a copy with the x8 sizes is generated in $(BUILD).
  DRV   := $(LIB)/PY32F07x_HAL_Driver $(LIB)/PY32F07x_HAL_BSP
  SYSC  := $(LIB)/CMSIS/Device/PY32F0xx/Source/system_py32f07x.c
  DRVDEF:= -DUSE_HAL_DRIVER
  LDS   := $(BUILD)/$(TARGET).ld
else
  DRV   := $(LIB)/PY32F0xx_LL_Driver $(LIB)/PY32F0xx_LL_BSP
  SYSC  := $(LIB)/CMSIS/Device/PY32F0xx/Source/system_py32f0xx.c
  DRVDEF:= -DUSE_FULL_LL_DRIVER
  LDS   := $(LIB)/LDScripts/$(TARGET).ld
endif

SRCS  := $(wildcard *.c) $(foreach d,$(DRV),$(wildcard $(d)/Src/*.c)) $(SYSC)
INC   := -I. -I$(LIB)/CMSIS/Core/Include -I$(LIB)/CMSIS/Device/PY32F0xx/Include $(foreach d,$(DRV),-I$(d)/Inc)
ifeq ($(RTOS),y)
  RT  := $(LIB)/FreeRTOS
  SRCS += $(wildcard $(RT)/*.c) $(RT)/portable/GCC/ARM_CM0/port.c $(RT)/portable/MemMang/heap_4.c
  INC  += -I$(RT)/include -I$(RT)/portable/GCC/ARM_CM0
endif
ASRC  := $(LIB)/CMSIS/Device/PY32F0xx/Source/gcc/$(STARTUP)
ARCH  := -mcpu=cortex-m0plus -mthumb
CFLAGS:= $(ARCH) -Os -g -std=c17 -D$(DEVDEF) $(DRVDEF) -Wall -ffunction-sections -fdata-sections $(INC)
LDFLAGS:= $(ARCH) -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(PROJECT).map -Wl,--no-warn-rwx-segments \
          -T$(LDS)
OBJS  := $(patsubst %.c,$(BUILD)/%.o,$(notdir $(SRCS))) $(BUILD)/startup.o
VPATH := $(sort $(dir $(SRCS)))

all: $(BUILD)/$(PROJECT).elf $(BUILD)/$(PROJECT).bin $(BUILD)/$(PROJECT).hex
	$(CROSS)size $<
$(BUILD):
	mkdir -p $@
$(BUILD)/py32f071x8.ld: $(LIB)/LDScripts/py32f072xb.ld | $(BUILD)
	sed -e 's/LENGTH = 16K/LENGTH = 8K/' -e 's/LENGTH = 128K/LENGTH = 64K/' $< > $@
$(BUILD)/%.o: %.c | $(BUILD)
	$(CROSS)gcc $(CFLAGS) -MMD -c $< -o $@
$(BUILD)/startup.o: $(ASRC) | $(BUILD)
	$(CROSS)gcc $(ARCH) -c $< -o $@
$(BUILD)/$(PROJECT).elf: $(OBJS) $(LDS)
	$(CROSS)gcc $(OBJS) $(LDFLAGS) -lc -lm -o $@
$(BUILD)/%.bin: $(BUILD)/%.elf
	$(CROSS)objcopy -O binary $< $@
$(BUILD)/%.hex: $(BUILD)/%.elf
	$(CROSS)objcopy -O ihex $< $@
-include $(wildcard $(BUILD)/*.d)
.PHONY: all
