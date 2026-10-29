#include "board.h"
#include <stdio.h>
#include <ctype.h>
int main(void) {
    stdio_init_all();       /* pico_enable_stdio_usb: TinyUSB CDC ACM */
    while (!stdio_usb_connected()) sleep_ms(50);
    printf("usb-cdc ready: type characters, they come back upper-cased\n");
    while (true) {
        int c = getchar_timeout_us(100000);
        if (c != PICO_ERROR_TIMEOUT) putchar(toupper(c));
    }
}
