#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* led0 alias comes from the board devicetree */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
	if (!gpio_is_ready_dt(&led) || gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) < 0) {
		printk("led0 not available\n");
		return 0;
	}
	printk("{{NAME}} running on %s\n", CONFIG_BOARD_TARGET);
	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(500);
	}
}
