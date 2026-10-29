#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "drv_fpioa.h"
#include "drv_uart.h"
#include "board.h"

int main(void)
{
    drv_uart_inst_t *u = NULL;
    const char *msg = "hello k230 uart\r\n";
    uint8_t rx[64];

    drv_fpioa_set_pin_func(UART_TX_GPIO, UART2_TXD);
    drv_fpioa_set_pin_func(UART_RX_GPIO, UART2_RXD);
    drv_fpioa_set_pin_pu(UART_RX_GPIO, 1);
    if (drv_uart_inst_create(2, &u) != 0) { printf("uart2 open failed\n"); return 1; }

    for (int i = 0; i < 5; i++) {
        drv_uart_write(u, (const uint8_t *)msg, strlen(msg));
        if (drv_uart_poll(u, 500) > 0) {
            ssize_t n = drv_uart_read(u, rx, sizeof rx - 1);
            if (n > 0) { rx[n] = 0; printf("rx(%d): %s", (int)n, (char *)rx); }
        } else {
            printf("no data (jumper TX GPIO%d to RX GPIO%d)\n", UART_TX_GPIO, UART_RX_GPIO);
        }
        sleep(1);
    }
    drv_uart_inst_destroy(&u);
    return 0;
}
