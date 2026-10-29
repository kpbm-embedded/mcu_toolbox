/* Low-level glue between the Cube USB Device core and HAL_PCD, per family (all on PA11 DM / PA12 DP):
 *   F411: OTG_FS (USB_OTG_FS), AF10, 48 MHz from PLLQ (SYSCLK 96 MHz, BOARD_USB48). VBUS sensing off (PA9 is the console TX).
 *   H743: OTG2_FS (USB_OTG_FS = USB2_OTG_FS), AF10_OTG2_FS, 48 MHz from PLL3Q (HSE 25 /5 x96 /10), USB regulator detector on.
 *   F103: USB FS device (FSDEV, packet memory), 48 MHz = PLL 72 / 1.5; D+ pull-up is a resistor on the board.
 *   G431: USB FS device (FSDEV), 48 MHz from PLLQ (VCO 288 / 6, board_clock_init), internal D+ pull-up. */
#include "usbd_core.h"
#include "usbd_cdc.h"

PCD_HandleTypeDef hpcd_usb_fs;

#if defined(BOARD_F4) || defined(BOARD_H7)
#define USB_INST USB_OTG_FS
#define USB_IRQn OTG_FS_IRQn
#define USB_IRQHandler OTG_FS_IRQHandler
#elif defined(BOARD_F1)
#define USB_INST USB
#define USB_IRQn USB_LP_CAN1_RX0_IRQn
#define USB_IRQHandler USB_LP_CAN1_RX0_IRQHandler
#elif defined(BOARD_G4)
#define USB_INST USB
#define USB_IRQn USB_LP_IRQn
#define USB_IRQHandler USB_LP_IRQHandler
#endif

static void usb_clock_init(void)
{
#if defined(BOARD_F1)
  RCC_PeriphCLKInitTypeDef p = {0};
  p.PeriphClockSelection = RCC_PERIPHCLK_USB; p.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  HAL_RCCEx_PeriphCLKConfig(&p);
#elif defined(BOARD_G4)
  __HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL_48M1CLK);
  RCC_PeriphCLKInitTypeDef p = {0};
  p.PeriphClockSelection = RCC_PERIPHCLK_USB; p.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  HAL_RCCEx_PeriphCLKConfig(&p);
#elif defined(BOARD_H7)
  RCC_PeriphCLKInitTypeDef p = {0};
  p.PeriphClockSelection = RCC_PERIPHCLK_USB;
  p.PLL3.PLL3M = 5; p.PLL3.PLL3N = 96; p.PLL3.PLL3P = 2; p.PLL3.PLL3Q = 10; p.PLL3.PLL3R = 2;
  p.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2; p.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE; p.PLL3.PLL3FRACN = 0;
  p.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
  HAL_RCCEx_PeriphCLKConfig(&p);
  HAL_PWREx_EnableUSBVoltageDetector();
#endif   /* F4: PLLQ already 48 MHz via BOARD_USB48 */
}

void HAL_PCD_MspInit(PCD_HandleTypeDef *h)
{
  if (h->Instance != USB_INST) return;
  __HAL_RCC_GPIOA_CLK_ENABLE();
#if defined(BOARD_F4) || defined(BOARD_H7)
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_11 | GPIO_PIN_12, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL,
                         .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
#if defined(BOARD_F4)
                         .Alternate = GPIO_AF10_OTG_FS };
#else
                         .Alternate = GPIO_AF10_OTG2_FS };
#endif
  HAL_GPIO_Init(GPIOA, &g);
#endif   /* FSDEV (F1/G4): PA11/PA12 are taken over by the USB block, no AF setting */
#if defined(BOARD_F4)
  __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
#elif defined(BOARD_H7)
  __HAL_RCC_USB2_OTG_FS_CLK_ENABLE();
#else
  __HAL_RCC_USB_CLK_ENABLE();
#endif
  HAL_NVIC_SetPriority(USB_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USB_IRQn);
}
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *h) { if (h->Instance == USB_INST) HAL_NVIC_DisableIRQ(USB_IRQn); }
void USB_IRQHandler(void) { HAL_PCD_IRQHandler(&hpcd_usb_fs); }

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *h) { USBD_LL_SetupStage(h->pData, (uint8_t *)h->Setup); }
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *h, uint8_t ep) { USBD_LL_DataOutStage(h->pData, ep, h->OUT_ep[ep].xfer_buff); }
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *h, uint8_t ep) { USBD_LL_DataInStage(h->pData, ep, h->IN_ep[ep].xfer_buff); }
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *h) { USBD_LL_SOF(h->pData); }
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *h) { USBD_LL_SetSpeed(h->pData, USBD_SPEED_FULL); USBD_LL_Reset(h->pData); }
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *h) { USBD_LL_Suspend(h->pData); }
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *h) { USBD_LL_Resume(h->pData); }
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *h, uint8_t ep) { USBD_LL_IsoOUTIncomplete(h->pData, ep); }
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *h, uint8_t ep) { USBD_LL_IsoINIncomplete(h->pData, ep); }
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *h) { USBD_LL_DevConnected(h->pData); }
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *h) { USBD_LL_DevDisconnected(h->pData); }

