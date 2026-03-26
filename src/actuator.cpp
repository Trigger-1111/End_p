#include "../include/actuator.h"
#include "../include/pins.h"

void setupActuators() {
    // 모터 드라이버 핀
    int outPins[] = {AIN1_1, AIN2_1, PWMA_1, BIN1_1, BIN2_1, PWMB_1,
                     AIN1_2, AIN2_2, PWMA_2, BIN1_2, BIN2_2, PWMB_2, STBY};
    for (int pin : outPins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
    digitalWrite(STBY, HIGH);  // TB6612 활성화
}

void setActuator(int ain1, int ain2, int pwm_pin, bool forward, int speed) {
    if (forward) {
        digitalWrite(ain1, HIGH);
        digitalWrite(ain2, LOW);
    } else {
        digitalWrite(ain1, LOW);
        digitalWrite(ain2, HIGH);
    }
    analogWrite(pwm_pin, speed);
}

void stopActuator(int ain1, int ain2, int pwm_pin) {
    digitalWrite(ain1, LOW);
    digitalWrite(ain2, LOW);
    analogWrite(pwm_pin, 0);
}

void stopAll() {
    stopActuator(AIN1_1, AIN2_1, PWMA_1);
    stopActuator(BIN1_1, BIN2_1, PWMB_1);
    stopActuator(AIN1_2, AIN2_2, PWMA_2);
    stopActuator(BIN1_2, BIN2_2, PWMB_2);
}
