#include <cstdint>
#include <cmath>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_timer.h"
#include "esp_log.h"

static constexpr adc_unit_t ADC_UNIT = ADC_UNIT_1;
static constexpr adc_channel_t ADC_CHANNEL = ADC_CHANNEL_4;

static constexpr std::uint32_t SAMPLE_PERIOD_US = (100 * 1000);

static const char* TAG = "ADC RAW";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Initialization");
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {};
    init_config.unit_id = ADC_UNIT; 
    
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &chan_config));

    adc_cali_handle_t cali_handle = nullptr;
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT, .chan = ADC_CHANNEL,
        .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));
          
    int64_t last_sample = 0; 

    printf("\npos\tRAW\tU_manual\tU_cali\tError(%%)\n");
    printf("________________________________________\n");

    while (1) {
        int64_t now = esp_timer_get_time();

        if (now - last_sample >= SAMPLE_PERIOD_US) {
            last_sample = now;

            int raw = 0;
            adc_oneshot_read(adc1_handle, ADC_CHANNEL, &raw);

            double U_manual_mV = static_cast<double>(raw) / 4095.0 * 3377.5;
            
            int U_cali_mV = 0;
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &U_cali_mV));

            float error = 0.0f;
            if (U_cali_mV != 0){
                error = fabsf(static_cast<float>(U_manual_mV-U_cali_mV)) / static_cast<float>(U_cali_mV) * 100.0f;
            }
            
            double pos = static_cast<double>(raw)/4095 * 100.0;

            printf("%.1f%%\t%d\t%.1f\t%d\t%.2f\n", pos, raw, U_manual_mV, U_cali_mV, error);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
