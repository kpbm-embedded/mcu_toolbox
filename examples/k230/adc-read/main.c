/* Read all six channels of the K230 12-bit SAR ADC once a second (drv_adc). Inputs are 0..1.8 V. */
#include <stdio.h>
#include <unistd.h>
#include "drv_adc.h"
#include "board.h"

int main(void)
{
    if (drv_adc_init()) { printf("adc init failed\n"); return 1; }
    printf("adc-read (%s): %d channels, 12 bit, ref %u uV\n", BOARD_NAME, DRV_ADC_MAX_CHANNEL, (unsigned)DRV_ADC_DEFAULT_REF_UV);
    for (int n = 0; n < 10; n++) {
        for (int ch = 0; ch < DRV_ADC_MAX_CHANNEL; ch++) {
            uint32_t raw = drv_adc_read(ch), uv = 0;
            if (raw == DRV_ADC_INVALID_VALUE || drv_adc_raw_to_uv(raw, DRV_ADC_DEFAULT_REF_UV, &uv)) { printf(" ch%d: err  ", ch); continue; }
            printf(" ch%d: %4u (%4u mV)", ch, (unsigned)raw, (unsigned)(uv / 1000));
        }
        printf("\n");
        sleep(1);
    }
    drv_adc_deinit();
    return 0;
}
