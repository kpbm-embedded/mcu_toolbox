/* DMA1 channel 1 memory-to-memory copy (1 KB, V003: 256 B; 32-bit words), polled for transfer complete, then checked with memcmp.
 * Register usage as in ch32fun examples/dma_gpio_mem2mem (MEM2MEM: "peripheral" address = source). */
#include "board.h"
#include <string.h>

#if defined(BOARD_CH32V003)
#define N 64      /* 2 KB SRAM */
#else
#define N 256
#endif
static uint32_t src[N], dst[N];

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_INIT();
#if defined(BOARD_CH32H417)
    RCC->HBPCENR |= RCC_HBPeriph_DMA1;
#else
    RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
#endif
    Delay_Ms(100);
    printf("dma (%s): DMA1 channel 1 mem2mem\r\n", BOARD_STR);
    for (uint32_t round = 1;; round++) {
        for (uint32_t i = 0; i < N; i++) { src[i] = i * 2654435761u + round; dst[i] = 0; }
        DMA1_Channel1->CFGR = 0;
        DMA1->INTFCR = DMA1_FLAG_GL1;
        DMA1_Channel1->PADDR = (uint32_t)src;
        DMA1_Channel1->MADDR = (uint32_t)dst;
        DMA1_Channel1->CNTR = N;
        DMA1_Channel1->CFGR = DMA_CFGR1_MEM2MEM | DMA_CFGR1_PL | DMA_CFGR1_MSIZE_1 | DMA_CFGR1_PSIZE_1 |
                              DMA_CFGR1_MINC | DMA_CFGR1_PINC;
        DMA1_Channel1->CFGR |= DMA_CFGR1_EN;
        uint32_t spins = 0;
        while (!(DMA1->INTFR & (DMA1_FLAG_TC1 | DMA1_FLAG_TE1)) && ++spins < 1000000) {}
        int ok = (DMA1->INTFR & DMA1_FLAG_TC1) && memcmp(src, dst, sizeof src) == 0;
        DMA1_Channel1->CFGR = 0;
        LED_SET(round & 1);
        printf("round %lu: %s\r\n", (unsigned long)round, ok ? "copy OK" : "FAILED");
        Delay_Ms(1000);
    }
}
