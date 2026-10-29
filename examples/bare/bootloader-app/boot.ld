/* Bootloader: 64 KB at 0x0; the app slot (header + image) starts at 0x10000. */
MEMORY
{
  FLASH  (rx)  : ORIGIN = 0x00000000, LENGTH = 64K
  APPIMG (rx)  : ORIGIN = 0x00010000, LENGTH = 1M
  RAM    (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}
INCLUDE cm.ld.in
SECTIONS { .app_image : { KEEP(*(.app_image)) } > APPIMG }
