#ifndef BOARD_H
#define BOARD_H
#include "ch32fun.h"
#include <stdio.h>
/* Pin map per target. Sources: BOARD_CH32V203 = WeAct BluePill Plus CH32V203: user LED PB2 (active high), user key PA0
 * (active high) from the Zephyr board file boards/weact/bluepillplus_ch32v203.dts. BOARD_CH32V307 = WCH CH32V307V-R1 EVT:
 * its two user LEDs are NOT wired to the SoC; Zephyr's board doc says to jumper LED1/LED2 to PD0/PD1 (assumed active low),
 * the WAKEUP key is PA0 (assumed active high). BOARD_CH32V003: the WeAct F4U6 core / EVT schematics could not be found;
 * defaults are PC0 (LED, active high; safe pin, not SWIO PD1 or NRST PD7) and PC3 (key, active low): put an LED/key there
 * or edit. BOARD_CH32H417: PE2 is the LED pin used by ch32fun's examples_h41x (unconfirmed for your board); key PA0
 * (WKUP, to GND, internal pull-up) is an arbitrary choice: there is no board file for the owned CH32H417.
 * Every value can be overridden with EXTRA_CFLAGS=-DLED_PIN=PB5 etc. */
#if defined(BOARD_CH32V003)
 #define BOARD_STR "CH32V003"
 #ifndef LED_PIN
  #define LED_PIN PC0
  #define LED_ON  FUN_HIGH
 #endif
 #ifndef KEY_PIN
  #define KEY_PIN PC3
  #define KEY_PRESSED 0
  #define KEY_PULL_UP 1
 #endif
#elif defined(BOARD_CH32V203)
 #define BOARD_STR "CH32V203"
 #ifndef LED_PIN
  #define LED_PIN PB2
  #define LED_ON  FUN_HIGH
 #endif
 #ifndef KEY_PIN
  #define KEY_PIN PA0
  #define KEY_PRESSED 1
  #define KEY_PULL_UP 0
 #endif
#elif defined(BOARD_CH32V307)
 #define BOARD_STR "CH32V307"
 #ifndef LED_PIN
  #define LED_PIN PD0
  #define LED_ON  FUN_LOW
 #endif
 #ifndef KEY_PIN
  #define KEY_PIN PA0
  #define KEY_PRESSED 1
  #define KEY_PULL_UP 0
 #endif
#elif defined(BOARD_CH32H417)
 #define BOARD_STR "CH32H417"
 #ifndef LED_PIN
  #define LED_PIN PE2
  #define LED_ON  FUN_HIGH
 #endif
 #ifndef KEY_PIN
  #define KEY_PIN PA0
  #define KEY_PRESSED 0
  #define KEY_PULL_UP 1
 #endif
#else
 #error "build with TARGET_NAME=ch32v003|ch32v203|ch32v307|ch32h417"
#endif
/* PIN_MODE(pin, cnf): ch32fun's funPinMode takes (pin, cnf, speed) on CH32H41x and (pin, speed|cnf) elsewhere.
 * cnf = GPIO_CNF_OUT_PP / _OUT_OD / _OUT_PP_AF / _IN_PUPD / _IN_ANALOG / _IN_FLOATING (input modes ignore the speed). */
#if defined(BOARD_CH32H417)
 #define PIN_MODE(p, cnf) funPinMode((p), (cnf), GPIO_Speed_180MHz)
 #define PIN_IN(p, cnf)   funPinMode((p), (cnf), GPIO_Speed_10MHz)
#else
 #define PIN_MODE(p, cnf) funPinMode((p), GPIO_Speed_10MHz | (cnf))
 #define PIN_IN(p, cnf)   funPinMode((p), (cnf))
#endif
#define PIN_OUT(p) PIN_MODE((p), GPIO_CNF_OUT_PP)
#define LED_INIT() PIN_OUT(LED_PIN)
#define LED_OFF (LED_ON == FUN_HIGH ? FUN_LOW : FUN_HIGH)
#define LED_SET(on) funDigitalWrite(LED_PIN, (on) ? LED_ON : LED_OFF)
#endif
