# i2c-scan

Scan the I2C bus with the new i2c_master driver (i2c_master_probe).

Status: builds; untested on hardware.

Hardware: I2C device on PIN_I2C_SDA/SCL (ESP32 21/22, S3 8/9, C3 5/6, C6 6/7, P4 7/8) with 4.7k pull-ups.

## Pins and sources
All pins live in `main/pins.h` (per target). Sources:
- ESP32 WROOM-32 CH340C: BOOT = GPIO0 (Espressif ESP32-DevKitC user guide); ADC1_CH0 = GPIO36 and UART2 IOMUX pins 17/16 (IDF `components/soc/esp32/include/soc/{adc_channel,uart_pins}.h`); onboard LED GPIO2, I2C 21/22 (community convention) and WS2812 GPIO4 (external) are UNVERIFIED.
- ESP32-S3-Zero WS2812 = GPIO21, ESP32-C3-Zero WS2812 = GPIO10: Waveshare wiki pinout pages (not re-verified offline; check the board silkscreen).
- ESP32-C3 SuperMini LED = GPIO8 (active low), BOOT = GPIO9: board vendor schematic/community pinout.
- ESP32-S3 BOOT = GPIO0, ESP32-C6 BOOT = GPIO9: Espressif DevKit user guides. ADC channels, default I2C pins: ESP-IDF GPIO/ADC tables for each chip.
- UNKNOWN: LED/button/WS2812 pins of the WT9932P4-TINY (P4) and C6 companion; values for esp32p4 and the "external LED" pins are placeholders, edit `pins.h`.

## Build, flash, monitor
```
source mcu/esp-idf/env.sh
T=esp32c3        # one of: esp32, esp32s3, esp32c3, esp32c6, esp32p4
idf.py -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/i2c-scan/$T -DSDKCONFIG=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/i2c-scan/$T/sdkconfig -DIDF_TARGET=$T build
idf.py -B <same build dir> -p /dev/ttyACM0 flash monitor
```
Hold BOOT and reset for download mode on native-USB boards.
ESP32 WROOM-32 CH340C: console on UART0 via the CH340, port /dev/ttyUSB0 (auto-reset into download mode via DTR/RTS).

Expected: List of 7-bit addresses every 3 s.
