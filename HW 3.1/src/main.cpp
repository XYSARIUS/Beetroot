#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_timer.h"
#include "esp_log.h"

static constexpr adc_unit_t ADC_UNIT = ADC_UNIT_1;
static constexpr adc_channel_t ADC_CHANNEL = ADC_CHANNEL_4;
static constexpr gpio_num_t LED_PIN = GPIO_NUM_4;

static constexpr uint8_t SAMPLES = 16;
static constexpr uint16_t SAMPLE_PERIOD_US = (50 * 1000);
static constexpr uint16_t THRESHOLD_ON_RAW = 1500;      
static constexpr uint16_t THRESHOLD_OFF_RAW = 1800;

static_assert(THRESHOLD_ON_RAW < THRESHOLD_OFF_RAW);
static_assert(SAMPLES > 0);

static const char* TAG = "ADC LDR";

static uint16_t read_adc_avarege(adc_oneshot_unit_handle_t handle, std::uint16_t* result)
{
    if (result == nullptr){
        return ESP_ERR_INVALID_ARG;
    }

    std::uint32_t sum = 0;

    for (std::uint8_t i = 0; i < SAMPLES; ++i) {
        int raw = 0;

        esp_err_t err = adc_oneshot_read(
            handle, 
            ADC_CHANNEL, 
            &raw
        );
        if (err != ESP_OK){
           return err;
        }

        sum += static_cast<std::uint32_t>(raw);
    }

    *result = static_cast<std::uint16_t>(sum / SAMPLES);

    return ESP_OK;
}

extern "C" void app_main(void)
{
    gpio_config_t led_conf = {
        .pin_bit_mask = 1ULL << LED_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&led_conf));
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
 
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

    bool led_on = false;               
    int64_t last_sample = 0;

    while (1) {
        int64_t now = esp_timer_get_time();

        if (now - last_sample >= SAMPLE_PERIOD_US) {
            last_sample = now;

            std::uint16_t level = 0;
            esp_err_t err = read_adc_avarege(adc1_handle, &level); 
            
            if (err == ESP_OK){
                int mv = 0;
                ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, level, &mv));

                if (!led_on && level < THRESHOLD_ON_RAW) {
                    led_on = true;
                } else if (led_on && level > THRESHOLD_OFF_RAW) {
                    led_on = false;               
                }

                ESP_ERROR_CHECK(gpio_set_level(LED_PIN, led_on));

                ESP_LOGI(TAG, "level=%4d (%d mV)   led=%s", level, mv, led_on ? "ON" : "off");
            } else {
                ESP_LOGW(TAG, "ADC read failed: %s", esp_err_to_name(err));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
