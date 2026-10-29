#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "soc/soc_caps.h"
#if SOC_TEMP_SENSOR_SUPPORTED /* classic ESP32 has no supported die temperature sensor */
#include "driver/temperature_sensor.h"
#endif
#include "esp_log.h"
#include "pins.h"

static const char *TAG = "adc-read";

void app_main(void)
{
    adc_oneshot_unit_handle_t adc;
    adc_oneshot_unit_init_cfg_t u = { .unit_id = ADC_UNIT_1 };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&u, &adc));
    adc_oneshot_chan_cfg_t cc = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, ADC_CH, &cc));

    adc_cali_handle_t cali = NULL;
    bool have_cali = false;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cf = { .unit_id = ADC_UNIT_1, .chan = ADC_CH, .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    have_cali = adc_cali_create_scheme_curve_fitting(&cf, &cali) == ESP_OK;
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t lf = { .unit_id = ADC_UNIT_1, .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    have_cali = adc_cali_create_scheme_line_fitting(&lf, &cali) == ESP_OK;
#endif

#if SOC_TEMP_SENSOR_SUPPORTED
    temperature_sensor_handle_t ts;
    temperature_sensor_config_t tc = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
    ESP_ERROR_CHECK(temperature_sensor_install(&tc, &ts));
    ESP_ERROR_CHECK(temperature_sensor_enable(ts));
#endif

    for (;;) {
        int raw = 0, mv = -1; float tc_deg = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc, ADC_CH, &raw));
        if (have_cali) adc_cali_raw_to_voltage(cali, raw, &mv);
#if SOC_TEMP_SENSOR_SUPPORTED
        ESP_ERROR_CHECK(temperature_sensor_get_celsius(ts, &tc_deg));
        ESP_LOGI(TAG, "raw=%d mv=%d chip_temp=%.1f C", raw, mv, tc_deg);
#else
        (void)tc_deg;
        ESP_LOGI(TAG, "raw=%d mv=%d", raw, mv);
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
