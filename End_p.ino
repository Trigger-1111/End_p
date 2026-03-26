#include "include/globals.h"
#include "include/actuator.h"
#include "include/display.h"
#include "include/buttons.h"
#include "include/modes.h"

int currentMode = 1;
bool isRunning = false;
bool ledLight = false;

void setup() {
    Serial.begin(115200);

    setupActuators();
    setupButtons();
    setupDisplay();

    Serial.println("STIRR Ready");
}

void loop() {
    handleButtons();

    if (isRunning) {
        switch (currentMode) {
            case 1: runMode1(); break;
            case 2: runMode2(); break;
            case 3: runMode3(); break;
            case 4: break;  // AI모드 추후 구현
        }
    }
}
