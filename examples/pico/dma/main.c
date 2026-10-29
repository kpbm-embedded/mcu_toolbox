#include "board.h"
#include "hardware/dma.h"
#include <stdio.h>
#include <string.h>
#define N 4096
static uint32_t src[N], dst[N];
int main(void) {
    stdio_init_all();
    for (int i = 0; i < N; i++) src[i] = i * 2654435761u;
    int ch = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(ch);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);
    while (true) {
        memset(dst, 0, sizeof dst);
        uint32_t t0 = time_us_32();
        dma_channel_configure(ch, &c, dst, src, N, true);
        dma_channel_wait_for_finish_blocking(ch);
        uint32_t dt = time_us_32() - t0;
        printf("DMA copied %d words in %u us, %s\n", N, (unsigned)dt, memcmp(src, dst, sizeof src) ? "MISMATCH" : "verified");
        sleep_ms(1000);
    }
}
