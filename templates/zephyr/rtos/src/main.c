#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

struct sample { uint32_t seq; int64_t ts_ms; };
K_MSGQ_DEFINE(samples, sizeof(struct sample), 8, 4);
K_MUTEX_DEFINE(stats_lock);
static uint32_t received;

static void producer(void *a, void *b, void *c)
{
	for (uint32_t i = 0;; i++) {
		struct sample s = { i, k_uptime_get() };
		if (k_msgq_put(&samples, &s, K_NO_WAIT) != 0) {
			printk("queue full, dropping %u\n", i);
		}
		k_msleep(250);
	}
}

static void consumer(void *a, void *b, void *c)
{
	struct sample s;
	while (k_msgq_get(&samples, &s, K_FOREVER) == 0) {
		k_mutex_lock(&stats_lock, K_FOREVER);
		received++;
		k_mutex_unlock(&stats_lock);
		gpio_pin_toggle_dt(&led);
		printk("sample %u @ %lld ms\n", s.seq, s.ts_ms);
	}
}

K_THREAD_DEFINE(producer_tid, 1024, producer, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(consumer_tid, 1024, consumer, NULL, NULL, NULL, 6, 0, 0);

int main(void)
{
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	return 0;
}
