// Timer2 16-bit auto-reload interrupt every 10 ms (Fsys/12 clock); the ISR toggles the LED every 50 ticks (500 ms).
// CH55xDuino uses Timer0 for millis() and leaves Timer2 to the sketch: defining Timer2Interrupt() here overrides the
// core's empty one (cores/ch55xduino/weakOverrideTimer2.c, vector declared in main.c).
#define LED_PIN 33   // P3.3
#define T2_TICKS (F_CPU / 12 / 100)          // 10 ms at Fsys/12 (24 MHz: 20000)
#define T2_RELOAD (65536UL - T2_TICKS)

volatile uint8_t ticks = 0;
volatile uint16_t seconds = 0;

void Timer2Interrupt(void) __interrupt {
  if (TF2) {
    TF2 = 0;
    if (++ticks >= 50) {
      ticks = 0;
      P3 ^= (1 << 3);                         // toggle P3.3 directly (no core calls in the ISR)
      seconds++;
    }
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  T2CON = 0;                                  // timer mode, auto-reload, internal clock (Fsys/12 with bT2_CLK = 0)
  T2MOD &= ~bT2_CLK;
  RCAP2L = TL2 = T2_RELOAD & 0xFF;
  RCAP2H = TH2 = (T2_RELOAD >> 8) & 0xFF;
  ET2 = 1;
  TR2 = 1;
}

void loop() {
  static uint16_t shown = 0;
  if (seconds != shown) {
    shown = seconds;
    USBSerial_print("half-seconds ");
    USBSerial_println(shown);
  }
}
