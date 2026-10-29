#include "logic.h"

uint32_t clamp_u32(uint32_t v, uint32_t lo, uint32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }

uint8_t triangle_u8(uint32_t phase)
{
  phase %= 510U;
  return (uint8_t)(phase <= 255U ? phase : 510U - phase);
}

/* S O S = 3 short, 3 long, 3 short; step pairs are (on, off). unit = 150 ms */
static const uint8_t pat[] = { 1,1, 1,1, 1,1, 3,1, 3,1, 3,1, 1,1, 1,1, 1,7 };
unsigned sos_len(void) { return sizeof pat; }
int sos_led_on(unsigned step) { return (step % sizeof pat) % 2U == 0U; }
uint32_t sos_step_ms(unsigned step) { return 150U * pat[step % sizeof pat]; }
