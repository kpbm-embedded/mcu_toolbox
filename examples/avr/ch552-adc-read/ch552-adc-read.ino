// Read the 8-bit ADC on P1.1 (AIN0) twice a second and print raw and mV (VDD reference) over USB CDC.
// ADC pins of the CH552 in CH55xDuino: 11 (P1.1 AIN0), 14 (P1.4 AIN1), 15 (P1.5 AIN2), 32 (P3.2 AIN3);
// see variants/ch552/pins_arduino.h (analog table) and Generic_Examples/01.Basics/AnalogReadSerial.
#define ADC_PIN 11   // P1.1
#define VDD_MV 5000  // 24 MHz needs the 5 V supply (boards.txt: "24 MHz (internal), 5V")

void setup() { pinMode(ADC_PIN, INPUT); }

void loop() {
  uint8_t raw = analogRead(ADC_PIN);
  USBSerial_print("P1.1 raw ");
  USBSerial_print(raw);
  USBSerial_print(" = ");
  USBSerial_print((unsigned long)raw * VDD_MV / 255);
  USBSerial_println(" mV");
  delay(500);
}
