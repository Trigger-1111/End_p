#include "../include/display.h"
#include "../include/pins.h"
#include "../include/globals.h"
#include <Wire.h>
#include <U8g2lib.h>

// SSH1106 1.3인치
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);

void setupDisplay() {
    Wire.begin(SDA_PIN, SCL_PIN);
    u8g2.begin();
    updateDisplay();
}

void updateDisplay() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    // 상태
    u8g2.drawStr(0, 12, isRunning ? "[ RUNNING ]" : "[  STOP   ]");

    // 모드명
    u8g2.setFont(u8g2_font_ncenB14_tr);
    switch (currentMode) {
        case 1: u8g2.drawStr(0, 38, "MODE 1"); break;
        case 2: u8g2.drawStr(0, 38, "MODE 2"); break;
        case 3: u8g2.drawStr(0, 38, "MODE 3"); break;
        case 4: u8g2.drawStr(0, 38, "MODE 4"); break;
    }

    // 모드 설명
    u8g2.setFont(u8g2_font_ncenB08_tr);
    switch (currentMode) {
        case 1: u8g2.drawStr(0, 56, "Circular"); break;
        case 2: u8g2.drawStr(0, 56, "Wok"); break;
        case 3: u8g2.drawStr(0, 56, "Alternate"); break;
        case 4: u8g2.drawStr(0, 56, "AI Mode"); break;
    }

    // 조명 상태
    u8g2.drawStr(90, 56, ledLight ? "LED:ON" : "LED:OF");

    u8g2.sendBuffer();
}
