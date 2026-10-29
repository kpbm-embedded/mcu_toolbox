const uint8_t BTN = 2;
volatile bool pressed = false;
static void isr() { pressed = true; }

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN), isr, FALLING);
}
void loop() {
  static unsigned long last;
  if (pressed) {
    pressed = false;
    if (millis() - last > 50) { last = millis(); digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); }
  }
}
