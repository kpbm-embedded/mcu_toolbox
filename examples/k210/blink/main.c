#include <stdio.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "sleep.h"
#include "board.h"

int main(void)
{
    const int pins[3] = {PIN_LED_R, PIN_LED_G, PIN_LED_B};
    for (int i = 0; i < 3; i++) {
        fpioa_set_function(pins[i], FUNC_GPIOHS0 + i);
        gpiohs_set_drive_mode(i, GPIO_DM_OUTPUT);
        gpiohs_set_pin(i, LED_OFF);
    }
    printf("blink: R/G/B cycle\n");
    for (int n = 0;; n = (n + 1) % 3) {
        gpiohs_set_pin(n, LED_ON);
        msleep(400);
        gpiohs_set_pin(n, LED_OFF);
    }
}
