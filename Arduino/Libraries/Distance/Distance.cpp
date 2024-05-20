/*
  Distance.cpp - Library for using HC-SR04 distance sensor
*/

#include "Arduino.h"
#include "Distance.h"

Distance::Distance(int echoPin, int triggerPin) {
  _echoPin = echoPin;
  _triggerPin = triggerPin;
}

void Distance::begin() {
  pinMode(_echoPin, INPUT);
  pinMode(_triggerPin, OUTPUT);
  digitalWrite(_triggerPin, LOW);
}

float Distance::measure() {
  // put your main code here, to run repeatedly:
  digitalWrite(_triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(_triggerPin, LOW);
 
  long duration = pulseIn(_echoPin, HIGH);
  delay(250);
  return duration / 2 / 29.1;
}
