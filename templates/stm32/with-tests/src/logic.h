/* Hardware-free logic: compiled for the MCU and for the host unit tests. */
#pragma once
#include <stdint.h>

uint32_t clamp_u32(uint32_t v, uint32_t lo, uint32_t hi);
uint8_t triangle_u8(uint32_t phase);        /* 0..255..0 over phase 0..510, then wraps */
unsigned sos_len(void);                     /* number of steps in the SOS pattern */
int sos_led_on(unsigned step);              /* LED level at step (wraps) */
uint32_t sos_step_ms(unsigned step);        /* duration of that step in ms (wraps) */
