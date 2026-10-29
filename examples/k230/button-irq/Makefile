# RT-Smart user-space app built out of tree against the K230 RTOS SDK (read only).
#   source mcu/k230/env.sh
#   make K230_SDK=$K230_SDK OUT=$PWD/build        # after `./mcu build k230 <target>` (the SDK's own output must exist)
# The SDK keeps ONE global .config; select the board first:  make -C $K230_SDK k230d_rtos_bpi_zero_defconfig
#   (or k230_rtos_01studio_defconfig) && make -C $K230_SDK .autoconf. mcu/templates/tools/check.sh does this under the SDK flock.
# Run the result: copy app.elf to the SD card (/sdcard/app/) of a *_rtos_* image and run it from the RT-Smart msh.
K230_SDK ?= $(K230_HOME)/sdk
SDK_REAL := $(realpath $(K230_SDK))
export SDK_SRC_ROOT_DIR := $(SDK_REAL)
include $(SDK_REAL)/src/applications/mkenv.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/toolchain_riscv64_musl.mk

OUT  ?= build
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,$(OUT)/%.o,$(SRCS))

include $(SDK_RTSMART_SRC_DIR)/libs/mk/librtsmart_hal.mk

C_CXX_FLAGS := -fopenmp -march=rv64imafdcv -mabi=lp64d -mcmodel=medany -Os
C_CXX_FLAGS += -I. $(LIB_CFLAGS) -I$(SDK_SRC_ROOT_DIR)/include -include "generated/autoconf.h"
C_CXX_FLAGS += -Wall -Wextra -Wno-sign-compare -ffunction-sections -fdata-sections
CFLAGS  := $(C_CXX_FLAGS) -std=gnu99 $(EXTRA_CFLAGS)
LDFLAGS := -T $(SDK_RTSMART_SRC_DIR)/libs/mk/link.lds --static -Wl,--gc-sections $(LIB_LDFLAGS)

all: $(OUT)/app.elf
$(OUT)/app.elf: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@
$(OUT)/%.o: %.c
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) -MD -MP -c $< -o $@
-include $(OBJS:.o=.d)
.PHONY: all
