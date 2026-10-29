#include <stdio.h>
#include <unistd.h>
#include "drv_fpioa.h"
#include "drv_gpio.h"
#include "board.h"

int main(void)
{
    drv_gpio_inst_t *led = NULL;
    if (drv_fpioa_set_pin_func(LED_GPIO, GPIO0 + LED_GPIO) != 0) { printf("fpioa mux of pin %d failed\n", LED_GPIO); return 1; }
    if (drv_gpio_inst_create(LED_GPIO, &led) != 0) { printf("gpio %d unavailable\n", LED_GPIO); return 1; }
    drv_gpio_mode_set(led, GPIO_DM_OUTPUT);
    printf("gpio-blink: GPIO%d (%s)\n", LED_GPIO, LED_ACTIVE_LOW ? "active low" : "active high");
    for (int i = 0; i < 20; i++) {
        int on = i & 1;
        drv_gpio_value_set(led, (on ^ LED_ACTIVE_LOW) ? GPIO_PV_HIGH : GPIO_PV_LOW);
        printf("led %s\n", on ? "on" : "off");
        usleep(500 * 1000);
    }
    drv_gpio_inst_destroy(&led);
    return 0;
}
