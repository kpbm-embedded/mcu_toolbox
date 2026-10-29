#include "board.h"
#include <stdio.h>
int main(void) {
    stdio_init_all();   /* UART0 on GP0/GP1, 115200 (pico_enable_stdio_uart) */
    for (unsigned n = 0;; n++) {
        printf("hello from UART0, n=%u\n", n);
        sleep_ms(1000);
    }
}
