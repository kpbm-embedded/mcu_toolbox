/* C owns startup, console and the test protocol; the logic is in Rust (src/lib.rs) behind a C ABI. */
#include <stddef.h>
#include "bare.h"

uint16_t rust_fletcher16(const uint8_t *data, size_t len);
void rust_sort(int32_t *data, size_t len);
void rust_hello(void);

int main(void)
{
  rust_hello();
  test_check(rust_fletcher16((const uint8_t *)"abcde", 5) == 0xC8F0, "Fletcher-16(\"abcde\") = 0xC8F0 computed in Rust");
  int32_t v[] = { 42, -7, 3, 1000, 0, -7 };
  rust_sort(v, 6);
  int sorted = 1;
  for (int i = 1; i < 6; i++) sorted &= v[i - 1] <= v[i];
  test_check(sorted && v[0] == -7 && v[5] == 1000, "slice sort_unstable() from core, called through FFI");
  test_done();
}
