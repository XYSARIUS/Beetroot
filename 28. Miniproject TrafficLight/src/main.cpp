#include <cstdint>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"

static constexpr char TAG[] = "Traffic Light";

static constexpr uint32_t RED_INTERVAL_MS = 5000*1000ULL;
static constexpr uint32_t YELLOW_INTERVAL_MS = 2000*1000ULL;
static constexpr uint32_t GREEN_INTERVAL_MS = 8000*1000ULL;
static constexpr uint32_t GREEN_BLINK_INTERVAL_MS = 3000*1000ULL;

static constexpr uint32_t BLINK_MS = 500*1000;

static constexpr gpio_num_t LED_RED = GPIO_NUM_4;
static constexpr gpio_num_t LED_YELLOW = GPIO_NUM_5;
static constexpr gpio_num_t LED_GREEN = GPIO_NUM_6;

enum class State {RED, RED_YELLOW, GREEN, GREEN_BLINK, YELLOW};

struct state_level{
    State currentState;
    int64_t StartTime;
    int64_t last_blink;
};

state_level state = {
    .currentState = State::RED,
    .StartTime = 0,
    .last_blink = 0
};
bool green_led_on = false;

uint32_t duration(State state){
    switch (state){
        case State::RED:        return RED_INTERVAL_MS;
        case State::RED_YELLOW: return YELLOW_INTERVAL_MS;
        case State::GREEN:      return GREEN_INTERVAL_MS;
        case State::GREEN_BLINK:return GREEN_BLINK_INTERVAL_MS;
        case State::YELLOW:     return YELLOW_INTERVAL_MS;
    }
    return RED_INTERVAL_MS;
};

State changeState(State mode, int64_t now){
    state.StartTime = now;
    switch (mode){
        case State::RED:        return State::RED_YELLOW;
        case State::RED_YELLOW: return State::GREEN;
        case State::GREEN:      return State::GREEN_BLINK;
        case State::GREEN_BLINK:return State::YELLOW;
        case State::YELLOW:     return State::RED;
    }
    return State::RED;
};

void LightOn(State state){
    gpio_set_level(LED_RED, state == State::RED ? 1 : (state == State::RED_YELLOW ? 1 : 0));
    gpio_set_level(LED_YELLOW, state == State::YELLOW ? 1 : (state == State::RED_YELLOW ? 1 : 0));
    gpio_set_level(LED_GREEN, state == State::GREEN ? 1 : 0);
}

void Blink(int64_t now){
    if (now - state.last_blink >= BLINK_MS) {
        green_led_on = !green_led_on;
        gpio_set_level(LED_GREEN, green_led_on ? 1 : 0);
        state.last_blink = now;
    }
}

extern "C" void app_main(){
    gpio_config_t led_cfg{};
    led_cfg.pin_bit_mask = (1ULL << LED_RED) | (1ULL << LED_YELLOW) | (1ULL << LED_GREEN);
    led_cfg.mode = GPIO_MODE_OUTPUT;

    ESP_ERROR_CHECK(gpio_config(&led_cfg));

    LightOn(state.currentState);
    
    while (true){
        const int64_t now_us = esp_timer_get_time();
        const int64_t elapsed = now_us - state.StartTime;

        if (elapsed >= duration(state.currentState)) {
            state.currentState = changeState(state.currentState, now_us);
            LightOn(state.currentState);

            if (state.currentState == State::GREEN_BLINK) {
                state.last_blink = now_us;
                green_led_on = false;
                gpio_set_level(LED_GREEN, green_led_on);
            }
        }
        if (state.currentState == State::GREEN_BLINK){
            Blink(now_us);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    
}
