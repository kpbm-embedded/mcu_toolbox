#pragma once
#include <stdint.h>
/* Pure logic (no hardware): blink period in ms for a duty step; clamps to [50, 2000]. */
uint32_t blink_period_ms(uint32_t level);
