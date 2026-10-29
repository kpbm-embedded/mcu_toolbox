// LED_BUILTIN: Nano D13 (PB5), Mega D13 (PB7), ATtiny85 pin 1 (PB1).
void setup() { pinMode(LED_BUILTIN, OUTPUT); }
void loop() {
  digitalWrite(LED_BUILTIN, HIGH); delay(1000);
  digitalWrite(LED_BUILTIN, LOW);  delay(1000);
}
