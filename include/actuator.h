#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <Arduino.h>

void setupActuators();
void setActuator(int ain1, int ain2, int pwm_pin, bool forward, int speed);
void stopActuator(int ain1, int ain2, int pwm_pin);
void stopAll();

#endif // ACTUATOR_H
