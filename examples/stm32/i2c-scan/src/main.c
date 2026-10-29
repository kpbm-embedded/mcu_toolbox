/* Scan I2C1 (100 kHz) for 7-bit devices and print a table every 5 s.
 *   F411: PB8 SCL / PB9 SDA (AF4)      F103: PB6 SCL / PB7 SDA (default mapping)
 *   G431: PA15 SCL / PB7 SDA (AF4)     H743: PB8 SCL / PB9 SDA (AF4)
 * G4/H7 have the I2C v2 peripheral (TIMINGR): kernel clock = HSI 16 MHz (G4) / HSI 64 MHz (H7),
 * timing from the RM 100 kHz example (SCLL 0x13, SCLH 0xF, SDADEL 2, SCLDEL 4 at tPRESC 250 ns). */
#include "board.h"

static void pins_init(void)
{
  GPIO_InitTypeDef g = {0};
  g.Mode = GPIO_MODE_AF_OD; g.Speed = GPIO_SPEED_FREQ_HIGH;
#if defined(BOARD_F1)
  __HAL_RCC_GPIOB_CLK_ENABLE();
  g.Pin = GPIO_PIN_6 | GPIO_PIN_7; HAL_GPIO_Init(GPIOB, &g);
#elif defined(BOARD_G4)
  __HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_GPIOB_CLK_ENABLE();
  g.Pull = GPIO_PULLUP; g.Alternate = GPIO_AF4_I2C1;
  g.Pin = GPIO_PIN_15; HAL_GPIO_Init(GPIOA, &g);
  g.Pin = GPIO_PIN_7;  HAL_GPIO_Init(GPIOB, &g);
#else
  __HAL_RCC_GPIOB_CLK_ENABLE();
  g.Pull = GPIO_PULLUP; g.Alternate = GPIO_AF4_I2C1;
  g.Pin = GPIO_PIN_8 | GPIO_PIN_9; HAL_GPIO_Init(GPIOB, &g);
#endif
}

int main(void)
{
  board_init();
  pins_init();
  I2C_HandleTypeDef h = {0};
  h.Instance = I2C1;
#if defined(BOARD_G4) || defined(BOARD_H7)
  RCC_PeriphCLKInitTypeDef p = {0};
  p.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
#if defined(BOARD_G4)
  p.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;     /* 16 MHz */
  h.Init.Timing = 0x30420F13;                       /* PRESC 3 -> 250 ns */
#else
  p.I2c123ClockSelection = RCC_I2C1CLKSOURCE_HSI;   /* 64 MHz (HSIDIV reset /1) */
  h.Init.Timing = 0xF0420F13;                       /* PRESC 15 -> 250 ns */
#endif
  HAL_RCCEx_PeriphCLKConfig(&p);
  __HAL_RCC_I2C1_CLK_ENABLE();
  h.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
#else
  __HAL_RCC_I2C1_CLK_ENABLE();
  h.Init.ClockSpeed = 100000; h.Init.DutyCycle = I2C_DUTYCYCLE_2;
#endif
  h.Init.OwnAddress1 = 0;
  h.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT; h.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  h.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE; h.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&h) != HAL_OK) { printf("I2C init failed\r\n"); for (;;); }
#if defined(BOARD_G4) || defined(BOARD_H7)
  HAL_I2CEx_ConfigAnalogFilter(&h, I2C_ANALOGFILTER_ENABLE);
#endif
  for (;;) {
    int found = 0;
    printf("\r\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
    for (uint8_t a = 0; a < 0x78; a++) {
      if ((a & 0xf) == 0) printf("\r\n%02x:", a);
      if (a < 0x08) { printf("   "); continue; }
      if (HAL_I2C_IsDeviceReady(&h, (uint16_t)(a << 1), 1, 5) == HAL_OK) { printf(" %02x", a); found++; } else printf(" --");
    }
    printf("\r\n%d device(s)\r\n", found);
    board_led_toggle();
    HAL_Delay(5000);
  }
}
