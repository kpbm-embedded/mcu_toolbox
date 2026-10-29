#include <assert.h>
#include <stdio.h>
#include "ringbuf.h"

int main(void)
{
  uint8_t s[4], b;
  ringbuf_t rb;
  ringbuf_init(&rb, s, sizeof s);
  assert(ringbuf_count(&rb) == 0 && !ringbuf_get(&rb, &b));
  assert(ringbuf_put(&rb, 1) && ringbuf_put(&rb, 2) && ringbuf_put(&rb, 3));
  assert(!ringbuf_put(&rb, 4));                       /* full at size - 1 */
  assert(ringbuf_count(&rb) == 3);
  assert(ringbuf_get(&rb, &b) && b == 1);
  assert(ringbuf_put(&rb, 5));                        /* wraps around */
  assert(ringbuf_get(&rb, &b) && b == 2);
  assert(ringbuf_get(&rb, &b) && b == 3);
  assert(ringbuf_get(&rb, &b) && b == 5);
  assert(!ringbuf_get(&rb, &b));
  puts("ringbuf host tests: ok");
  return 0;
}
