/* USART1 echo with DMA: RX idle-line framed (HAL_UARTEx_ReceiveToIdle_DMA), TX via DMA. Request mapping per family:
 *   F411: DMA2 Stream2 ch4 (RX), DMA2 Stream7 ch4 (TX)      - RM0383 DMA2 request mapping
 *   F103: DMA1 Channel5 (RX), DMA1 Channel4 (TX)            - RM0008 DMA1 request mapping (fixed)
 *   G431: DMA1 Channel1 (RX), DMA1 Channel2 (TX) via DMAMUX - DMA_REQUEST_USART1_RX/TX
 *   H743: DMA1 Stream0 (RX), DMA1 Stream1 (TX) via DMAMUX   - buffers in AXI SRAM (reachable by DMA1), D-cache off */
#include <string.h>
#include "board.h"

#if defined(BOARD_F4)
#define RX_DMA DMA2_Stream2
#define TX_DMA DMA2_Stream7
#define RX_IRQn DMA2_Stream2_IRQn
#define TX_IRQn DMA2_Stream7_IRQn
#define RX_IRQHandler DMA2_Stream2_IRQHandler
#define TX_IRQHandler DMA2_Stream7_IRQHandler
#define DMA_CLK_EN() __HAL_RCC_DMA2_CLK_ENABLE()
#elif defined(BOARD_F1)
#define RX_DMA DMA1_Channel5
#define TX_DMA DMA1_Channel4
#define RX_IRQn DMA1_Channel5_IRQn
#define TX_IRQn DMA1_Channel4_IRQn
#define RX_IRQHandler DMA1_Channel5_IRQHandler
#define TX_IRQHandler DMA1_Channel4_IRQHandler
#define DMA_CLK_EN() __HAL_RCC_DMA1_CLK_ENABLE()
#elif defined(BOARD_G4)
#define RX_DMA DMA1_Channel1
#define TX_DMA DMA1_Channel2
#define RX_IRQn DMA1_Channel1_IRQn
#define TX_IRQn DMA1_Channel2_IRQn
#define RX_IRQHandler DMA1_Channel1_IRQHandler
#define TX_IRQHandler DMA1_Channel2_IRQHandler
#define DMA_CLK_EN() do { __HAL_RCC_DMAMUX1_CLK_ENABLE(); __HAL_RCC_DMA1_CLK_ENABLE(); } while (0)
#elif defined(BOARD_H7)
#define RX_DMA DMA1_Stream0
#define TX_DMA DMA1_Stream1
#define RX_IRQn DMA1_Stream0_IRQn
#define TX_IRQn DMA1_Stream1_IRQn
#define RX_IRQHandler DMA1_Stream0_IRQHandler
#define TX_IRQHandler DMA1_Stream1_IRQHandler
#define DMA_CLK_EN() __HAL_RCC_DMA1_CLK_ENABLE()
#endif

static DMA_HandleTypeDef hdma_rx, hdma_tx;
static uint8_t rxbuf[64], txbuf[64];
static volatile uint16_t rxlen;
static volatile uint8_t tx_busy;
static uint32_t dropped;

void RX_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_rx); }
void TX_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_tx); }
void USART1_IRQHandler(void) { HAL_UART_IRQHandler(&board_uart); }

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *h, uint16_t size)
{
  if (h != &board_uart) return;
  if (!tx_busy) { memcpy(txbuf, rxbuf, size); tx_busy = 1; HAL_UART_Transmit_DMA(h, txbuf, size); } else dropped += size;
  rxlen = size;
  HAL_UARTEx_ReceiveToIdle_DMA(h, rxbuf, sizeof rxbuf);          /* re-arm */
  __HAL_DMA_DISABLE_IT(&hdma_rx, DMA_IT_HT);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *h) { if (h == &board_uart) tx_busy = 0; }

static void dma_init(DMA_HandleTypeDef *d, void *inst, uint32_t req, uint32_t dir)
{
  d->Instance = inst;
#if defined(BOARD_F4)
  d->Init.Channel = req; d->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
#elif defined(BOARD_G4)
  d->Init.Request = req;
#elif defined(BOARD_H7)
  d->Init.Request = req; d->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
#else
  (void)req;
#endif
  d->Init.Direction = dir; d->Init.PeriphInc = DMA_PINC_DISABLE; d->Init.MemInc = DMA_MINC_ENABLE;
  d->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; d->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  d->Init.Mode = DMA_NORMAL; d->Init.Priority = DMA_PRIORITY_LOW;
  HAL_DMA_Init(d);
}

int main(void)
{
  board_init();
  printf("dma-uart on %s: type something, it is echoed through DMA\r\n", BOARD_NAME);
  DMA_CLK_EN();
#if defined(BOARD_F4)
  dma_init(&hdma_rx, RX_DMA, DMA_CHANNEL_4, DMA_PERIPH_TO_MEMORY);
  dma_init(&hdma_tx, TX_DMA, DMA_CHANNEL_4, DMA_MEMORY_TO_PERIPH);
#elif defined(BOARD_G4) || defined(BOARD_H7)
  dma_init(&hdma_rx, RX_DMA, DMA_REQUEST_USART1_RX, DMA_PERIPH_TO_MEMORY);
  dma_init(&hdma_tx, TX_DMA, DMA_REQUEST_USART1_TX, DMA_MEMORY_TO_PERIPH);
#else
  dma_init(&hdma_rx, RX_DMA, 0, DMA_PERIPH_TO_MEMORY);
  dma_init(&hdma_tx, TX_DMA, 0, DMA_MEMORY_TO_PERIPH);
#endif
  __HAL_LINKDMA(&board_uart, hdmarx, hdma_rx);
  __HAL_LINKDMA(&board_uart, hdmatx, hdma_tx);
  HAL_NVIC_SetPriority(RX_IRQn, 5, 0);     HAL_NVIC_EnableIRQ(RX_IRQn);
  HAL_NVIC_SetPriority(TX_IRQn, 5, 0);     HAL_NVIC_EnableIRQ(TX_IRQn);
  HAL_NVIC_SetPriority(USART1_IRQn, 5, 0); HAL_NVIC_EnableIRQ(USART1_IRQn);
  HAL_UARTEx_ReceiveToIdle_DMA(&board_uart, rxbuf, sizeof rxbuf);
  __HAL_DMA_DISABLE_IT(&hdma_rx, DMA_IT_HT);
  for (;;) {
    if (rxlen) { rxlen = 0; board_led_toggle(); }
    __WFI();
  }
}
