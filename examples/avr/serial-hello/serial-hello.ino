#if defined(__AVR_ATtiny85__)
// ATtiny85 has no UART: ATTinyCore's builtin software Serial, TX = AIN0 = PB0, RX = AIN1 = PB1 (half duplex).
#define BAUD 9600
#else
#define BAUD 115200
#endif

void setup() { Serial.begin(BAUD); }
void loop() {
  static unsigned n;
  Serial.print(F("hello "));
  Serial.println(n++);
  delay(1000);
}
