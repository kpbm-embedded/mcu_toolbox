/* C owns startup, console and the test protocol; the logic is in Zig (src/logic.zig) behind a C ABI. */
#include <stddef.h>
#include "bare.h"

uint16_t zig_crc16_ccitt(const uint8_t *data, size_t len);
uint32_t zig_bitrev32(uint32_t x);
void zig_hello(void);

int main(void)
{
  zig_hello();
  test_check(zig_crc16_ccitt((const uint8_t *)"123456789", 9) == 0x29B1, "CRC-16/CCITT-FALSE check value 0x29B1 (comptime table)");
  test_check(zig_bitrev32(0x00000001u) == 0x80000000u && zig_bitrev32(0x12345678u) == 0x1E6A2C48u, "@bitReverse");
  test_done();
}
