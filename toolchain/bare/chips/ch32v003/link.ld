MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 16K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 2K
}
_min_stack = 0x200;
INCLUDE rv.ld.in
