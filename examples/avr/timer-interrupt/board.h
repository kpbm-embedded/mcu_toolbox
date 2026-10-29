// board.h: per-target LED pin and register aliases. Selected by -DBOARD_xxx (see Makefile TARGET).
#pragma once
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#if defined(BOARD_NANO)        // ATmega328P, Arduino D13 = PB5
  #define LED_PORT PORTB
  #define LED_DDR  DDRB
  #define LED_BIT  PB5
#elif defined(BOARD_MEGA)      // ATmega2560, Arduino D13 = PB7
  #define LED_PORT PORTB
  #define LED_DDR  DDRB
  #define LED_BIT  PB7
#elif defined(BOARD_ATTINY85)  // ATTinyCore tinyX5 LED_BUILTIN = 1 = PB1
  #define LED_PORT PORTB
  #define LED_DDR  DDRB
  #define LED_BIT  PB1
  #ifndef TIMSK0
    #define TIMSK0 TIMSK
  #endif
  #ifndef WDTCSR
    #define WDTCSR WDTCR
  #endif
#else
  #error "define BOARD_NANO, BOARD_MEGA or BOARD_ATTINY85"
#endif
