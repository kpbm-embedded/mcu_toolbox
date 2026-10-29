#include "board.h"

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_INIT();
    printf("{{NAME}} on {{BOARD}}\r\n");
    for (unsigned n = 0;; n++) {
        LED_SET(n & 1);
        Delay_Ms(500);
    }
}
