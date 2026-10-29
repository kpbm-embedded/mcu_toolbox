# WCH CH32V003 (QingKe V2A, RV32EC): 16 KB flash, 2 KB RAM. Console USART1 TX=PD5 at 115200 from the 24 MHz HSI.
# Renode has no WCH model: chips/ch32v003/renode.repl (RV32EC + STM32F1-style USART1) runs the console code;
# interrupts (PFIC) and SysTick are not modelled. Flash a board with wlink/minichlink.
set(CHIP_ARCH riscv)
set(CHIP_FLAGS -march=rv32ec_zicsr -mabi=ilp32e -msmall-data-limit=0)
set(CHIP_LD ${CMAKE_CURRENT_LIST_DIR}/link.ld)
set(CHIP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../common/rv_start.S ${CMAKE_CURRENT_LIST_DIR}/chip.c)
set(CHIP_DEFINES CHIP_CH32V003)
set(CHIP_RUN "renode ${CMAKE_CURRENT_LIST_DIR}/renode.repl usart1")
