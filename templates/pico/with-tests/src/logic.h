#pragma once
#include <stdint.h>
/* Hardware-independent logic: unit-tested on the host, reused by the firmware. */
uint32_t clamp_u32(uint32_t v, uint32_t lo, uint32_t hi);
uint8_t  triangle_u8(uint32_t step);   /* 0..255..0 ramp, period 510 */
