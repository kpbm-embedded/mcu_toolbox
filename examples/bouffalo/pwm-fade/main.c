/* Fade the duty cycle of PWM channel 0 up and down at 1 kHz.
 * Output pins are whatever board_pwm_gpio_init() routes in the SDK board file:
 *   bl616: PWM_V2 CH0 on GPIO24 (SDK bsp/board/bl616dk/board_gpio.c), bl702: PWM_V1 CH0 on GPIO0 (bsp/board/bl702dk/board.c),
 *   bl808 (legacy SDK, M0): PWM_V2 CH0 on GPIO24 (bouffalo_sdk_legacy/bsp/board/bl808dk/board.c; channel unverified).
 * Wire an LED (+ resistor) to that pin to see the fade. */
#include "bflb_mtimer.h"
#include "bflb_clock.h"
#include "board.h"
#if defined(BL702)
#include "bflb_pwm_v1.h"
#else
#include "bflb_pwm_v2.h"
#endif

#define PERIOD 1000

int main(void)
{
    board_init();
    board_pwm_gpio_init();

#if defined(BL702)
    struct bflb_device_s *pwm = bflb_device_get_by_name("pwm_v1");
    struct bflb_pwm_v1_channel_config_s cfg = { .clk_source = BFLB_SYSTEM_XCLK, .clk_div = 32, .period = PERIOD };
    bflb_pwm_v1_channel_init(pwm, PWM_CH0, &cfg);
    bflb_pwm_v1_channel_set_threshold(pwm, PWM_CH0, 0, 0);
    bflb_pwm_v1_start(pwm, PWM_CH0);
#else
    struct bflb_device_s *pwm = bflb_device_get_by_name("pwm_v2_0");
    struct bflb_pwm_v2_config_s cfg = { .clk_source = BFLB_SYSTEM_XCLK, .clk_div = 40, .period = PERIOD };
#if defined(PWM_CMD_IO_SEL) /* SDK 2.3 (bl616); the legacy SDK used for bl808 has no IO_SEL command */
    bflb_pwm_v2_feature_control(pwm, PWM_CMD_IO_SEL, PWM_IO_SEL_SINGLE_END);
#endif
    bflb_pwm_v2_init(pwm, &cfg);
    bflb_pwm_v2_channel_set_threshold(pwm, PWM_CH0, 0, 0);
    bflb_pwm_v2_channel_positive_start(pwm, PWM_CH0);
    bflb_pwm_v2_start(pwm);
#endif

    int duty = 0, step = 10;
    while (1) {
#if defined(BL702)
        bflb_pwm_v1_channel_set_threshold(pwm, PWM_CH0, 0, duty);
#else
        bflb_pwm_v2_channel_set_threshold(pwm, PWM_CH0, 0, duty);
#endif
        duty += step;
        if (duty >= PERIOD || duty <= 0) {
            step = -step;
            printf("pwm duty turnaround at %d/%d\r\n", duty, PERIOD);
        }
        bflb_mtimer_delay_ms(10);
    }
}
