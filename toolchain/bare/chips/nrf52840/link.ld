/* nRF52840 without SoftDevice/bootloader (Renode, SWD flashing). For the nice!nano UF2 bootloader (S140 below the
 * app) link at 0x26000: see templates/bare/nrf52840-uf2. */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 1M
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
}
INCLUDE cm.ld.in
