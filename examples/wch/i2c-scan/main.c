#include "board.h"

/* Bit-banged I2C so the same code runs on every CH32 part. V003: SCL PC2, SDA PC1 (the I2C1 default pins).
 * V203/V307: SCL PB6, SDA PB7 (I2C1 default pins). H417: PB6/PB7 too (I2C1_SCL/SDA on AF4 in the CH32H417 datasheet
 * pin table). External pull-ups (2.2k-10k) required. */
#if defined(BOARD_CH32V003)
 #define SCL PC2
 #define SDA PC1
#else
 #define SCL PB6
 #define SDA PB7
#endif
#define OD(p) PIN_MODE((p), GPIO_CNF_OUT_OD)
#define HI(p) funDigitalWrite((p), FUN_HIGH)   /* open-drain: high = released */
#define LO(p) funDigitalWrite((p), FUN_LOW)
static void dly(void) { Delay_Us(5); }          /* ~100 kHz */

static void start(void) { HI(SDA); HI(SCL); dly(); LO(SDA); dly(); LO(SCL); dly(); }
static void stop(void)  { LO(SDA); dly(); HI(SCL); dly(); HI(SDA); dly(); }
static int wbyte(uint8_t b)   /* returns 1 if ACKed */
{
    for (int i = 7; i >= 0; i--) {
        if (b & (1 << i)) HI(SDA); else LO(SDA);
        dly(); HI(SCL); dly(); LO(SCL);
    }
    HI(SDA); dly(); HI(SCL); dly();
    int ack = !funDigitalRead(SDA);
    LO(SCL); dly();
    return ack;
}

int main(void)
{
    SystemInit();
    funGpioInitAll();
    OD(SCL); OD(SDA); HI(SCL); HI(SDA);
    Delay_Ms(100);
    printf("i2c-scan (bit-bang)\r\n");
    for (;;) {
        int n = 0;
        for (int a = 0x08; a <= 0x77; a++) {
            start();
            if (wbyte((uint8_t)(a << 1))) { printf("  found 0x%02x\r\n", a); n++; }
            stop();
        }
        printf("done, %d device(s)\r\n", n);
        Delay_Ms(3000);
    }
}
