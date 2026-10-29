/* GPIO edge interrupt from the board key toggles the LED (RT-Smart user space, drv_gpio). */
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include "drv_fpioa.h"
#include "drv_gpio.h"
#include "board.h"

static atomic_int presses;

static void on_key(void *args)
{
    drv_gpio_inst_t *led = args;
    atomic_fetch_add(&presses, 1);
    drv_gpio_toggle(led);                 /* keep the callback short */
}

int main(void)
{
    drv_gpio_inst_t *led = NULL, *key = NULL;

    drv_fpioa_set_pin_func(LED_GPIO, GPIO0 + LED_GPIO);
    drv_fpioa_set_pin_func(KEY_GPIO, GPIO0 + KEY_GPIO);
    if (drv_gpio_inst_create(LED_GPIO, &led) || drv_gpio_inst_create(KEY_GPIO, &key)) { printf("gpio open failed\n"); return 1; }
    drv_gpio_mode_set(led, GPIO_DM_OUTPUT);
    drv_gpio_value_set(led, LED_ACTIVE_LOW ? GPIO_PV_HIGH : GPIO_PV_LOW);
    drv_gpio_mode_set(key, KEY_ACTIVE_LOW ? GPIO_DM_INPUT_PULLUP : GPIO_DM_INPUT_PULLDOWN);

    if (drv_gpio_register_irq(key, KEY_ACTIVE_LOW ? GPIO_PE_FALLING : GPIO_PE_RISING, 20 /* debounce ms */, on_key, led) ||
        drv_gpio_enable_irq(key)) {
        printf("irq on GPIO%d failed (only IO0..63 have interrupts)\n", KEY_GPIO);
        return 1;
    }
    printf("button-irq (%s): press the key on GPIO%d, LED on GPIO%d; exits after 30 s\n", BOARD_NAME, KEY_GPIO, LED_GPIO);
    int last = 0;
    for (int t = 0; t < 300; t++) {
        int now = atomic_load(&presses);
        if (now != last) { last = now; printf("presses: %d\n", now); }
        usleep(100 * 1000);
    }
    drv_gpio_disable_irq(key);
    drv_gpio_unregister_irq(key);
    drv_gpio_inst_destroy(&key);
    drv_gpio_inst_destroy(&led);
    return 0;
}
