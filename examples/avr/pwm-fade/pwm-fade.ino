#if defined(__AVR_ATtiny85__)
const uint8_t PWM_PIN = 1;   // PB1 / OC0B
#else
const uint8_t PWM_PIN = 9;
#endif
void setup() { pinMode(PWM_PIN, OUTPUT); }
void loop() {
  for (int v = 0; v <= 255; v++) { analogWrite(PWM_PIN, v); delay(4); }
  for (int v = 255; v >= 0; v--) { analogWrite(PWM_PIN, v); delay(4); }
}
