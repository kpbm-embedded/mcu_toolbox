#define LED_PIN 33   // P3.3, CH55xDuino numbering = port*10 + bit

void setup() { pinMode(LED_PIN, OUTPUT); }
void loop() {
  digitalWrite(LED_PIN, HIGH); delay(500);
  digitalWrite(LED_PIN, LOW);  delay(500);
}
