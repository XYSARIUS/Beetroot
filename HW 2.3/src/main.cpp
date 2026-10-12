#include <Arduino.h>

uint32_t last1 = 0, last2 = 0, last3 = 0;

struct LedConfig {
    uint8_t pin;
    uint32_t interval;
};

constexpr LedConfig LED1 {5, 200}, 
                    LED2 {6, 500}, 
                    LED3 {7, 1000};

void ledBlynk(const LedConfig& led, uint32_t& last){
    const uint32_t now = millis();
    if (now - last >= led.interval){
        digitalWrite(led.pin,!digitalRead(led.pin));
        last = now;
    }
}

void setup() {
    pinMode(LED1.pin,OUTPUT);
    pinMode(LED2.pin,OUTPUT);
    pinMode(LED3.pin,OUTPUT);
}

void loop() {
    ledBlynk(LED1, last1);
    ledBlynk(LED2, last2);
    ledBlynk(LED3, last3);
}
