# nRF52840 (nice!nano / Pro Micro nRF52840): Cortex-M4F, 1 MB flash, 256 KB RAM. Console UART0 (legacy registers),
# TXD on P0.06 (nice!nano D1). Tested in Renode (nrf52840 model). The image links at 0x0 (no SoftDevice); for the
# nice!nano UF2 bootloader with S140 see templates/bare/nrf52840-uf2 (links at 0x26000).
set(CHIP_ARCH arm)
set(CHIP_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(CHIP_LD ${CMAKE_CURRENT_LIST_DIR}/link.ld)
set(CHIP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../common/cm_startup.c ${CMAKE_CURRENT_LIST_DIR}/chip.c)
set(CHIP_DEFINES CHIP_NUM_IRQ=48 CHIP_NRF52840)
set(CHIP_RUN "renode ${CMAKE_CURRENT_LIST_DIR}/renode.repl uart0")
