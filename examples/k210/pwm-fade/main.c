#include <stdio.h>
#include "fpioa.h"
#include "pwm.h"
#include "timer.h"
#include "plic.h"
#include "sysctl.h"
#include "sleep.h"
#include "board.h"

/* PWM device 1 channel 0 is the TIMER1 toggle output 0 signal; any IO can carry it through the FPIOA. */
static int fade(void *ctx)
{
    static double duty = 0.0, step = 0.02;
    (void)ctx;
    duty += step;
    if (duty >= 1.0) { duty = 1.0; step = -step; }
    if (duty <= 0.0) { duty = 0.0; step = -step; }
    /* LED is active low: invert the duty */
    pwm_set_frequency(PWM_DEVICE_1, PWM_CHANNEL_0, 1000, 1.0 - duty);
    return 0;
}

int main(void)
{
    plic_init();
    fpioa_set_function(PIN_LED_G, FUNC_TIMER1_TOGGLE1);
    pwm_init(PWM_DEVICE_1);
    pwm_set_frequency(PWM_DEVICE_1, PWM_CHANNEL_0, 1000, 1.0);
    pwm_set_enable(PWM_DEVICE_1, PWM_CHANNEL_0, 1);

    timer_init(TIMER_DEVICE_0);      /* timer 0 (not used by PWM device 1) drives the ramp */
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 20 * 1000 * 1000);
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, fade, NULL);
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
    sysctl_enable_irq();
    printf("pwm-fade running\n");
    for (;;) msleep(1000);
}
