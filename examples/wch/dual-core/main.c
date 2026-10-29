/* CH32H417 dual core (V3F boots and starts the V5F), from ch32fun examples_h41x/dual_hello_world (MIT).
 * Each core prints its hart id on its own UART: V3F on USART1 TX PA9, V5F on USART6 TX PA12 (ch32fun _write()
 * picks the UART by mhartid). The V5F also blinks the LED (PE2) while the V3F counts. */
#include "board.h"

static volatile uint32_t v5f_ticks;   /* shared SRAM: the V3F reports the V5F's progress */

int main_V5F(void)
{
    printf("Hello from V5F, hart %lu, SP %lx\r\n", (unsigned long)__get_MHARTID(), (unsigned long)__get_SP());
    for (;;) {
        LED_SET(v5f_ticks & 1);
        v5f_ticks++;
        Delay_Ms(250);
    }
}

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_INIT();
    printf("Hello from V3F, hart %lu, SP %lx\r\n", (unsigned long)__get_MHARTID(), (unsigned long)__get_SP());
    StartV5F(main_V5F);
    for (unsigned n = 0;; n++) {
        Delay_Ms(1000);
        printf("V3F %u: V5F ticks %lu\r\n", n, (unsigned long)v5f_ticks);
    }
}
