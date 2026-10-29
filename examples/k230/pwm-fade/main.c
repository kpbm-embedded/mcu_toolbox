/* PWM fade: 1 kHz PWM, duty ramped 0..100 % and back (drv_pwm). Connect an LED (with resistor) to the PWM pin. */
#include <stdio.h>
#include <unistd.h>
#include "drv_fpioa.h"
#include "drv_pwm.h"
#include "board.h"

int main(void)
{
    if (drv_fpioa_set_pin_func(PWM_GPIO, PWM0 + PWM_CH)) { printf("cannot route PWM%d to IO%d\n", PWM_CH, PWM_GPIO); return 1; }
    if (drv_pwm_init() || drv_pwm_set_freq(PWM_CH, 1000) || drv_pwm_set_duty(PWM_CH, 0) || drv_pwm_enable(PWM_CH)) {
        printf("pwm%d setup failed\n", PWM_CH);
        return 1;
    }
    printf("pwm-fade (%s): PWM%d on IO%d, 1 kHz, 5 breaths\n", BOARD_NAME, PWM_CH, PWM_GPIO);
    for (int n = 0; n < 5; n++) {
        for (int d = 0; d <= 100; d += 2) { drv_pwm_set_duty(PWM_CH, d); usleep(20 * 1000); }
        for (int d = 100; d >= 0; d -= 2) { drv_pwm_set_duty(PWM_CH, d); usleep(20 * 1000); }
        printf("breath %d\n", n + 1);
    }
    drv_pwm_disable(PWM_CH);
    drv_pwm_deinit();
    return 0;
}
