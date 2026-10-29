/* Polled ADC: PA1 plus the internal temperature sensor and VREFINT, printed once per second.
 *   F411: ADC1 IN1 (PA1), IN18 temp, IN17 VREFINT; datasheet typical slope (no factory calibration).
 *   F103: ADC1 IN1 (PA1), IN16 temp, IN17 VREFINT; datasheet typical slope (no factory calibration).
 *   G431: ADC1 IN2 (PA1), temp + VREFINT on ADC1; factory calibration values (TS_CAL1/2, VREFINT_CAL).
 *   H743: ADC1 INP17 (PA1); temp + VREFINT exist only on ADC3; factory calibration values. */
#include "board.h"

static ADC_HandleTypeDef hadc;           /* ADC1 */
#if defined(BOARD_H7)
static ADC_HandleTypeDef hadc3;          /* internal channels */
#endif

static uint32_t read_channel(ADC_HandleTypeDef *h, uint32_t ch, uint32_t sample_time)
{
  ADC_ChannelConfTypeDef c = {0};
  c.Channel = ch; c.SamplingTime = sample_time;
#if defined(BOARD_G4) || defined(BOARD_H7)
  c.Rank = ADC_REGULAR_RANK_1; c.SingleDiff = ADC_SINGLE_ENDED; c.OffsetNumber = ADC_OFFSET_NONE;
#else
  c.Rank = 1;
#endif
  HAL_ADC_ConfigChannel(h, &c);
  HAL_ADC_Start(h);
  HAL_ADC_PollForConversion(h, 10);
  uint32_t v = HAL_ADC_GetValue(h);
  HAL_ADC_Stop(h);
  return v;
}

#if defined(BOARD_F4)
static void adc_init(void)
{
  __HAL_RCC_ADC1_CLK_ENABLE();
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;       /* PCLK2 100 MHz / 4 = 25 MHz (max 36) */
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.ScanConvMode = DISABLE; hadc.Init.ContinuousConvMode = DISABLE; hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT; hadc.Init.NbrOfConversion = 1; hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc) != HAL_OK) for (;;);
}
#define PA1_CH ADC_CHANNEL_1
#elif defined(BOARD_F1)
static void adc_init(void)
{
  RCC_PeriphCLKInitTypeDef p = {0};
  p.PeriphClockSelection = RCC_PERIPHCLK_ADC; p.AdcClockSelection = RCC_ADCPCLK2_DIV6;   /* 72 / 6 = 12 MHz (max 14) */
  HAL_RCCEx_PeriphCLKConfig(&p);
  __HAL_RCC_ADC1_CLK_ENABLE();
  hadc.Instance = ADC1;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE; hadc.Init.ContinuousConvMode = DISABLE; hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START; hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT; hadc.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc) != HAL_OK) for (;;);
  HAL_ADCEx_Calibration_Start(&hadc);
}
#define PA1_CH ADC_CHANNEL_1
#elif defined(BOARD_G4)
static void adc_init(void)
{
  __HAL_RCC_ADC12_CLK_ENABLE();
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;       /* HCLK 144 MHz / 4 = 36 MHz (max 60) */
  hadc.Init.Resolution = ADC_RESOLUTION_12B; hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.GainCompensation = 0; hadc.Init.ScanConvMode = ADC_SCAN_DISABLE; hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE; hadc.Init.ContinuousConvMode = DISABLE; hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE; hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN; hadc.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK) for (;;);
  HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);
}
#define PA1_CH ADC_CHANNEL_2
#elif defined(BOARD_H7)
static void adc_init_one(ADC_HandleTypeDef *h, ADC_TypeDef *inst)
{
  h->Instance = inst;
  h->Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;             /* per_ck = HSI 64 MHz / 4 = 16 MHz */
  h->Init.Resolution = ADC_RESOLUTION_16B; h->Init.ScanConvMode = ADC_SCAN_DISABLE; h->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  h->Init.LowPowerAutoWait = DISABLE; h->Init.ContinuousConvMode = DISABLE; h->Init.NbrOfConversion = 1;
  h->Init.DiscontinuousConvMode = DISABLE; h->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  h->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; h->Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  h->Init.Overrun = ADC_OVR_DATA_OVERWRITTEN; h->Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE; h->Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(h) != HAL_OK) for (;;);
  HAL_ADCEx_Calibration_Start(h, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
}
static void adc_init(void)
{
  RCC_PeriphCLKInitTypeDef p = {0};
  p.PeriphClockSelection = RCC_PERIPHCLK_ADC; p.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;   /* per_ck, reset = HSI 64 MHz */
  HAL_RCCEx_PeriphCLKConfig(&p);
  __HAL_RCC_ADC12_CLK_ENABLE();
  __HAL_RCC_ADC3_CLK_ENABLE();
  adc_init_one(&hadc, ADC1);
  adc_init_one(&hadc3, ADC3);
}
#define PA1_CH ADC_CHANNEL_17
#endif

