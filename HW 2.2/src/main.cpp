#include <Arduino.h>

constexpr uint8_t RELAY_PIN = 5, READ_PIN = 6, DEBOUNCE_MS = 25, MEASURMENTS = 10;

constexpr uint32_t MEASUREMENT_TIME = 250;

volatile uint32_t startTimeUs = 0;
volatile uint32_t finishTimeUs = 0;
volatile bool relay = false;
volatile bool read_relay = false;

void IRAM_ATTR readPin() {
    if (read_relay) {
        finishTimeUs = micros();
        read_relay = false;
    }
}

void 

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(READ_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(READ_PIN), readPin, CHANGE);
}

void loop() {
    static uint32_t totalElapsedTimeOnUs = 0;
    static uint32_t totalElapsedTimeOffUs = 0;
    static uint32_t elapsedTimeOnUs = 0;
    static uint32_t elapsedTimeOffUs = 0;
    static uint32_t count = 0;
    static uint32_t lastMillis = millis();
    
    if (millis() - lastMillis >= MEASUREMENT_TIME/2) {
        lastMillis = millis();
        startTimeUs = micros();
        read_relay = true;
        digitalWrite(RELAY_PIN, !digitalRead(RELAY_PIN));
        delay(DEBOUNCE_MS); // Add a small delay to debounce the relay pin
    }

    if (startTimeUs != 0 && finishTimeUs != 0 && count < 2 * MEASURMENTS && count % 2 == 0) {
        elapsedTimeOnUs = finishTimeUs - startTimeUs;
        totalElapsedTimeOnUs += elapsedTimeOnUs;
        count++;
        Serial.print("Elapsed time: ");
        Serial.print(elapsedTimeOnUs);
        Serial.println(" us");
        startTimeUs = 0;
        finishTimeUs = 0;
    }
    if (startTimeUs != 0 && finishTimeUs != 0 && count < 2 * MEASURMENTS && count % 2 == 1) {
        elapsedTimeOffUs = finishTimeUs - startTimeUs;
        totalElapsedTimeOffUs += elapsedTimeOffUs;
        count++;
        Serial.print("Elapsed time: ");
        Serial.print(elapsedTimeOffUs);
        Serial.println(" us");
        startTimeUs = 0;
        finishTimeUs = 0;
    }
    if (count >= 2 * MEASURMENTS) {
        uint32_t averageElapsedTimeOnUs = totalElapsedTimeOnUs / MEASURMENTS;
        uint32_t averageElapsedTimeOffUs = totalElapsedTimeOffUs / MEASURMENTS;

        Serial.print("Average elapsed time on: ");
        Serial.print(averageElapsedTimeOnUs);
        Serial.println(" us");
        Serial.print("Average elapsed time off: ");
        Serial.print(averageElapsedTimeOffUs);
        Serial.println(" us");
        count = 0;
        totalElapsedTimeOnUs = 0;
        totalElapsedTimeOffUs = 0;
    }
}
