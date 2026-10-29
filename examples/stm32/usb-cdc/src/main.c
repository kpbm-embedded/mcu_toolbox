/* USB CDC-ACM virtual COM port (Cube USB Device middleware of the board's Cube pack): everything received is echoed back; the
 * byte count is also printed on the USART1 console. USB peripheral and 48 MHz clock per family: see usbd_conf.c.
 * F411: SYSCLK is 96 MHz so that PLLQ gives the 48 MHz USB clock (BOARD_USB48). */
#include "board.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

static USBD_HandleTypeDef hUsb;
static uint8_t rxbuf[CDC_DATA_FS_MAX_PACKET_SIZE], txbuf[CDC_DATA_FS_MAX_PACKET_SIZE];
static volatile uint32_t total_rx;

static int8_t cdc_init(void) { USBD_CDC_SetTxBuffer(&hUsb, txbuf, 0); USBD_CDC_SetRxBuffer(&hUsb, rxbuf); return USBD_OK; }
static int8_t cdc_deinit(void) { return USBD_OK; }
static int8_t cdc_control(uint8_t cmd, uint8_t *pbuf, uint16_t len)
{
  static uint8_t line[7] = { 0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08 };   /* 115200 8N1 reported to the host */
  (void)len;
  if (cmd == CDC_SET_LINE_CODING) memcpy(line, pbuf, 7);
  else if (cmd == CDC_GET_LINE_CODING) memcpy(pbuf, line, 7);
  return USBD_OK;
}
static int8_t cdc_receive(uint8_t *buf, uint32_t *len)
{
  USBD_CDC_HandleTypeDef *c = (USBD_CDC_HandleTypeDef *)hUsb.pClassData;
  if (c->TxState == 0U) {                          /* echo (drop if the previous packet is still going out) */
    memcpy(txbuf, buf, *len);
    USBD_CDC_SetTxBuffer(&hUsb, txbuf, *len);
    USBD_CDC_TransmitPacket(&hUsb);
  }
  total_rx += *len;
  USBD_CDC_SetRxBuffer(&hUsb, buf);
  USBD_CDC_ReceivePacket(&hUsb);
  return USBD_OK;
}
#if defined(BOARD_F1)   /* the Cube F1 pack ships an older USB Device library: no TransmitCplt callback */
static USBD_CDC_ItfTypeDef fops = { cdc_init, cdc_deinit, cdc_control, cdc_receive };
#else
static int8_t cdc_tx_done(uint8_t *buf, uint32_t *len, uint8_t ep) { (void)buf; (void)len; (void)ep; return USBD_OK; }
static USBD_CDC_ItfTypeDef fops = { cdc_init, cdc_deinit, cdc_control, cdc_receive, cdc_tx_done };
#endif

int main(void)
{
  board_init();
  USBD_Init(&hUsb, &CDC_Desc, 0);
  USBD_RegisterClass(&hUsb, &USBD_CDC);
  USBD_CDC_RegisterInterface(&hUsb, &fops);
  USBD_Start(&hUsb);
  printf("usb-cdc on %s: plug the board USB port into a host, open the new /dev/ttyACM*\r\n", BOARD_NAME);
  uint32_t seen = 0;
  for (;;) {
    if (total_rx != seen) { seen = total_rx; printf("usb rx total %lu bytes\r\n", (unsigned long)seen); board_led_toggle(); }
    HAL_Delay(50);
  }
}
