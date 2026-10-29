#define LED_PIN 33

void setup() { pinMode(LED_PIN, OUTPUT); }

void loop() {
  static unsigned long last;
  while (USBSerial_available()) USBSerial_write(USBSerial_read());
  if (millis() - last >= 1000) {
    last = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    USBSerial_println("hello from CH552");
  }
}
