#include "board.h"
#include "pico/multicore.h"
#include <stdio.h>
static void core1_entry(void) {
    while (true) {
        uint32_t v = multicore_fifo_pop_blocking();
        multicore_fifo_push_blocking(v * v);
    }
}
int main(void) {
    stdio_init_all();
    multicore_launch_core1(core1_entry);
    for (uint32_t i = 1;; i++) {
        multicore_fifo_push_blocking(i);
        uint32_t r = multicore_fifo_pop_blocking();
        printf("core0 sent %u, core1 replied %u\n", (unsigned)i, (unsigned)r);
        sleep_ms(500);
    }
}
