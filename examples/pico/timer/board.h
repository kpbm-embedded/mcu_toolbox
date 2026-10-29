#pragma once
#include "pico/stdlib.h"
/* Plain LED: GPIO25 on Pico / Pico 2 (pico.h, pico2.h). The Waveshare RP2040-Zero
 * and RP2350-Zero have no plain LED (only the WS2812 on GPIO16), so wire an
 * external LED + resistor from GP15 to GND there. */
#ifdef PICO_DEFAULT_LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN
#else
#define LED_PIN 15
#endif
/* Button: external momentary switch from GP14 to GND (internal pull-up). The Zero boards
 * route BOOT to the flash chip, not to a GPIO, so there is no usable on-board button. */
#define BUTTON_PIN 14
