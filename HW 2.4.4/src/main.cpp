#include <cstdint>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"

static constexpr char TAG[] = "BUTTON_LOG";

static constexpr gpio_num_t button_gpio = GPIO_NUM_5;
static constexpr int64_t debounce_us = 50 * 1000;
static constexpr uint32_t POLL_INTERVAL_MS = 10;

struct ButtonState {
    bool raw_level;
    bool stable_level;
    int64_t last_change_us;
};


extern "C" void app_main(){
    gpio_config_t button_cfg{};
    button_cfg.pin_bit_mask = 1ULL << button_gpio;
    button_cfg.mode = GPIO_MODE_INPUT;
    button_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    button_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    button_cfg.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&button_cfg));
    
    const bool initial_level = gpio_get_level(button_gpio);
    ButtonState button = {
        .raw_level = initial_level,
        .stable_level = initial_level,
        .last_change_us = esp_timer_get_time()
    };

    uint32_t counter = 0;

    ESP_LOGI(TAG,"Polling started!");
       
    while (true){
        const int64_t now_us = esp_timer_get_time();
        const bool current_level = gpio_get_level(button_gpio);
        
        if(current_level != button.raw_level){
            button.raw_level = current_level;
            button.last_change_us = now_us;
        }

        if ((now_us - button.last_change_us >= debounce_us) 
            && (button.stable_level != button.raw_level)){
                button.stable_level = button.raw_level;
                if (button.stable_level == 0) {
                    ++counter;
                    ESP_LOGI(TAG, "Button pressed. Counter: %d",counter);
                }else{
                    ESP_LOGI(TAG, "Button released");
                }
        }
        vTaskDelay(1);
    }
}
