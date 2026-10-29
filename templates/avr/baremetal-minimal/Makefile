# Bare-metal AVR build with the avr-gcc from mcu/avr (source mcu/avr/env.sh first).
# make TARGET=nano|nano-old|mega|attiny85   (nano/nano-old = ATmega328P 16 MHz, same code, mega = ATmega2560 16 MHz, attiny85 = 8 MHz internal)
NAME   ?= {{NAME}}
TARGET ?= nano
ifneq (,$(filter $(TARGET),nano nano-old))
  MCU := atmega328p
  F_CPU := 16000000UL
  BOARD := BOARD_NANO
  AVRDUDE_P := m328p
else ifeq ($(TARGET),mega)
  MCU := atmega2560
  F_CPU := 16000000UL
  BOARD := BOARD_MEGA
  AVRDUDE_P := m2560
else ifeq ($(TARGET),attiny85)
  MCU := attiny85
  F_CPU := 8000000UL
  BOARD := BOARD_ATTINY85
  AVRDUDE_P := t85
else
  $(error unknown TARGET $(TARGET))
endif
BOX_OUT ?= $(BOX_V3)
OUT ?= $(if $(BOX_OUT),$(BOX_OUT)/30_39_tools/32_embedded/32_01_mcu/build,build)/examples/avr/$(NAME)/$(TARGET)
SRCS := $(wildcard *.c)
CFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -D$(BOARD) -Os -Wall -Wextra -std=gnu11 -ffunction-sections -fdata-sections
all: $(OUT)/$(NAME).hex
	avr-size -C --mcu=$(MCU) $(OUT)/$(NAME).elf
$(OUT)/$(NAME).elf: $(SRCS) board.h
	mkdir -p $(OUT)
	avr-gcc $(CFLAGS) -Wl,--gc-sections -o $@ $(SRCS)
$(OUT)/$(NAME).hex: $(OUT)/$(NAME).elf
	avr-objcopy -O ihex -R .eeprom $< $@
# flash (nano: -c arduino -P /dev/ttyUSB0 -b 115200 [57600 old bootloader]; mega: -c wiring -b 115200; attiny85: -c usbasp)
flash: $(OUT)/$(NAME).hex
	avrdude -p $(AVRDUDE_P) -c $(PROG) -P $(PORT) -U flash:w:$<
.PHONY: all flash}}
