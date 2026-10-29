/* nice!nano v2: Adafruit nRF52 UF2 bootloader with SoftDevice S140 6.1.1 -> application from 0x26000 to 0xF4000. */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00026000, LENGTH = 0xF4000 - 0x26000
  RAM   (rwx) : ORIGIN = 0x20006000, LENGTH = 0x3A000      /* above the SoftDevice's RAM */
}
INCLUDE cm.ld.in
