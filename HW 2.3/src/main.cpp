#include <Arduino.h>

uint32_t last1 = 0, last2 = 0, last3 = 0;

struct LedConfig {
    uint8_t pin;
    uint32_t delay;
};

constexpr LedConfig LED1 {.pin = 5, .delay = 200}, 
                    LED2 {.pin = 6, .delay = 500}, 
                    LED3 {.pin = 7, .delay = 1000};

void ledBlynk(LedConfig led, uint32_t &last){
    if (millis() - last >= led.delay){
        digitalWrite(led.pin,!digitalRead(led.pin));
        last = millis();
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
