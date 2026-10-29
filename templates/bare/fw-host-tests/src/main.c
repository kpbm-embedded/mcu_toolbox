/* The firmware side of the split: the same ring buffer the host tests cover, used on the target. */
#include "bare.h"
#include "ringbuf.h"

int main(void)
{
  uint8_t storage[8], out;
  ringbuf_t rb;
  ringbuf_init(&rb, storage, sizeof storage);
  int pushed = 0;
  for (int i = 0; i < 10; i++) pushed += ringbuf_put(&rb, (uint8_t)i);
  test_check(pushed == 7, "capacity is size - 1 (7 of 10 accepted)");
  int ok = 1;
  for (int i = 0; i < 7; i++) ok &= ringbuf_get(&rb, &out) && out == i;
  test_check(ok, "FIFO order on the target");
  test_check(!ringbuf_get(&rb, &out), "empty after draining");
  test_done();
}
