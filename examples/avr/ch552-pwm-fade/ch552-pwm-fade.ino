// Fade an LED with analogWrite() on P3.4 (PWM2). CH552 PWM pins: P1.5 / P3.0 (PWM1, mutually exclusive) and
// P3.4 / P3.1 (PWM2, mutually exclusive); see variants/ch552/pins_arduino.h and Generic_Examples/01.Basics/Fade.
// The board LED on P3.3 is not a PWM pin, so wire an external LED + 1k resistor from P3.4 to GND.
#define PWM_PIN 34   // P3.4

void setup() { pinMode(PWM_PIN, OUTPUT); }

void loop() {
  static int level = 0, step = 5;
  analogWrite(PWM_PIN, level);
  level += step;
  if (level <= 0 || level >= 255) step = -step;
  delay(20);
}
