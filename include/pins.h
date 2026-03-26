#ifndef PINS_H
#define PINS_H

// ===================== 핀 정의 =====================
// TB6612 #1 (액추에이터 1, 2)
#define AIN1_1  25
#define AIN2_1  26
#define BIN1_1  27
#define BIN2_1  14
#define PWMA_1  32
#define PWMB_1  33

// TB6612 #2 (액추에이터 3, 4)
#define AIN1_2  13
#define AIN2_2  19
#define BIN1_2  18
#define BIN2_2  23
#define PWMA_2  4
#define PWMB_2  5

#define STBY    15

// 버튼
#define BTN_START  34
#define BTN_STOP   35
#define BTN_MODE   39
#define BTN_LED    12

// LED
#define LED_STATUS  2
#define LED_LIGHT   0  // MOSFET gate

// OLED I2C
#define SDA_PIN 21
#define SCL_PIN 22

#endif // PINS_H
