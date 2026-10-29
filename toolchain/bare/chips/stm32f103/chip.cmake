# STM32F103C8 (BluePill): Cortex-M3, 64 KB flash, 20 KB RAM. Console USART1 TX=PA9, 8 MHz HSI. Tested in Renode.
set(CHIP_ARCH arm)
set(CHIP_FLAGS -mcpu=cortex-m3 -mthumb)
set(CHIP_LD ${CMAKE_CURRENT_LIST_DIR}/link.ld)
set(CHIP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../common/cm_startup.c ${CMAKE_CURRENT_LIST_DIR}/chip.c)
set(CHIP_DEFINES CHIP_NUM_IRQ=68 CHIP_STM32F103)
set(CHIP_RUN "renode platforms/cpus/stm32f103.repl usart1")
