/* Pin map for the owned boards. Console UART pins come from the SDK board files
 * (bsp/board/<chip>dk/board.c: bl616 UART0 TX=GPIO21 RX=GPIO22, bl702 UART0 TX=GPIO14 RX=GPIO15,
 * bl808 console = UART on M0 per bsp/board/bl808dk/board.c), peripheral pins from board_*_gpio_init() there.
 * LED_PIN / BTN_PIN are NOT confirmed against the Sipeed schematics (not reachable offline):
 * check the schematic or override on the make line, e.g.  make ... cmake_definition="-DLED_PIN=GPIO_PIN_27"
 * (or edit the values below). BTN is the BOOT key (active low, GPIO2 is the BL616 boot-strap pin). */
#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#if defined(BL616)          /* Sipeed M0s Dock / Ai-M62 */
#define LED_PIN   GPIO_PIN_27
#define BTN_PIN   GPIO_PIN_2
#elif defined(BL702)        /* Sipeed M0sense / RV-Debugger Plus */
#define LED_PIN   GPIO_PIN_22
#define BTN_PIN   GPIO_PIN_2
#elif defined(BL808)        /* Sipeed M1s Dock, M0 core */
#define LED_PIN   GPIO_PIN_8
#define BTN_PIN   GPIO_PIN_22
#else
#error "unsupported chip"
#endif

#endif
