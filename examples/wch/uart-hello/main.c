#include "board.h"
#define TARGET_NAME_STR BOARD_STR

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_INIT();
    Delay_Ms(100);
    printf("hello from %s, SystemCoreClock %lu Hz\r\n", TARGET_NAME_STR, (unsigned long)FUNCONF_SYSTEM_CORE_CLOCK);
    for (unsigned n = 0;; n++) {
        LED_SET(n & 1);
        printf("count %u\r\n", n);
        Delay_Ms(500);
    }
}
