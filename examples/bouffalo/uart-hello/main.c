/* Print a banner on the console UART0 and echo every received character back. */
#include "bflb_uart.h"
#include "bflb_mtimer.h"
#include "board.h"

int main(void)
{
    board_init();

    struct bflb_device_s *uart0 = bflb_device_get_by_name("uart0");
    printf("uart-hello: type something, it is echoed back\r\n");

    uint32_t last = 0;
    unsigned n = 0;
    while (1) {
        int ch = bflb_uart_getchar(uart0);
        if (ch != -1) {
            bflb_uart_putchar(uart0, ch);
            if (ch == '\r') {
                bflb_uart_putchar(uart0, '\n');
            }
        }
        uint32_t now = (uint32_t)bflb_mtimer_get_time_ms();
        if (now - last >= 2000) {
            last = now;
            printf("hello %u\r\n", n++);
        }
    }
}
