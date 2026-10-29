/* Memory-to-memory DMA on dma0_ch0 with a completion interrupt; compares the result with the CPU and times both.
 * API and cache handling as in the SDK example examples/peripherals/dma/dma_normal (both SDK 2.3 and legacy 2.2.0). */
#include <string.h>
#include "bflb_mtimer.h"
#include "bflb_dma.h"
#include "bflb_l1c.h"
#include "board.h"

#define LEN 4096

static __attribute((aligned(32))) uint8_t src[LEN];
static __attribute((aligned(32))) uint8_t dst[LEN];
static struct bflb_dma_channel_lli_pool_s lli[4]; /* one LLI moves up to 4064 bytes */
static volatile int done;

static void dma_isr(void *arg)
{
    (void)arg;
    done = 1;
}

int main(void)
{
    board_init();
    struct bflb_device_s *ch = bflb_device_get_by_name("dma0_ch0");

    struct bflb_dma_channel_config_s cfg = {
        .direction = DMA_MEMORY_TO_MEMORY,
        .src_req = 0,
        .dst_req = 0,
        .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
        .src_width = DMA_DATA_WIDTH_32BIT,
        .dst_width = DMA_DATA_WIDTH_32BIT,
    };
    bflb_dma_channel_init(ch, &cfg);
    bflb_dma_channel_irq_attach(ch, dma_isr, NULL);

    for (unsigned round = 1;; round++) {
        for (int i = 0; i < LEN; i++) {
            src[i] = (uint8_t)(i * 7 + round);
        }
        memset(dst, 0, LEN);
        bflb_l1c_dcache_clean_range(src, LEN);
        bflb_l1c_dcache_clean_range(dst, LEN);

        struct bflb_dma_channel_lli_transfer_s xfer = { .src_addr = (uint32_t)(uintptr_t)src,
                                                        .dst_addr = (uint32_t)(uintptr_t)dst,
                                                        .nbytes = LEN };
        done = 0;
        uint64_t t0 = bflb_mtimer_get_time_us();
        bflb_dma_channel_lli_reload(ch, lli, 4, &xfer, 1);
        bflb_dma_channel_start(ch);
        while (!done) {
        }
        uint64_t t_dma = bflb_mtimer_get_time_us() - t0;
        bflb_l1c_dcache_invalidate_range(dst, LEN);

        int bad = memcmp(src, dst, LEN) != 0;
        t0 = bflb_mtimer_get_time_us();
        memcpy(dst, src, LEN);
        uint64_t t_cpu = bflb_mtimer_get_time_us() - t0;

        printf("round %u: DMA %d bytes in %d us (%s), memcpy %d us\r\n", round, LEN, (int)t_dma, bad ? "MISMATCH" : "ok",
               (int)t_cpu);
        bflb_mtimer_delay_ms(2000);
    }
}
