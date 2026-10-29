// Button on P3.2 (INT0, falling edge) toggles the LED on P3.3; count printed over USB CDC.
// Based on CH55xDuino Generic_Examples/02.Digital/AttachInterrupts. INT1 is P3.3 = the LED, so only INT0 is used.
#define LED_PIN 33   // P3.3 (CH55xDuino pin = port*10 + bit)
#define KEY_PIN 32   // P3.2 = INT0

volatile uint8_t presses = 0;

#pragma save
#pragma nooverlay
void onKey() {        // ISR context: keep it short
  presses++;
}
#pragma restore

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(KEY_PIN, INPUT_PULLUP);
  attachInterrupt(0, onKey, FALLING);   // 0 = INT0 = P3.2
}

void loop() {
  static uint8_t seen = 0;
  if (presses != seen) {
    seen = presses;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    USBSerial_print("button ");
    USBSerial_println(seen);
  }
}
