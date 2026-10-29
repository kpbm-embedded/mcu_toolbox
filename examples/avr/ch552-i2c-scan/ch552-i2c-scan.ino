// Bit-banged I2C scan (CH552 has no I2C peripheral) with the core's SoftI2C library: SCL P3.0, SDA P3.1, as in
// CH55xDuino libraries/SoftI2C/examples/i2cScanner. Results on the USB CDC port.
#include <SoftI2C.h>
#define SCL_PIN 30   // P3.0
#define SDA_PIN 31   // P3.1

void setup() { Wire_begin(SCL_PIN, SDA_PIN); }

void loop() {
  uint8_t n = 0;
  USBSerial_println("i2c-scan:");
  for (uint8_t a = 0x08; a < 0x78; a++) {
    delay(1);
    if (Wire_scan(a)) { USBSerial_print("  found 0x"); USBSerial_println(a, HEX); n++; }
  }
  USBSerial_print(n);
  USBSerial_println(" device(s)");
  delay(3000);
}
