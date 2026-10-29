# STM32F411CE (WeAct BlackPill): Cortex-M4F, 512 KB flash, 128 KB RAM. Console USART1 TX=PA9 (AF7), 16 MHz HSI.
# Tested in Renode on its stm32f412 model (same core, USART1 and memory map). Flash: st-flash/openocd/probe-rs (board).
set(CHIP_ARCH arm)
set(CHIP_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(CHIP_LD ${CMAKE_CURRENT_LIST_DIR}/link.ld)
set(CHIP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../common/cm_startup.c ${CMAKE_CURRENT_LIST_DIR}/chip.c)
set(CHIP_DEFINES CHIP_NUM_IRQ=86 CHIP_STM32F411)
set(CHIP_RUN "renode platforms/cpus/stm32f412.repl usart1")
