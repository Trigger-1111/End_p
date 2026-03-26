#include "../include/modes.h"
#include "../include/pins.h"
#include "../include/globals.h"
#include "../include/actuator.h"
#include "../include/buttons.h"
#include <Arduino.h>

void smartDelay(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        handleButtons(); // 대기 중에도 지속적으로 버튼 감지 (응답성 극대화)
        if (!isRunning) return; // 사용자가 중지 버튼을 누르면 즉각 루프 탈출
        delay(10); // 시스템 감시 회피 및 CPU 과점유 방지
    }
}

// ===================== 모드 1: 원형 회전 =====================
// 액추에이터를 순서대로 밀어 원형으로 회전하는 효과
void runMode1() {
    // 1→2→3→4 순서로 전진, 반대편 후진
    int phases[4][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };

    for (int p = 0; p < 4; p++) {
        if (!isRunning) return;
        setActuator(AIN1_1, AIN2_1, PWMA_1, phases[p][0], ACT_SPEED);
        setActuator(BIN1_1, BIN2_1, PWMB_1, phases[p][1], ACT_SPEED);
        setActuator(AIN1_2, AIN2_2, PWMA_2, phases[p][2], ACT_SPEED);
        setActuator(BIN1_2, BIN2_2, PWMB_2, phases[p][3], ACT_SPEED);
        smartDelay(PHASE_TIME_1);
    }
}

// ===================== 모드 2: 앞뒤 왕복 (웍질) =====================
// 1,3번 전진 + 2,4번 후진 → 반전 반복
void runMode2() {
    if (!isRunning) return;
    setActuator(AIN1_1, AIN2_1, PWMA_1, true,  ACT_SPEED);
    setActuator(BIN1_1, BIN2_1, PWMB_1, false, ACT_SPEED);
    setActuator(AIN1_2, AIN2_2, PWMA_2, true,  ACT_SPEED);
    setActuator(BIN1_2, BIN2_2, PWMB_2, false, ACT_SPEED);
    smartDelay(PHASE_TIME_2);

    if (!isRunning) return;
    setActuator(AIN1_1, AIN2_1, PWMA_1, false, ACT_SPEED);
    setActuator(BIN1_1, BIN2_1, PWMB_1, true,  ACT_SPEED);
    setActuator(AIN1_2, AIN2_2, PWMA_2, false, ACT_SPEED);
    setActuator(BIN1_2, BIN2_2, PWMB_2, true,  ACT_SPEED);
    smartDelay(PHASE_TIME_2);
}

// ===================== 모드 3: 교번 혼합 =====================
// 대각선 쌍으로 교대 동작
void runMode3() {
    if (!isRunning) return;
    // 1,4 전진 / 2,3 후진
    setActuator(AIN1_1, AIN2_1, PWMA_1, true,  ACT_SPEED);
    setActuator(BIN1_1, BIN2_1, PWMB_1, false, ACT_SPEED);
    setActuator(AIN1_2, AIN2_2, PWMA_2, false, ACT_SPEED);
    setActuator(BIN1_2, BIN2_2, PWMB_2, true,  ACT_SPEED);
    smartDelay(PHASE_TIME_3);

    if (!isRunning) return;
    // 반전
    setActuator(AIN1_1, AIN2_1, PWMA_1, false, ACT_SPEED);
    setActuator(BIN1_1, BIN2_1, PWMB_1, true,  ACT_SPEED);
    setActuator(AIN1_2, AIN2_2, PWMA_2, true,  ACT_SPEED);
    setActuator(BIN1_2, BIN2_2, PWMB_2, false, ACT_SPEED);
    smartDelay(PHASE_TIME_3);
}
