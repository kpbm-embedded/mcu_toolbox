#include <stdio.h>
#include <stdint.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "atomic.h"
#include "encoding.h"
#include "entry.h"
#include "sleep.h"
#include "board.h"

static corelock_t lock = CORELOCK_INIT;
static volatile uint64_t count[2];

static int core1_main(void *ctx)
{
    (void)ctx;
    fpioa_set_function(PIN_LED_B, FUNC_GPIOHS1);
    gpiohs_set_drive_mode(1, GPIO_DM_OUTPUT);
    for (;;) {
        corelock_lock(&lock);
        count[1]++;
        corelock_unlock(&lock);
        gpiohs_set_pin(1, (count[1] & 1) ? LED_ON : LED_OFF);
        msleep(250);
    }
    return 0;
}

int main(void)
{
    fpioa_set_function(PIN_LED_R, FUNC_GPIOHS0);
    gpiohs_set_drive_mode(0, GPIO_DM_OUTPUT);
    register_core1(core1_main, NULL);   /* core 1 starts here; the SDK parks it until then */
    for (;;) {
        corelock_lock(&lock);
        count[0]++;
        uint64_t a = count[0], b = count[1];
        corelock_unlock(&lock);
        gpiohs_set_pin(0, (a & 1) ? LED_ON : LED_OFF);
        printf("core %d: c0=%lu c1=%lu\n", (int)current_coreid(), (unsigned long)a, (unsigned long)b);
        msleep(500);
    }
}
