#pragma once
/* Pin map per target. See README "Pins and sources". Change here for other boards. */
#include "sdkconfig.h"
#if CONFIG_IDF_TARGET_ESP32
/* ESP32 WROOM-32 CH340C devkit (DevKitC-style clone). Console = UART0 via CH340 (/dev/ttyUSB0). */
#define PIN_WS2812      4   /* no onboard WS2812: wire an external one (free GPIO, unverified) */
#define PIN_LED         2   /* onboard blue LED on most WROOM-32 CH340 boards: UNVERIFIED (not on Espressif DevKitC) */
#define PIN_BUTTON      0   /* BOOT button = GPIO0 strapping pin (Espressif ESP32-DevKitC user guide) */
#define PIN_I2C_SDA     21  /* community/Arduino-ESP32 convention, any GPIO works via the matrix */
#define PIN_I2C_SCL     22
#define PIN_UART_TX     17  /* UART2 IOMUX pins (IDF soc/esp32/uart_pins.h), routed to UART1; UART1 IOMUX 9/10 = flash */
#define PIN_UART_RX     16
#define ADC_CH          ADC_CHANNEL_0 /* ADC1_CH0 = GPIO36 / SENSOR_VP (IDF soc/esp32/adc_channel.h) */
#elif CONFIG_IDF_TARGET_ESP32S3
#define PIN_WS2812      21  /* ESP32-S3-Zero onboard WS2812 (Waveshare wiki) */
#define PIN_LED         2   /* external LED + resistor (devkit has no plain LED) */
#define PIN_BUTTON      0   /* BOOT button */
#define PIN_I2C_SDA     8   /* IDF default I2C0 pins for S3 */
#define PIN_I2C_SCL     9
#define PIN_UART_TX     17
#define PIN_UART_RX     18
#define ADC_CH          ADC_CHANNEL_0 /* ADC1_CH0 = GPIO1 */
#elif CONFIG_IDF_TARGET_ESP32C3
#define PIN_WS2812      10  /* ESP32-C3-Zero onboard WS2812 (Waveshare wiki) */
#define PIN_LED         8   /* ESP32-C3 SuperMini blue LED, active low */
#define PIN_BUTTON      9   /* BOOT button */
#define PIN_I2C_SDA     5
#define PIN_I2C_SCL     6
#define PIN_UART_TX     3
#define PIN_UART_RX     4
#define ADC_CH          ADC_CHANNEL_0 /* ADC1_CH0 = GPIO0 */
#elif CONFIG_IDF_TARGET_ESP32C6
#define PIN_WS2812      8   /* ESP32-C6-DevKitC-1 WS2812; WT9932P4-TINY: UNKNOWN */
#define PIN_LED         15  /* external LED + resistor (assumed free pin) */
#define PIN_BUTTON      9   /* BOOT button */
#define PIN_I2C_SDA     6
#define PIN_I2C_SCL     7
#define PIN_UART_TX     4
#define PIN_UART_RX     5
#define ADC_CH          ADC_CHANNEL_0 /* ADC1_CH0 = GPIO0 */
#elif CONFIG_IDF_TARGET_ESP32P4
#define PIN_WS2812      23  /* UNKNOWN on WT9932P4-TINY: wire an external WS2812 */
#define PIN_LED         23  /* UNKNOWN on WT9932P4-TINY: wire an external LED */
#define PIN_BUTTON      35  /* BOOT button on P4 function-ev-board (assumed) */
#define PIN_I2C_SDA     7
#define PIN_I2C_SCL     8
#define PIN_UART_TX     10
#define PIN_UART_RX     11
#define ADC_CH          ADC_CHANNEL_0 /* ADC1_CH0 = GPIO16 */
#else
#error "unsupported target"
#endif
