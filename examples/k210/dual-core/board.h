#ifndef BOARD_H
#define BOARD_H
/* Pin map for the owned K210 boards (target m1n = Sipeed M1n Maix Nano, cm1 = CM-1 K210 CanMV).
 * UNCONFIRMED for both boards: the defaults follow the Sipeed Maix family (Maix Bit/Dock/Duino: RGB LED
 * IO12=B IO13=R IO14=G, active low; BOOT/ISP key IO16, which is the K210 ISP strap pin). I could not find a
 * published schematic for M1n or CM-1 that confirms them. Override on the command line / in CMake, e.g.
 * add_definitions(-DPIN_LED_R=21), once you have checked your board with a multimeter. */
#if !defined(BOARD_M1N) && !defined(BOARD_CM1)
#error "define BOARD_M1N or BOARD_CM1 (the CMake variable BOARD = m1n | cm1 does it)"
#endif
#ifndef PIN_LED_B
#define PIN_LED_B 12
#endif
#ifndef PIN_LED_R
#define PIN_LED_R 13
#endif
#ifndef PIN_LED_G
#define PIN_LED_G 14
#endif
#ifndef PIN_KEY
#define PIN_KEY 16
#endif
#define LED_ON  0  /* active low */
#define LED_OFF 1
/* free pins used by the i2c/uart demos (UNCONFIRMED: check your header, any IO can be routed by the FPIOA) */
#ifndef PIN_I2C_SCL
#define PIN_I2C_SCL 30
#endif
#ifndef PIN_I2C_SDA
#define PIN_I2C_SDA 31
#endif
#endif
