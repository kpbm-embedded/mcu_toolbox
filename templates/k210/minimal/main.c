#include <stdio.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "sleep.h"
#include "board.h"

int main(void)
{
    fpioa_set_function(PIN_LED_G, FUNC_GPIOHS0);
    gpiohs_set_drive_mode(0, GPIO_DM_OUTPUT);
    printf("{{NAME}} on {{BOARD}}\n");
    for (;;) {
        gpiohs_set_pin(0, LED_ON);  msleep(500);
        gpiohs_set_pin(0, LED_OFF); msleep(500);
    }
}
