// Hardware UART0 (Serial0): TX P3.1, RX P3.0, 115200 8N1. Prints a counter every second and echoes received bytes.
// The USB CDC port of the board stays the CH55xDuino default (used for uploads/reset), the UART needs a USB-UART adapter.
#define LED_PIN 33

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial0_begin(115200);
  Serial0_println("hello from CH552 UART0");
}

void loop() {
  static unsigned long last = 0;
  static unsigned int n = 0;
  while (Serial0_available()) {
    uint8_t c = Serial0_read();
    Serial0_print("echo: ");
    Serial0_write(c);
    Serial0_println("");
  }
  if (millis() - last >= 1000) {
    last = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    Serial0_print("alive ");
    Serial0_println(++n);
  }
}
