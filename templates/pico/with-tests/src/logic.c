#include "logic.h"
uint32_t clamp_u32(uint32_t v, uint32_t lo, uint32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }
uint8_t triangle_u8(uint32_t step) { step %= 510; return (uint8_t)(step < 255 ? step : 510 - step); }
