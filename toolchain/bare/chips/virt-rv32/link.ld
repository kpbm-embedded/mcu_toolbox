/* qemu riscv32 virt: everything in RAM at 0x80000000 (loaded by -kernel / -bios none) */
MEMORY
{
  FLASH (rwx) : ORIGIN = 0x80000000, LENGTH = 1M
  RAM   (rwx) : ORIGIN = 0x80100000, LENGTH = 1M
}
INCLUDE rv.ld.in
