#include "board.h"

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_INIT();
    for (;;) {
        LED_SET(1); Delay_Ms(250);
        LED_SET(0); Delay_Ms(250);
    }
}