#if defined(BOARD_F1)   /* older USB Device library in the Cube F1 pack: 16-bit transfer sizes */
typedef uint16_t xfer_len_t;
#else
typedef uint32_t xfer_len_t;
#endif

static USBD_StatusTypeDef st(HAL_StatusTypeDef s) { return s == HAL_OK ? USBD_OK : (s == HAL_BUSY ? USBD_BUSY : USBD_FAIL); }

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  usb_clock_init();
#if defined(BOARD_F1)
  /* BluePill: D+ has a fixed pull-up; pull PA12 low briefly so the host re-enumerates after a reset/flash */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_12, .Mode = GPIO_MODE_OUTPUT_PP, .Speed = GPIO_SPEED_FREQ_LOW };
  HAL_GPIO_Init(GPIOA, &g);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);
#endif
  hpcd_usb_fs.pData = pdev; pdev->pData = &hpcd_usb_fs;
  hpcd_usb_fs.Instance = USB_INST;
  hpcd_usb_fs.Init.speed = PCD_SPEED_FULL; hpcd_usb_fs.Init.low_power_enable = DISABLE; hpcd_usb_fs.Init.lpm_enable = DISABLE;
  hpcd_usb_fs.Init.battery_charging_enable = DISABLE; hpcd_usb_fs.Init.Sof_enable = DISABLE;
#if defined(BOARD_F4) || defined(BOARD_H7)
#if defined(BOARD_F4)
  hpcd_usb_fs.Init.dev_endpoints = 4;
#else
  hpcd_usb_fs.Init.dev_endpoints = 9;
#endif
  hpcd_usb_fs.Init.dma_enable = DISABLE;
  hpcd_usb_fs.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_usb_fs.Init.vbus_sensing_enable = DISABLE; hpcd_usb_fs.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_usb_fs) != HAL_OK) return USBD_FAIL;
  HAL_PCDEx_SetRxFiFo(&hpcd_usb_fs, 0x40);          /* 320 words total: rx 64 + tx0 32 + tx1 128 + tx2 32 */
  HAL_PCDEx_SetTxFiFo(&hpcd_usb_fs, 0, 0x20);
  HAL_PCDEx_SetTxFiFo(&hpcd_usb_fs, 1, 0x80);
  HAL_PCDEx_SetTxFiFo(&hpcd_usb_fs, 2, 0x20);
#else
  hpcd_usb_fs.Init.dev_endpoints = 8; hpcd_usb_fs.Init.phy_itface = PCD_PHY_EMBEDDED;
  if (HAL_PCD_Init(&hpcd_usb_fs) != HAL_OK) return USBD_FAIL;
  /* packet memory: buffer table at 0, EP0 OUT/IN, CDC data OUT 0x01 / IN 0x81, CDC command IN 0x82 */
  HAL_PCDEx_PMAConfig(&hpcd_usb_fs, 0x00, PCD_SNG_BUF, 0x18);
  HAL_PCDEx_PMAConfig(&hpcd_usb_fs, 0x80, PCD_SNG_BUF, 0x58);
  HAL_PCDEx_PMAConfig(&hpcd_usb_fs, 0x81, PCD_SNG_BUF, 0xC0);
  HAL_PCDEx_PMAConfig(&hpcd_usb_fs, 0x01, PCD_SNG_BUF, 0x110);
  HAL_PCDEx_PMAConfig(&hpcd_usb_fs, 0x82, PCD_SNG_BUF, 0x100);
#endif
  return USBD_OK;
}
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev) { return st(HAL_PCD_DeInit(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev) { return st(HAL_PCD_Start(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev) { return st(HAL_PCD_Stop(pdev->pData)); }
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t type, uint16_t mps) { return st(HAL_PCD_EP_Open(pdev->pData, ep, mps, type)); }
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep) { return st(HAL_PCD_EP_Close(pdev->pData, ep)); }
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep) { return st(HAL_PCD_EP_Flush(pdev->pData, ep)); }
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep) { return st(HAL_PCD_EP_SetStall(pdev->pData, ep)); }
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep) { return st(HAL_PCD_EP_ClrStall(pdev->pData, ep)); }
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep)
{
  PCD_HandleTypeDef *h = pdev->pData;
  return (ep & 0x80U) ? h->IN_ep[ep & 0x7FU].is_stall : h->OUT_ep[ep & 0x7FU].is_stall;
}
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t addr) { return st(HAL_PCD_SetAddress(pdev->pData, addr)); }
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t *buf, xfer_len_t size) { return st(HAL_PCD_EP_Transmit(pdev->pData, ep, buf, size)); }
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t *buf, xfer_len_t size) { return st(HAL_PCD_EP_Receive(pdev->pData, ep, buf, size)); }
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep) { return HAL_PCD_EP_GetRxCount(pdev->pData, ep); }
void USBD_LL_Delay(uint32_t ms) { HAL_Delay(ms); }

void *USBD_static_malloc(uint32_t size)
{
  static uint32_t mem[(sizeof(USBD_CDC_HandleTypeDef) / 4U) + 1U];
  (void)size;
  return mem;
}
void USBD_static_free(void *p) { (void)p; }
