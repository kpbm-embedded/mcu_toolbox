#include <stdio.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "plic.h"
#include "sysctl.h"
#include "sleep.h"
#include "board.h"

#define GPIO_LED 0
#define GPIO_KEY 1
static volatile int presses;

static int key_isr(void *ctx)
{
    (void)ctx;
    if (gpiohs_get_pin(GPIO_KEY) == 0) { /* pressed (pull-up, active low) */
        presses++;
        gpiohs_set_pin(GPIO_LED, gpiohs_get_pin(GPIO_LED) ? LED_ON : LED_OFF);
    }
    return 0;
}

int main(void)
{
    plic_init();
    fpioa_set_function(PIN_LED_G, FUNC_GPIOHS0 + GPIO_LED);
    gpiohs_set_drive_mode(GPIO_LED, GPIO_DM_OUTPUT);
    gpiohs_set_pin(GPIO_LED, LED_OFF);
    fpioa_set_function(PIN_KEY, FUNC_GPIOHS0 + GPIO_KEY);
    gpiohs_set_drive_mode(GPIO_KEY, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_pin_edge(GPIO_KEY, GPIO_PE_FALLING);
    gpiohs_irq_register(GPIO_KEY, 1, key_isr, NULL);
    sysctl_enable_irq();
    printf("button-irq: press the key on IO%d\n", PIN_KEY);
    int last = 0;
    for (;;) {
        if (presses != last) { last = presses; printf("presses: %d\n", last); }
        msleep(20);
    }
}
