# Arm MPS2 AN385 (Cortex-M3) as emulated by qemu-system-arm -M mps2-an385. Console: CMSDK UART0; exit: semihosting.
set(CHIP_ARCH arm)
set(CHIP_FLAGS -mcpu=cortex-m3 -mthumb)
set(CHIP_LD ${CMAKE_CURRENT_LIST_DIR}/link.ld)
set(CHIP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../common/cm_startup.c ${CMAKE_CURRENT_LIST_DIR}/chip.c)
set(CHIP_DEFINES CHIP_NUM_IRQ=32 CHIP_MPS2_AN385)
set(CHIP_RUN "qemu-arm mps2-an385")
