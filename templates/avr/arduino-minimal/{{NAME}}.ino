// {{NAME}}: blink LED pin {{LED_PIN}} on {{TARGET}}; UART heartbeat at 115200 where the chip has a UART (not ATtiny85).
void setup() {
  pinMode({{LED_PIN}}, OUTPUT);
#ifdef HAVE_HWSERIAL0
  Serial.begin(115200);
#endif
}

void loop() {
  static unsigned n;
  digitalWrite({{LED_PIN}}, HIGH); delay(500);
  digitalWrite({{LED_PIN}}, LOW);  delay(500);
#ifdef HAVE_HWSERIAL0
  Serial.print(F("{{NAME}} tick "));
  Serial.println(n);
#endif
  n++;
}
