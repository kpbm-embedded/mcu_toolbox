/* Read one external ADC pin once a second and print raw value and millivolts (16-bit, 3.2 V reference).
 * bl616: ADC_CH3 = GPIO3 (board_adc_gpio_init in bsp/board/bl616dk/board_gpio.c maps CH0..CH6 to GPIO20,19,2,3,14,13,12).
 * bl702: ADC_CH0; board_adc_gpio_init sets GPIO18,19,20 analog but the CH->GPIO order is not stated in the SDK
 *        (take it from the BL702 datasheet; unverified here).
 * bl808 (legacy SDK, M0): ADC_CH3 = GPIO11 (bouffalo_sdk_legacy/bsp/board/bl808dk/board.c board_adc_gpio_init). */
#include "bflb_adc.h"
#include "bflb_mtimer.h"
#include "board.h"

#if defined(BL702)
#define ADC_CH ADC_CHANNEL_0
#else
#define ADC_CH ADC_CHANNEL_3
#endif

static struct bflb_adc_channel_s chan[] = { { .pos_chan = ADC_CH, .neg_chan = ADC_CHANNEL_GND } };

int main(void)
{
    board_init();
    board_adc_gpio_init();

    struct bflb_device_s *adc = bflb_device_get_by_name("adc");
    struct bflb_adc_config_s cfg = { .clk_div = ADC_CLK_DIV_32, .scan_conv_mode = false, .continuous_conv_mode = true,
                                     .differential_mode = false, .resolution = ADC_RESOLUTION_16B, .vref = ADC_VREF_3P2V };
    bflb_adc_init(adc, &cfg);
    bflb_adc_channel_config(adc, chan, 1);
    bflb_adc_start_conversion(adc);

    while (1) {
        while (bflb_adc_get_count(adc) == 0) {
            bflb_mtimer_delay_ms(1);
        }
        uint32_t raw = bflb_adc_read_raw(adc);
        struct bflb_adc_result_s res;
        bflb_adc_parse_result(adc, &raw, &res, 1);
        printf("adc raw=0x%08lx %d mV\r\n", (unsigned long)raw, (int)res.millivolt);
        bflb_mtimer_delay_ms(1000);
    }
}
