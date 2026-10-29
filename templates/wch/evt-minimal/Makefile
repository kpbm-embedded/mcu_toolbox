# WCH EVT-style build with xPack riscv-none-elf-gcc. The EVT library (openwch/ch32v20x) is read from $(WCH_DIR) and not modified.
#   source mcu/wch/env.sh; make BUILD=$PWD/build            (check.sh passes TARGET_NAME=ch32v203, ignored here)
EVT   ?= $(WCH_DIR)/ch32v20x/EVT/EXAM
CROSS ?= riscv-none-elf-
BUILD ?= build
PROJECT ?= app
ARCH  := -march=rv32imac_zicsr_zifencei -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore
CFLAGS:= $(ARCH) -O2 -g -ffunction-sections -fdata-sections -fsigned-char -Wall \
  -I. -I$(EVT)/SRC/Core -I$(EVT)/SRC/Debug -I$(EVT)/SRC/Peripheral/inc
LDFLAGS:= $(ARCH) -T $(EVT)/SRC/Ld/Link.ld -nostartfiles -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(PROJECT).map --specs=nano.specs --specs=nosys.specs
SRCS  := $(wildcard *.c) $(EVT)/SRC/Core/core_riscv.c $(EVT)/SRC/Debug/debug.c $(wildcard $(EVT)/SRC/Peripheral/src/*.c)
OBJS  := $(patsubst %.c,$(BUILD)/%.o,$(notdir $(SRCS))) $(BUILD)/startup.o
VPATH := $(dir $(SRCS))
all: $(BUILD)/$(PROJECT).elf $(BUILD)/$(PROJECT).bin $(BUILD)/$(PROJECT).hex
	$(CROSS)size $<
$(BUILD):
	mkdir -p $@
$(BUILD)/%.o: %.c | $(BUILD)
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
$(BUILD)/startup.o: $(EVT)/SRC/Startup/startup_ch32v20x_D6.S | $(BUILD)
	$(CROSS)gcc $(ARCH) -x assembler-with-cpp -c $< -o $@
$(BUILD)/$(PROJECT).elf: $(OBJS)
	$(CROSS)gcc $(LDFLAGS) $^ -o $@
$(BUILD)/%.bin: $(BUILD)/%.elf
	$(CROSS)objcopy -O binary $< $@
$(BUILD)/%.hex: $(BUILD)/%.elf
	$(CROSS)objcopy -O ihex $< $@
.PHONY: all
