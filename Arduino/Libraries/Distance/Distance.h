/*
  Distance.h - Library for calculating distances
  Requires HC-SR04 sensosr
*/
#ifndef Distance_h
#define Distance_h 

#include "Arduino.h"

class Distance {
  public:
    Distance(int echoPin, int triggerPin);
    void begin();
    float measure();
  private:
    int _echoPin;
    int _triggerPin;
};

#endif
