# Bare-metal blink with the avr-gcc bundled by arduino-cli. Override MCU/F_CPU, e.g. make MCU=attiny85 F_CPU=8000000
GCC  ?= $(wildcard $(abspath ../data)/packages/arduino/tools/avr-gcc/*/bin)
MCU  ?= atmega328p
F_CPU?= 16000000UL
OUT  ?= $(or $(MCU_OUT_BASE),$(abspath $(dir $(lastword $(MAKEFILE_LIST)))../../../build))/avr/baremetal-$(MCU)
all: $(OUT)/blink.hex
	$(GCC)/avr-size -C --mcu=$(MCU) $(OUT)/blink.elf
$(OUT)/blink.elf: blink.c
	mkdir -p $(OUT)
	$(GCC)/avr-gcc -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -o $@ $<
$(OUT)/blink.hex: $(OUT)/blink.elf
	$(GCC)/avr-objcopy -O ihex -R .eeprom $< $@
# flash: avrdude -p m328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:$(OUT)/blink.hex
