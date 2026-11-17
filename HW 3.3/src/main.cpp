#include <cstdint>                 // uint32_t: 32-бітне ціле без знака.
#include "driver/gpio.h"           // gpio_num_t та GPIO_NUM_5: тип і номер виводу.
#include "driver/ledc.h"           // Таймер, канал і функції апаратного PWM.
#include "esp_err.h"               // ESP_ERROR_CHECK: зупинка з діагностикою при помилці.
#include "freertos/FreeRTOS.h"     // pdMS_TO_TICKS: мілісекунди -> тіки планувальника.
#include "freertos/task.h"         // vTaskDelay: віддати процесор на час очікування.

#include "esp_adc/adc_oneshot.h"


constexpr gpio_num_t LED = GPIO_NUM_5;
constexpr gpio_num_t MOTOR = GPIO_NUM_4;
constexpr ledc_mode_t kMode = LEDC_LOW_SPEED_MODE;
constexpr ledc_timer_t kTimer = LEDC_TIMER_0;
constexpr ledc_channel_t ChannelLed = LEDC_CHANNEL_0;
constexpr ledc_channel_t ChannelMotor = LEDC_CHANNEL_1;
constexpr adc_channel_t adc_chan = ADC_CHANNEL_5;
constexpr uint32_t frequency = 10000;

constexpr uint32_t adc_max = (1U << 12) - 1;
constexpr uint32_t duty_max = (1U << 10) - 1;

extern "C" void app_main()
{
    ledc_timer_config_t timer{};
    timer.speed_mode = kMode;
    timer.timer_num = kTimer;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.freq_hz = frequency;
    timer.clk_cfg = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t led_cfg{};
    led_cfg.gpio_num = LED;
    led_cfg.speed_mode = kMode;
    led_cfg.timer_sel = kTimer;
    led_cfg.channel = ChannelLed;
    led_cfg.duty = 0;
    led_cfg.hpoint = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&led_cfg));

    ledc_channel_config_t motor_cfg{};
    motor_cfg.gpio_num = MOTOR;
    motor_cfg.speed_mode = kMode;
    motor_cfg.timer_sel = kTimer;
    motor_cfg.channel = ChannelMotor;
    motor_cfg.duty = 0;
    motor_cfg.hpoint = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&motor_cfg));

    adc_oneshot_unit_init_cfg_t adc_cfg{};
    adc_cfg.unit_id = ADC_UNIT_1;
    adc_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;
    adc_oneshot_unit_handle_t adc_handle = nullptr;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &adc_handle));
    adc_oneshot_chan_cfg_t adc_channel_cfg{};
    adc_channel_cfg.bitwidth = ADC_BITWIDTH_12;
    adc_channel_cfg.atten = ADC_ATTEN_DB_12;
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, adc_chan, &adc_channel_cfg));
    
    while (true) 
    {
    int adc_raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_chan, &adc_raw));
    const uint32_t duty = static_cast<uint32_t>(adc_raw) * duty_max / adc_max;
    
    ESP_ERROR_CHECK(ledc_set_duty(kMode, ChannelLed, duty));
    ESP_ERROR_CHECK(ledc_update_duty(kMode, ChannelLed));
    ESP_ERROR_CHECK(ledc_set_duty(kMode, ChannelMotor, duty));
    ESP_ERROR_CHECK(ledc_update_duty(kMode, ChannelMotor));
    vTaskDelay(pdMS_TO_TICKS(50));
    }
}
