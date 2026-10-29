#!/usr/bin/env bash
# Exit 3 (skip) when the Cube pack for the chip ($1) is not installed (mcu setup install stm32cube-f4|f1|h7).
case "${1:-stm32f411}" in stm32f103) f=F1 ;; stm32h743) f=H7 ;; *) f=F4 ;; esac
[ -d "{{MCU_ROOT}}/stm32/STM32Cube$f/Drivers" ] || { echo "SKIP: STM32Cube$f not installed (mcu setup install stm32)"; exit 3; }
