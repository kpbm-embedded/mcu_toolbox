#include <avr/io.h>
#include <util/delay.h>
/* Arduino Nano/Uno LED on PB5 (D13). For ATtiny85 use DDRB/PORTB bit PB1 or PB0 and F_CPU=8000000. */
int main(void) {
    DDRB |= _BV(PB5);
    for (;;) { PORTB ^= _BV(PB5); _delay_ms(500); }
}
