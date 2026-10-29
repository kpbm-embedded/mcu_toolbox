/* Hardware timer 0 in periodic mode (500 ms) toggles the LED; a software timer counts 100 ms ticks (drv_timer). */
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include "drv_fpioa.h"
#include "drv_gpio.h"
#include "drv_timer.h"
#include "board.h"

static atomic_int hw_ticks, sw_ticks;

static void on_hw(void *args) { atomic_fetch_add(&hw_ticks, 1); drv_gpio_toggle((drv_gpio_inst_t *)args); }
static void on_sw(void *args) { (void)args; atomic_fetch_add(&sw_ticks, 1); }

int main(void)
{
    drv_gpio_inst_t *led = NULL;
    drv_hard_timer_inst_t *hw = NULL;
    drv_soft_timer_inst_t *sw = NULL;
    rt_hwtimer_info_t info;

    drv_fpioa_set_pin_func(LED_GPIO, GPIO0 + LED_GPIO);
    if (drv_gpio_inst_create(LED_GPIO, &led)) { printf("gpio %d unavailable\n", LED_GPIO); return 1; }
    drv_gpio_mode_set(led, GPIO_DM_OUTPUT);

    if (drv_hard_timer_inst_create(0, &hw)) { printf("hard timer 0 unavailable\n"); return 1; }
    drv_hard_timer_get_info(hw, &info);
    printf("timer0: freq %u..%u Hz, maxcnt %u\n", (unsigned)info.minfreq, (unsigned)info.maxfreq, (unsigned)info.maxcnt);
    drv_hard_timer_set_mode(hw, HWTIMER_MODE_PERIOD);
    drv_hard_timer_set_freq(hw, info.minfreq);           /* slowest clock gives the longest possible period */
    if (drv_hard_timer_set_period(hw, 500)) { printf("500 ms period out of range\n"); return 1; }
    drv_hard_timer_register_irq(hw, on_hw, led);
    drv_hard_timer_start(hw);

    drv_soft_timer_create(&sw);
    drv_soft_timer_set_mode(sw, HWTIMER_MODE_PERIOD);
    drv_soft_timer_set_period(sw, 100);
    drv_soft_timer_register_irq(sw, on_sw, NULL);
    drv_soft_timer_start(sw);

    for (int i = 0; i < 10; i++) {
        sleep(1);
        printf("timer (%s): hw %d x 500 ms, sw %d x 100 ms\n", BOARD_NAME, atomic_load(&hw_ticks), atomic_load(&sw_ticks));
    }
    drv_soft_timer_stop(sw);
    drv_soft_timer_destroy(&sw);
    drv_hard_timer_stop(hw);
    drv_hard_timer_inst_destroy(&hw);
    drv_gpio_inst_destroy(&led);
    return 0;
}
