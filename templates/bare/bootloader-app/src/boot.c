/* Bootloader: check the app header and CRC-32, then hand over (VTOR, MSP, reset vector). */
#include "bare.h"

#define SLOT 0x00010000u
#define APP (SLOT + 0x100u)
typedef struct { char magic[4]; uint32_t size, crc; } header_t;

static uint32_t crc32(const uint8_t *p, uint32_t n)
{
  uint32_t c = 0xFFFFFFFFu;
  while (n--) { c ^= *p++; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u))); }
  return ~c;
}

int main(void)
{
  const header_t *h = (const header_t *)SLOT;
  if (h->magic[0] != 'A' || h->magic[3] != '1') { bare_printf("boot: no app\nRESULT: FAIL (no app)\n"); return 1; }
  uint32_t crc = crc32((const uint8_t *)APP, h->size);
  bare_printf("boot: app %lu bytes, crc %08lx (%s)\n", (unsigned long)h->size, (unsigned long)crc, crc == h->crc ? "ok" : "BAD");
  if (crc != h->crc) { bare_printf("RESULT: FAIL (crc)\n"); return 1; }
  const uint32_t *vec = (const uint32_t *)APP;
  REG32(0xE000ED08u) = APP;                                   /* VTOR */
  __asm volatile("msr msp, %0\n bx %1" : : "r"(vec[0]), "r"(vec[1]));
  return 1;
}
