// {{NAME}} on {{TARGET}} (bare metal): Timer0 CTC at 1 kHz toggles the LED every 500 ms.
#include "board.h"

static volatile uint16_t ms;

ISR(TIMER0_COMPA_vect) {
  if (++ms >= 500) { ms = 0; LED_PORT ^= _BV(LED_BIT); }
}

int main(void) {
  LED_DDR |= _BV(LED_BIT);
  TCCR0A = _BV(WGM01);
  TCCR0B = _BV(CS01) | _BV(CS00);
  OCR0A  = (uint8_t)(F_CPU / 64 / 1000 - 1);
  TIMSK0 = _BV(OCIE0A);
  sei();
  for (;;) { set_sleep_mode(SLEEP_MODE_IDLE); sleep_mode(); }
}
