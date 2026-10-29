/* A small firmware for the 16 KB / 2 KB CH32V003 (and the BluePill): a CRC-8 table lookup over a buffer. The budget
 * check is in test.sh; the run checks the result. */
#include "bare.h"

static const uint8_t data[] = "123456789";
static uint8_t table[256];

static void crc8_init(void)
{
  for (int i = 0; i < 256; i++) {
    uint8_t c = (uint8_t)i;
    for (int b = 0; b < 8; b++) c = (uint8_t)((c & 0x80) ? (c << 1) ^ 0x07 : c << 1);
    table[i] = c;
  }
}

int main(void)
{
  crc8_init();
  uint8_t crc = 0;
  for (unsigned i = 0; i < sizeof data - 1; i++) crc = table[crc ^ data[i]];
  bare_printf("crc8(\"123456789\") = 0x%02x\n", crc);
  test_check(crc == 0xF4, "CRC-8/SMBUS check value 0xF4");
  test_done();
}
