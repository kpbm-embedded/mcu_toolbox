# Official WCH EVT GPIO_Toggle for CH32V203 (C8T6 = D6 variant) built with xPack riscv-none-elf-gcc.
EVT   ?= $(abspath ../ch32v20x/EVT/EXAM)
APP   := GPIO/GPIO_Toggle/User
CROSS ?= riscv-none-elf-
BUILD ?= build
# Flag tweaks vs MounRiver: -march=rv32imac_zicsr_zifencei (newer GCC splits csr/fence.i out of base ISA)
ARCH  := -march=rv32imac_zicsr_zifencei -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore
CFLAGS:= $(ARCH) -O2 -g -ffunction-sections -fdata-sections -fsigned-char -Wall \
  -I$(EVT)/SRC/Core -I$(EVT)/SRC/Debug -I$(EVT)/SRC/Peripheral/inc -I$(EVT)/$(APP)
ASFLAGS:= $(ARCH) -x assembler-with-cpp
LDFLAGS:= $(ARCH) -T $(EVT)/SRC/Ld/Link.ld -nostartfiles -Wl,--gc-sections -Wl,-Map=$(BUILD)/fw.map --specs=nano.specs --specs=nosys.specs
SRCS := $(wildcard $(EVT)/$(APP)/*.c) $(EVT)/SRC/Core/core_riscv.c $(EVT)/SRC/Debug/debug.c $(wildcard $(EVT)/SRC/Peripheral/src/*.c)
OBJS := $(patsubst %.c,$(BUILD)/%.o,$(notdir $(SRCS))) $(BUILD)/startup.o
VPATH:= $(dir $(SRCS))
all: $(BUILD)/GPIO_Toggle.elf $(BUILD)/GPIO_Toggle.bin $(BUILD)/GPIO_Toggle.hex
	$(CROSS)size $<
$(BUILD):
	mkdir -p $@
$(BUILD)/%.o: %.c | $(BUILD)
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
$(BUILD)/startup.o: $(EVT)/SRC/Startup/startup_ch32v20x_D6.S | $(BUILD)
	$(CROSS)gcc $(ASFLAGS) -c $< -o $@
$(BUILD)/%.elf: $(OBJS)
	$(CROSS)gcc $(LDFLAGS) $^ -o $@
$(BUILD)/%.bin: $(BUILD)/%.elf
	$(CROSS)objcopy -O binary $< $@
$(BUILD)/%.hex: $(BUILD)/%.elf
	$(CROSS)objcopy -O ihex $< $@
