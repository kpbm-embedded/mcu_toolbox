#include "board.h"
#include "py32f0xx_ll_dma.h"
#include <string.h>

/* DMA1 channel 1 memory-to-memory copy with transfer-complete interrupt; the CPU then compares the buffers.
 * PY32F003/F030 have a 3-channel DMA (datasheet); PY32F002A has none. */
#define N 256
static uint32_t src[N], dst[N];
static volatile uint8_t done;

void DMA1_Channel1_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC1(DMA1)) { LL_DMA_ClearFlag_TC1(DMA1); done = 1; }
    if (LL_DMA_IsActiveFlag_TE1(DMA1)) { LL_DMA_ClearFlag_TE1(DMA1); done = 2; }
}

int main(void)
{
    BSP_RCC_HSI_24MConfig();
    BSP_USART_Config(115200);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    NVIC_SetPriority(DMA1_Channel1_IRQn, 1);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    for (uint32_t round = 1;; round++) {
        for (uint32_t i = 0; i < N; i++) { src[i] = i * 2654435761u + round; dst[i] = 0; }
        done = 0;
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
        LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_MEMORY_TO_MEMORY | LL_DMA_MODE_NORMAL |
                              LL_DMA_PERIPH_INCREMENT | LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_WORD |
                              LL_DMA_MDATAALIGN_WORD | LL_DMA_PRIORITY_HIGH);
        LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_1, (uint32_t)src, (uint32_t)dst, LL_DMA_DIRECTION_MEMORY_TO_MEMORY);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, N);
        LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
        LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
        while (!done) {}
        printf("dma round %lu: %s, %s\r\n", (unsigned long)round, done == 1 ? "complete" : "transfer error",
               memcmp(src, dst, sizeof src) == 0 ? "buffers match" : "MISMATCH");
        LL_mDelay(1000);
    }
}
