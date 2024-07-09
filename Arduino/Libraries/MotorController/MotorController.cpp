/*
  Library for a L298N Microcrontroller
*/

#include "MotorController.h"
#include "Arduino.h"

MotorController::MotorController(int pin1, int pin2, int pin3, int pin4, int ena, int enb) {
    _pin1 = pin1;
    _pin2 = pin2;
    _pin3 = pin3;
    _pin4 = pin4;
    _ena = ena;
    _enb = enb;
}

void MotorController::setup() {
    pinMode(_pin1, OUTPUT);
    pinMode(_pin2, OUTPUT);
    pinMode(_pin3, OUTPUT);
    pinMode(_pin4, OUTPUT);
    pinMode(_ena, OUTPUT);
    pinMode(_enb, OUTPUT);
}

void MotorController::stop() {
    digitalWrite(_pin1, LOW);
    digitalWrite(_pin2, LOW);
    digitalWrite(_pin3, LOW);
    digitalWrite(_pin4, LOW);
}

void MotorController::forward() {
  digitalWrite(_pin1, HIGH);
  digitalWrite(_pin2, HIGH);
  digitalWrite(_pin3, LOW);
  digitalWrite(_pin4, LOW);
}

void MotorController::backward() {
  digitalWrite(_pin1, LOW);
  digitalWrite(_pin2, LOW);
  digitalWrite(_pin3, HIGH);
  digitalWrite(_pin4, HIGH);
}

void MotorController::left() {
    digitalWrite(_pin1, HIGH);
    digitalWrite(_pin2, LOW);
    digitalWrite(_pin3, LOW);
    digitalWrite(_pin4, HIGH);
}

void MotorController::right() {
    digitalWrite(_pin1, LOW);
    digitalWrite(_pin2, HIGH);
    digitalWrite(_pin3, HIGH);
    digitalWrite(_pin4, LOW);
}

void MotorController::speed(int absolute_speed) {
    if (absolute_speed > 250) absolute_speed = 250;
    else if (absolute_speed < 0) absolute_speed = 0;

    analogWrite(_ena, absolute_speed);
    analogWrite(_enb, absolute_speed);
    _currentSpeed = absolute_speed;
}

void MotorController::speed_change(int relative_speed) {
    int newSpeed = _currentSpeed + relative_speed;
    speed(newSpeed);
}
