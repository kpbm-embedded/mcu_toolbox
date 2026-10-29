/* Blink one GPIO LED and print a heartbeat on the console UART. */
#include "bflb_gpio.h"
#include "bflb_mtimer.h"
#include "board.h"
#include "board_pins.h"

int main(void)
{
    board_init();

    struct bflb_device_s *gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_init(gpio, LED_PIN, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

    unsigned n = 0;
    while (1) {
        if (n & 1) {
            bflb_gpio_set(gpio, LED_PIN);
        } else {
            bflb_gpio_reset(gpio, LED_PIN);
        }
        printf("blink %u (LED_PIN=%d)\r\n", n++, LED_PIN);
        bflb_mtimer_delay_ms(500);
    }
}
