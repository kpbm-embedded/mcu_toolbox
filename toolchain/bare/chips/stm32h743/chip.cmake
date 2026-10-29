# STM32H743 (mini H743 boards): Cortex-M7 (DP FPU), 2 MB flash, DTCM 128 KB at 0x20000000. Console USART1 TX=PA9 (AF7),
# 64 MHz HSI. Tested in Renode (stm32h743 model).
set(CHIP_ARCH arm)
set(CHIP_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
set(CHIP_LD ${CMAKE_CURRENT_LIST_DIR}/link.ld)
set(CHIP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../common/cm_startup.c ${CMAKE_CURRENT_LIST_DIR}/chip.c)
set(CHIP_DEFINES CHIP_NUM_IRQ=150 CHIP_STM32H743)
set(CHIP_RUN "renode platforms/cpus/stm32h743.repl usart1")
