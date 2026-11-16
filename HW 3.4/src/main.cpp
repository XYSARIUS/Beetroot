#include <cstdint>                 // uint32_t: 32-бітне ціле без знака.
#include "driver/gpio.h"           // gpio_num_t та GPIO_NUM_5: тип і номер виводу.
#include "driver/ledc.h"           // Таймер, канал і функції апаратного PWM.
#include "esp_err.h"               // ESP_ERROR_CHECK: зупинка з діагностикою при помилці.
#include "freertos/FreeRTOS.h"     // pdMS_TO_TICKS: мілісекунди -> тіки планувальника.
#include "freertos/task.h"         // vTaskDelay: віддати процесор на час очікування.
#include "esp_timer.h"

constexpr gpio_num_t kOutputGpio = GPIO_NUM_5;
constexpr ledc_mode_t kMode = LEDC_LOW_SPEED_MODE;
constexpr ledc_timer_t kTimer = LEDC_TIMER_0;
constexpr ledc_channel_t kChannel = LEDC_CHANNEL_0;
constexpr uint32_t kDuty = 512;
constexpr uint64_t interval_ms = 25;

// melody

constexpr uint32_t kA4 = 440;
constexpr uint32_t kG3 = 196;
constexpr uint32_t kC4 = 262;
constexpr uint32_t kD4 = 294;
constexpr uint32_t kE4 = 330;
constexpr uint32_t kF4 = 349;
constexpr uint32_t kG4 = 392;

struct Note {
    uint32_t frequencyHz;
    uint32_t durationMs;
};

constexpr Note kMelody[] = {
    {kC4, 300},
    {kD4, 300},
    {kE4, 300},
    {kC4, 300},
    {0, 50},
    {kC4, 300},
    {kD4, 300},
    {kE4, 300},
    {kC4, 300},
    {0, 50},
    {kE4, 300},
    {kF4, 300},
    {kG4, 600},
    {0, 50},
    {kE4, 300},
    {kF4, 300},
    {kG4, 600},
    {0, 50},
    {kG4, 150},
    {kA4, 150},
    {kG4, 150},
    {kF4, 150},
    {kE4, 300},
    {kC4, 300},
    {0, 50},
    {kG4, 150},
    {kA4, 150},
    {kG4, 150},
    {kF4, 150},
    {kE4, 300},
    {kC4, 300},
    {0, 50},
    {kC4, 300},
    {kG3, 300},
    {kC4, 600},
    {0,2000},
};

const uint32_t melodySize =
    sizeof(kMelody) / sizeof(kMelody[0]);
//
uint32_t currentNote = 0;
uint64_t NoteStartTime = 0;

bool isPlaying = false;  

void configurePwm(uint32_t frequencyHz)
{
    ledc_timer_config_t timer{};
    timer.speed_mode = kMode;
    timer.timer_num = kTimer;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.freq_hz = frequencyHz;
    timer.clk_cfg = LEDC_USE_APB_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t output{};
    output.gpio_num = kOutputGpio;
    output.speed_mode = kMode;
    output.timer_sel = kTimer;
    output.channel = kChannel;
    output.duty = 0;
    output.hpoint = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&output));
}

void setFrequency(uint32_t frequency){
    if (frequency == 0){
        ESP_ERROR_CHECK(ledc_set_duty(kMode, kChannel, 0));
        ESP_ERROR_CHECK(ledc_update_duty(kMode, kChannel));
        return;
    }
    ESP_ERROR_CHECK(ledc_set_freq(kMode, kTimer, frequency));
    ESP_ERROR_CHECK(ledc_set_duty(kMode, kChannel, kDuty));
    ESP_ERROR_CHECK(ledc_update_duty(kMode, kChannel));
}

void playMelody()
{
    currentNote = 0;
    uint64_t now = esp_timer_get_time();

    NoteStartTime = now;

    isPlaying = true;
    
    setFrequency(kMelody[currentNote].frequencyHz);
}

void stopMelody()
{
    isPlaying = false;
    setFrequency(0);
    playMelody();
}

void updatePlayer()
{
    if (!isPlaying) return;
    
    uint64_t now = esp_timer_get_time();

    uint64_t duration = kMelody[currentNote].durationMs * 1000ULL;

    if (now - NoteStartTime < duration) return;

    currentNote++;

    if (currentNote >= melodySize){
        stopMelody();
        return;
    }

    NoteStartTime = now;
    setFrequency(kMelody[currentNote].frequencyHz);
}


extern "C" void app_main()
{
    configurePwm(kMelody[0].frequencyHz);
    
    playMelody();
    while (true) {
        updatePlayer();
    }
}
