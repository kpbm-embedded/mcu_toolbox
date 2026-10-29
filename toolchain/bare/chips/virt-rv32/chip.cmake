# qemu-system-riscv32 -M virt (RV32IMAC): NS16550A UART at 0x10000000, SiFive test device at 0x100000 for the exit
# status, RAM at 0x80000000. Compiler: xPack riscv-none-elf-gcc (wch/toolchain).
set(CHIP_ARCH riscv)
set(CHIP_FLAGS -march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medany)
set(CHIP_LD ${CMAKE_CURRENT_LIST_DIR}/link.ld)
set(CHIP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../common/rv_start.S ${CMAKE_CURRENT_LIST_DIR}/chip.c)
set(CHIP_DEFINES CHIP_VIRT_RV32)
set(CHIP_RUN "qemu-riscv virt")
