// {{NAME}} on {{TARGET}}: blink P3.3 (pin {{LED_PIN}}, CH55xDuino numbering port*10+bit) and print over USB CDC.
void setup() { pinMode({{LED_PIN}}, OUTPUT); }

void loop() {
  digitalWrite({{LED_PIN}}, HIGH); delay(500);
  digitalWrite({{LED_PIN}}, LOW);  delay(500);
  USBSerial_println("{{NAME}} tick");
}