int main(void)
{
  board_init();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef g = { .Pin = GPIO_PIN_1, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL };
  HAL_GPIO_Init(GPIOA, &g);
  adc_init();
  printf("adc-read on %s: PA1 / temp / vrefint\r\n", BOARD_NAME);
  for (;;) {
    int32_t mv_pa1, t_c_x10;
    uint32_t pa1, ts, vr;
#if defined(BOARD_F4) || defined(BOARD_F1)
#if defined(BOARD_F4)
    pa1 = read_channel(&hadc, PA1_CH, ADC_SAMPLETIME_56CYCLES);
    ts  = read_channel(&hadc, ADC_CHANNEL_TEMPSENSOR, ADC_SAMPLETIME_480CYCLES);   /* sensor needs >= 10 us */
    vr  = read_channel(&hadc, ADC_CHANNEL_VREFINT, ADC_SAMPLETIME_480CYCLES);
    const int32_t v25 = 760, slope_x10 = 25;   /* F411 datasheet typical: 0.76 V at 25 C, +2.5 mV/C */
#else
    pa1 = read_channel(&hadc, PA1_CH, ADC_SAMPLETIME_55CYCLES_5);
    ts  = read_channel(&hadc, ADC_CHANNEL_TEMPSENSOR, ADC_SAMPLETIME_239CYCLES_5); /* 20 us >= 17.1 us */
    vr  = read_channel(&hadc, ADC_CHANNEL_VREFINT, ADC_SAMPLETIME_239CYCLES_5);
    const int32_t v25 = 1430, slope_x10 = -43; /* F103 datasheet typical: 1.43 V at 25 C, -4.3 mV/C */
#endif
    /* assumes VDDA = 3.3 V (approximate) */
    mv_pa1 = (int32_t)(pa1 * 3300U / 4095U);
    int32_t mv_ts = (int32_t)(ts * 3300U / 4095U);
    t_c_x10 = ((mv_ts - v25) * 100) / slope_x10 + 250;
#elif defined(BOARD_G4)
    pa1 = read_channel(&hadc, PA1_CH, ADC_SAMPLETIME_47CYCLES_5);
    ts  = read_channel(&hadc, ADC_CHANNEL_TEMPSENSOR_ADC1, ADC_SAMPLETIME_640CYCLES_5);   /* >= 5 us */
    vr  = read_channel(&hadc, ADC_CHANNEL_VREFINT, ADC_SAMPLETIME_640CYCLES_5);
    uint32_t vdda = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vr, ADC_RESOLUTION_12B);
    mv_pa1 = (int32_t)__HAL_ADC_CALC_DATA_TO_VOLTAGE(vdda, pa1, ADC_RESOLUTION_12B);
    t_c_x10 = 10 * (int32_t)__HAL_ADC_CALC_TEMPERATURE(vdda, ts, ADC_RESOLUTION_12B);
#elif defined(BOARD_H7)
    pa1 = read_channel(&hadc, PA1_CH, ADC_SAMPLETIME_64CYCLES_5);
    ts  = read_channel(&hadc3, ADC_CHANNEL_TEMPSENSOR, ADC_SAMPLETIME_810CYCLES_5);       /* >= 9 us */
    vr  = read_channel(&hadc3, ADC_CHANNEL_VREFINT, ADC_SAMPLETIME_810CYCLES_5);
    uint32_t vdda = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vr, ADC_RESOLUTION_16B);
    mv_pa1 = (int32_t)__HAL_ADC_CALC_DATA_TO_VOLTAGE(vdda, pa1, ADC_RESOLUTION_16B);
    t_c_x10 = 10 * (int32_t)__HAL_ADC_CALC_TEMPERATURE(vdda, ts, ADC_RESOLUTION_16B);
#endif
    printf("PA1 raw=%5lu (%4ld mV)  temp raw=%5lu (~%ld.%ld C)  vrefint raw=%5lu\r\n", (unsigned long)pa1, (long)mv_pa1,
           (unsigned long)ts, (long)(t_c_x10 / 10), (long)(t_c_x10 < 0 ? -t_c_x10 % 10 : t_c_x10 % 10), (unsigned long)vr);
    board_led_toggle();
    HAL_Delay(1000);
  }
}
