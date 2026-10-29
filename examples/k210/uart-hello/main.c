#include <stdio.h>
#include <stdint.h>
#include "uarths.h"
#include "sysctl.h"
#include "sleep.h"
#include "board.h"

/* printf() in the standalone SDK is wired to UARTHS (IO4 = RX, IO5 = TX, the ISP port used by kflash). */
int main(void)
{
    uarths_init();
    uarths_config(115200, UARTHS_STOP_1);
    printf("hello from K210, cpu %u MHz\n", (unsigned)(sysctl_cpu_get_freq() / 1000000));
    unsigned tick = 0;
    for (;;) {
        uint8_t c;
        if (uarths_receive_data(&c, 1)) {
            printf("echo: %c (0x%02x)\n", c >= 32 && c < 127 ? c : '.', c);
        } else {
            msleep(50);
            if (++tick % 40 == 0)
                printf("alive %u s\n", tick / 20);
        }
    }
}
