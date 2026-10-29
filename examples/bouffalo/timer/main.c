/* Hardware timer0 compare interrupt every 100 ms; main loop prints the tick count once a second. */
#include "bflb_timer.h"
#include "bflb_mtimer.h"
#include "bflb_irq.h"
#include "board.h"

static struct bflb_device_s *timer0;
static volatile uint32_t ticks;

static void timer0_isr(int irq, void *arg)
{
    if (bflb_timer_get_compint_status(timer0, TIMER_COMP_ID_0)) {
        bflb_timer_compint_clear(timer0, TIMER_COMP_ID_0);
        ticks++;
    }
}

int main(void)
{
    board_init();

    /* timer clock = XCLK / (div + 1): bl616/bl808 40 MHz / 40 = 1 MHz, bl702 32 MHz / 32 = 1 MHz (SDK timer_int example:
     * "for bl616/bl808/bl606p is 39, for bl702 is 31") */
    struct bflb_timer_config_s cfg = {
        .counter_mode = TIMER_COUNTER_MODE_PROLOAD,
        .clock_source = TIMER_CLKSRC_XTAL,
#if defined(BL702)
        .clock_div = 31,
#else
        .clock_div = 39,
#endif
        .trigger_comp_id = TIMER_COMP_ID_0,
        .comp0_val = 100000, /* 100 ms at 1 MHz */
        .comp1_val = 0xFFFFFFFF,
        .comp2_val = 0xFFFFFFFF,
        .preload_val = 0,
    };

    timer0 = bflb_device_get_by_name("timer0");
    bflb_timer_init(timer0, &cfg);
    bflb_irq_attach(timer0->irq_num, timer0_isr, NULL);
    bflb_irq_enable(timer0->irq_num);
    bflb_timer_start(timer0);

    while (1) {
        bflb_mtimer_delay_ms(1000);
        printf("timer ticks: %u (expect ~10 per second)\r\n", (unsigned)ticks);
        ticks = 0;
    }
}
