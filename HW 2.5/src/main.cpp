#include <stdint.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"

static const char* TAG = "VENT_LOG";

constexpr uint64_t SECOND_US = 1000*1000;
constexpr uint64_t MINUTE_US = SECOND_US * 1; // 1 -> 60 for minutes

constexpr uint64_t PERIOD_US = 10LL * MINUTE_US; // 10 -> 60
constexpr uint64_t RUNTIME_US = 2LL * MINUTE_US; //2 -> 15

constexpr gpio_num_t FAN_PIN = GPIO_NUM_5; 

static esp_timer_handle_t start_timer = nullptr;
static esp_timer_handle_t stop_timer = nullptr;

static bool fan_running = false;

static void fan_stop_callback(void* arg){
    if (!fan_running){
        return;
    }
    gpio_set_level(FAN_PIN,0);
    fan_running = false;
    
    ESP_LOGI(TAG, "Fan OFF");
}

static void fan_start_callback(void* arg){
    if (fan_running){
        return;
    }
    gpio_set_level(FAN_PIN, 1);
    fan_running = true;

    ESP_LOGI(TAG, "Fan ON");

    esp_err_t err = esp_timer_start_once(stop_timer, RUNTIME_US);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start stop timer: %s",
                 esp_err_to_name(err));

        gpio_set_level(FAN_PIN, 0);
        fan_running = false;
    }
}

static void fan_gpio_init(){
    gpio_config_t config = {};
    config.pin_bit_mask = (1ULL<<FAN_PIN);
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&config));

    gpio_set_level(FAN_PIN, 0);
}

static void timers_init(){
    esp_timer_create_args_t start_args = {};
    start_args.callback = fan_start_callback;
    start_args.name = "fan_start";

    ESP_ERROR_CHECK(esp_timer_create(&start_args, &start_timer));

    esp_timer_create_args_t stop_args = {};

    stop_args.callback = fan_stop_callback;
    stop_args.name = "fan_stop";

    ESP_ERROR_CHECK(
        esp_timer_create(&stop_args, &stop_timer)
    );
}



extern "C" void app_main(void){
    ESP_LOGI(TAG, "System initialization");

    fan_gpio_init();

    timers_init();

    ESP_ERROR_CHECK(esp_timer_start_periodic(start_timer, PERIOD_US));
    ESP_LOGI(TAG, "Fan scheduler started");
}