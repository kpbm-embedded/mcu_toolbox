/* DMAC memory-to-memory copy on the K210 standalone SDK.
 * Buffers come from iomem_malloc() (the uncached 0x40000000 alias of SRAM), so the DMAC reads/writes them directly;
 * with ordinary cached buffers (0x80000000) the SDK's FIX_CACHE path copies through a bounce buffer instead. */
#include <stdio.h>
#include <string.h>
#include "dmac.h"
#include "iomem.h"
#include "plic.h"
#include "sysctl.h"
#include "board.h"

#define WORDS 4096                     /* 16 KiB per transfer */

static volatile int dma_done;

static int on_dma_done(void *ctx)
{
    (void)ctx;
    dma_done = 1;                      /* never printf inside the ISR */
    return 0;
}

int main(void)
{
    plic_init();
    dmac_init();
    sysctl_enable_irq();

    uint32_t *src = iomem_malloc(WORDS * sizeof(uint32_t));
    uint32_t *dst = iomem_malloc(WORDS * sizeof(uint32_t));
    if (!src || !dst) { printf("dma: iomem_malloc failed\n"); for (;;) {} }
    for (uint32_t i = 0; i < WORDS; i++) { src[i] = 0xA5000000u ^ (i * 2654435761u); dst[i] = 0; }
    printf("dma: src=%p dst=%p, %u words\n", (void *)src, (void *)dst, WORDS);

    /* 1) polled transfer */
    uint64_t t0 = sysctl_get_time_us();
    dmac_set_single_mode(DMAC_CHANNEL0, src, dst, DMAC_ADDR_INCREMENT, DMAC_ADDR_INCREMENT,
                         DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, WORDS);
    dmac_wait_done(DMAC_CHANNEL0);
    uint64_t t_dma = sysctl_get_time_us() - t0;
    printf("dma: polled copy %s in %u us\n", memcmp(src, dst, WORDS * 4) ? "MISMATCH" : "ok", (unsigned)t_dma);

    /* 2) interrupt-signalled transfer on another channel */
    memset(dst, 0, WORDS * 4);
    dmac_irq_register(DMAC_CHANNEL1, on_dma_done, NULL, 1);
    dmac_set_single_mode(DMAC_CHANNEL1, src, dst, DMAC_ADDR_INCREMENT, DMAC_ADDR_INCREMENT,
                         DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, WORDS);
    while (!dma_done) { }              /* the CPU is free to do other work here */
    dmac_wait_done(DMAC_CHANNEL1);
    printf("dma: irq copy %s\n", memcmp(src, dst, WORDS * 4) ? "MISMATCH" : "ok");

    /* 3) CPU memcpy for comparison */
    t0 = sysctl_get_time_us();
    memcpy(dst, src, WORDS * 4);
    printf("dma: cpu memcpy (uncached) took %u us\n", (unsigned)(sysctl_get_time_us() - t0));

    iomem_free(src);
    iomem_free(dst);
    for (;;) { }
}
