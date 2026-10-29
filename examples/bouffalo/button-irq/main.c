/* BOOT key (BTN_PIN, active low) raises a falling-edge GPIO interrupt that toggles the LED. */
#include "bflb_gpio.h"
#include "bflb_irq.h"
#include "bflb_mtimer.h"
#include "board.h"
#include "board_pins.h"

static struct bflb_device_s *gpio;
static volatile uint32_t presses;
static volatile bool led_on;

static void btn_isr(uint8_t pin)
{
    if (pin == BTN_PIN) {
        presses++;
        led_on = !led_on;
        if (led_on) {
            bflb_gpio_set(gpio, LED_PIN);
        } else {
            bflb_gpio_reset(gpio, LED_PIN);
        }
    }
}

int main(void)
{
    board_init();
    gpio = bflb_device_get_by_name("gpio");

    bflb_gpio_init(gpio, LED_PIN, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_init(gpio, BTN_PIN, GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN);

    bflb_irq_disable(gpio->irq_num);
    bflb_gpio_int_init(gpio, BTN_PIN, GPIO_INT_TRIG_MODE_SYNC_FALLING_EDGE);
    bflb_gpio_irq_attach(BTN_PIN, btn_isr);
    bflb_irq_enable(gpio->irq_num);

    uint32_t shown = 0;
    while (1) {
        if (presses != shown) {
            shown = presses;
            printf("button presses: %u\r\n", (unsigned)shown);
        }
        bflb_mtimer_delay_ms(20);
    }
}
