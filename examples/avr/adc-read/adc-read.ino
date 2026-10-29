#if defined(__AVR_ATtiny85__)
// ATTinyCore: no hardware UART; the builtin software "Serial" sends on AIN0 = PB0 (TX only used here).
// A0 on the ATtiny85 is PB5 = RESET, so read A3 = PB3 (ADC3). 9600 baud is safe on the 8 MHz internal RC.
#define ADC_IN A3
#define BAUD 9600
#else
#define ADC_IN A0
#define BAUD 115200
#endif

void setup() { Serial.begin(BAUD); }

#if defined(__AVR_ATmega328P__)
static int tempRaw() {            // internal sensor: ADC8 vs 1.1 V reference
  ADMUX = _BV(REFS1) | _BV(REFS0) | 8;
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (ADCSRA & _BV(ADSC)) {}
  return ADC;
}
#endif

void loop() {
  int raw = analogRead(ADC_IN);   // default VCC reference (5 V assumed)
#if defined(__AVR_ATtiny85__)
  Serial.print(F("A3="));
#else
  Serial.print(F("A0="));
#endif
  Serial.print(raw);
  Serial.print(' '); Serial.print((long)raw * 5000 / 1023); Serial.println(F(" mV"));
#if defined(__AVR_ATmega328P__)
  tempRaw();                      // discard first conversion after the reference change
  Serial.print(F("temp_raw=")); Serial.println(tempRaw());
  analogReference(DEFAULT);
  analogRead(A0);
#endif
  delay(500);
}
