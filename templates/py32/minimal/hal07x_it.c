/* PY32F071 (HAL) only: 1 ms SysTick for HAL_Delay. The LL targets build this file empty. */
#if defined(PY32F071x8)
#include "py32f07x_hal.h"
void SysTick_Handler(void) { HAL_IncTick(); }
#endif
