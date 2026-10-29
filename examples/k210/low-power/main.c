/* Low power on the K210 standalone SDK. The K210 has no sleep/standby/deep-sleep state and no RTC wake-up domain:
 * what software can do is (1) lower the CPU/bus clock (PLL0), (2) switch off PLL1 (KPU/AI clock) and gate unused
 * peripheral clocks, and (3) idle the core with WFI (core clock gated until an interrupt is pending).
 * Here: run at 100 MHz with PLL1 off and the core in WFI; TIMER0 wakes it every 2 s, the BOOT key wakes it at once. */
#include <stdio.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "timer.h"
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "encoding.h"
#include "board.h"

#define GPIO_LED 0
#define GPIO_KEY 1
static volatile unsigned timer_wakes, key_wakes;

static int on_timer(void *ctx) { (void)ctx; timer_wakes++; return 0; }
static int on_key(void *ctx) { (void)ctx; key_wakes++; return 0; }

static void show_clocks(const char *when)
{
    printf("low-power %s: cpu %u Hz, pll0 %u, pll1 %u, apb0 %u\n", when,
           (unsigned)sysctl_clock_get_freq(SYSCTL_CLOCK_CPU), (unsigned)sysctl_pll_get_freq(SYSCTL_PLL0),
           (unsigned)sysctl_pll_get_freq(SYSCTL_PLL1), (unsigned)sysctl_clock_get_freq(SYSCTL_CLOCK_APB0));
}

int main(void)
{
    show_clocks("boot");

    /* (1)+(2): KPU clock off, PLL1 off, CPU (and the AXI/APB buses derived from it) down to 100 MHz */
    sysctl_clock_disable(SYSCTL_CLOCK_AI);
    sysctl_pll_disable(SYSCTL_PLL1);
    sysctl_cpu_set_freq(100000000UL);
    uarths_init();                       /* UARTHS divisor is derived from the CPU clock: recompute it */
    show_clocks("reduced");

    plic_init();
    fpioa_set_function(PIN_LED_G, FUNC_GPIOHS0 + GPIO_LED);
    gpiohs_set_drive_mode(GPIO_LED, GPIO_DM_OUTPUT);
    gpiohs_set_pin(GPIO_LED, LED_OFF);
    fpioa_set_function(PIN_KEY, FUNC_GPIOHS0 + GPIO_KEY);
    gpiohs_set_drive_mode(GPIO_KEY, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_pin_edge(GPIO_KEY, GPIO_PE_FALLING);
    gpiohs_irq_register(GPIO_KEY, 1, on_key, NULL);

    timer_init(TIMER_DEVICE_0);          /* interval is computed from the (new) timer clock */
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 2000ULL * 1000 * 1000);   /* ns */
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, on_timer, NULL);
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
    sysctl_enable_irq();

    for (;;) {
        unsigned t = timer_wakes, k = key_wakes;
        uint64_t c0 = read_cycle();
        asm volatile("wfi");             /* (3) core idles here until the timer or key interrupt is pending */
        uint64_t slept = read_cycle() - c0;
        gpiohs_set_pin(GPIO_LED, LED_ON);
        printf("wake: %s (timer %u, key %u), mcycle advanced %lu during wfi\n",
               key_wakes != k ? "key" : (timer_wakes != t ? "timer" : "other"), timer_wakes, key_wakes,
               (unsigned long)slept);
        gpiohs_set_pin(GPIO_LED, LED_OFF);
    }
}
