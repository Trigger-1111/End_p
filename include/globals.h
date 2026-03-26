#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

extern int currentMode;
extern bool isRunning;
extern bool ledLight;

// 액추에이터 속도 (0~255)
#define ACT_SPEED 200

// 모드별 타이밍 (ms)
#define PHASE_TIME_1  800
#define PHASE_TIME_2  1000
#define PHASE_TIME_3  600

#define DEBOUNCE_MS 200

#endif // GLOBALS_H
