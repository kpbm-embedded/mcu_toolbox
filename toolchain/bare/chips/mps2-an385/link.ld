/* qemu mps2-an385: 4 MB SSRAM1 at 0 (code, loaded by -kernel), 4 MB SSRAM2/3 at 0x20000000 (data) */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}
INCLUDE cm.ld.in
