#include "board.h"
#include "py32f0xx_ll_adc.h"

#define VDDA_MV 3300   /* assumed supply; the LL macro converts counts to mV against it */

static void adc_init(void)
{
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_ANALOG);
    LL_ADC_Reset(ADC1);
    if (LL_ADC_IsEnabled(ADC1) == 0) {
        LL_ADC_StartCalibration(ADC1);
        while (LL_ADC_IsCalibrationOnGoing(ADC1)) {}
        LL_mDelay(1);
    }
    LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_NONE);
    LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_SYNC_PCLK_DIV2);
    LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);
    LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);
    LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_41CYCLES_5);
    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);
    LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_4);
    LL_ADC_Enable(ADC1);
    LL_mDelay(1);
}

int main(void)
{
    BSP_RCC_HSI_24MConfig();
    BSP_USART_Config(115200);
    adc_init();
    printf("adc-read: PA4 (ch4)\r\n");
    for (;;) {
        LL_ADC_REG_StartConversion(ADC1);
        while (!LL_ADC_IsActiveFlag_EOS(ADC1)) {}
        LL_ADC_ClearFlag_EOS(ADC1);
        uint32_t raw = LL_ADC_REG_ReadConversionData12(ADC1);
        printf("raw %lu = %lu mV\r\n", (unsigned long)raw, (unsigned long)__LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_MV, raw, LL_ADC_RESOLUTION_12B));
        LL_mDelay(500);
    }
}
