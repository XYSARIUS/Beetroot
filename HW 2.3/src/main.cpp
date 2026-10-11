#include <Arduino.h>

constexpr uint8_t RELAY_PIN = 5, READ_PIN = 6, DEBOUNCE_MS = 50, MEASURMENTS = 10;

constexpr uint32_t MEASUREMENT_TIME = 500;

volatile uint32_t startTimeUs = 0;
volatile uint32_t finishTimeUs = 0;
volatile bool relay = false;
volatile bool read_relay = false;

void IRAM_ATTR relayPin() {
    relay = true;
}

void IRAM_ATTR readPin() {
    finishTimeUs = micros();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(READ_PIN, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(RELAY_PIN), relayPin, RISING);
    attachInterrupt(digitalPinToInterrupt(READ_PIN), readPin, FALLING);
}

void loop() {
    static uint32_t totalElapsedTimeOnUs = 0;
    static uint32_t count = 0;
    static uint32_t lastMillis = millis();
    
    if (millis() - lastMillis >= MEASUREMENT_TIME/2) {
        lastMillis = millis();
        startTimeUs = micros();
        digitalWrite(RELAY_PIN, !digitalRead(RELAY_PIN));
    }

    if (startTimeUs != 0 && finishTimeUs != 0 && count < MEASURMENTS ) {
        uint32_t elapsedTimeOnUs = finishTimeUs - startTimeUs;
        totalElapsedTimeOnUs += elapsedTimeOnUs;
        count++;
        Serial.print("Elapsed time: ");
        Serial.print(elapsedTimeOnUs);
        Serial.println(" us");
        startTimeUs = 0;
        finishTimeUs = 0;
    }
    if (count >= MEASURMENTS) {
        uint32_t averageElapsedTimeOnUs = totalElapsedTimeOnUs / MEASURMENTS;
      
        Serial.print("Average elapsed time on: ");
        Serial.print(averageElapsedTimeOnUs);
        Serial.println(" us");
        count = 0;
        totalElapsedTimeOnUs = 0;
    }
}
