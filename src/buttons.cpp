#include "../include/buttons.h"
#include "../include/pins.h"
#include "../include/globals.h"
#include "../include/display.h"
#include "../include/actuator.h"
#include <Arduino.h>

unsigned long lastBtnTime = 0;

void setupButtons() {
    // 버튼 핀
    pinMode(BTN_START, INPUT);
    pinMode(BTN_STOP,  INPUT);
    pinMode(BTN_MODE,  INPUT);
    pinMode(BTN_LED,   INPUT);

    // LED 핀
    pinMode(LED_STATUS, OUTPUT);
    pinMode(LED_LIGHT,  OUTPUT);
    digitalWrite(LED_STATUS, LOW);
    digitalWrite(LED_LIGHT,  LOW);
}

void handleButtons() {
    unsigned long now = millis();
    if (now - lastBtnTime < DEBOUNCE_MS) return;

    // 시작 버튼
    if (digitalRead(BTN_START) == LOW) {
        isRunning = true;
        digitalWrite(LED_STATUS, HIGH);
        updateDisplay();
        lastBtnTime = now;
    }

    // 종료 버튼
    if (digitalRead(BTN_STOP) == LOW) {
        isRunning = false;
        stopAll();
        digitalWrite(LED_STATUS, LOW);
        updateDisplay();
        lastBtnTime = now;
    }

    // 모드 버튼 (정지 중에만 변경)
    if (digitalRead(BTN_MODE) == LOW && !isRunning) {
        currentMode = (currentMode % 4) + 1;
        updateDisplay();
        lastBtnTime = now;
    }

    // 조명 버튼
    if (digitalRead(BTN_LED) == HIGH) {
        ledLight = !ledLight;
        digitalWrite(LED_LIGHT, ledLight ? HIGH : LOW);
        updateDisplay();
        lastBtnTime = now;
    }
}
