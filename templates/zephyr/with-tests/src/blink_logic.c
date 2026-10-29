#include "blink_logic.h"

uint32_t blink_period_ms(uint32_t level)
{
	uint32_t p = 2000U - level * 100U;
	if (level > 19U || p < 50U) { return 50U; }
	return p;
}
