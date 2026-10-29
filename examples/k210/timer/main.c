#include <stdio.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "timer.h"
#include "plic.h"
#include "sysctl.h"
#include "sleep.h"
#include "board.h"

static volatile unsigned ticks100;

static int cb_led(void *ctx)
{
    (void)ctx;
    gpiohs_set_pin(0, gpiohs_get_pin(0) ? LED_ON : LED_OFF);
    return 0;
}
static int cb_tick(void *ctx)
{
    (void)ctx;
    ticks100++;
    return 0;
}

int main(void)
{
    plic_init();
    fpioa_set_function(PIN_LED_B, FUNC_GPIOHS0);
    gpiohs_set_drive_mode(0, GPIO_DM_OUTPUT);
    gpiohs_set_pin(0, LED_OFF);

    timer_init(TIMER_DEVICE_0);
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 500 * 1000 * 1000);   /* ns */
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, cb_led, NULL);
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);

    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_1, 100 * 1000 * 1000);
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_1, 0, 2, cb_tick, NULL);
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_1, 1);

    sysctl_enable_irq();
    for (;;) {
        printf("timer: %u x 100 ms\n", ticks100);   /* never printf inside the ISR */
        msleep(1000);
    }
}
