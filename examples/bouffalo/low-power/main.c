/* Deep sleep (HBN = hibernate, level 0) with RTC wake-up, SDK 2.3 PM API (drivers/soc/<chip>/std/include/<chip>_pm.h),
 * pattern from bouffalo_sdk examples/pmu/<chip>/hbn_rtc. Each boot: blink the LED 3x, print, then hibernate 5 s.
 * Wake-up from HBN is a reset: RAM is lost and main() starts again. */
#include "bflb_gpio.h"
#include "bflb_mtimer.h"
#include "board.h"
#include "board_pins.h"
#if defined(BL702)
#include "bl702_pm.h"
#else
#include "bl616_pm.h"
#endif

#define SLEEP_S 5

int main(void)
{
    board_init();

    struct bflb_device_s *gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_init(gpio, LED_PIN, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    for (int i = 0; i < 6; i++) {
        if (i & 1) {
            bflb_gpio_reset(gpio, LED_PIN);
        } else {
            bflb_gpio_set(gpio, LED_PIN);
        }
        bflb_mtimer_delay_ms(150);
    }

    printf("awake; entering HBN level 0 for %d s (RTC wake-up = reset)\r\n", SLEEP_S);
    bflb_mtimer_delay_ms(100); /* let the UART drain */

    pm_pds_mask_all_wakeup_src();
#if defined(BL702)
    pm_hbn_mask_all_wakeup_src();
#endif
    pm_hbn_mode_enter(PM_HBN_LEVEL_0, 32768 * SLEEP_S); /* RTC runs from the 32.768 kHz clock */

    while (1) {
    }
}
