#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "blink_logic.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
	if (!gpio_is_ready_dt(&led) || gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) < 0) { return 0; }
	for (uint32_t level = 0;; level = (level + 1) % 20) {
		gpio_pin_toggle_dt(&led);
		printk("{{NAME}} level %u\n", level);
		k_msleep(blink_period_ms(level));
	}
}
