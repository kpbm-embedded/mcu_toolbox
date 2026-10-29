#include "board.h"

ISR(WDT_vect) { LED_PORT ^= _BV(LED_BIT); }

int main(void) {
  cli();
  MCUSR &= ~_BV(WDRF);
  WDTCSR |= _BV(WDCE) | _BV(WDE);                       // timed sequence
  WDTCSR  = _BV(WDIE) | _BV(WDP2) | _BV(WDP1);          // interrupt only, ~1 s
  LED_DDR |= _BV(LED_BIT);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sei();
  for (;;) sleep_mode();
}
