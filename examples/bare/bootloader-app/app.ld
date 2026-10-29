/* Application: after the 256-byte image header at 0x10000 (vector table needs 256-byte alignment here). */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00010100, LENGTH = 1M - 0x100
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}
INCLUDE cm.ld.in
