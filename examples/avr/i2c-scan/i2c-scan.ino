#include <Wire.h>
#if defined(__AVR_ATtiny85__)
// ATTinyCore Wire runs on the USI: SDA = PB0, SCL = PB2. The builtin software Serial would send on PB0 (AIN0), so
// Serial.setTxBit(3) moves TX to PB3 (ATTinyCore 1.5 ChangeLog: "Serial.setTxBit()"). 9600 baud on the 8 MHz RC.
#define BAUD 9600
#else
#define BAUD 115200
#endif

void setup() {
#if defined(__AVR_ATtiny85__)
  Serial.setTxBit(3);
#endif
  Serial.begin(BAUD);
  Wire.begin();
}

void loop() {
  uint8_t n = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { Serial.print(F("found 0x")); Serial.println(a, HEX); n++; }
  }
  if (!n) Serial.println(F("no devices"));
  delay(3000);
}
