#ifndef MotorController_h
#define MotorController_h

#include "Arduino.h"

class MotorController {
    public:
        MotorController(int pin1, int pin2, int pin3, int pin4, int ena, int enb);
        void setup();
        void stop();
        void forward();
        void backward();
        void left();
        void right();
        void speed(int absolute_speed);
        void speed_change(int relative_speed);
    private:
        int _pin1;
        int _pin2;
        int _pin3;
        int _pin4;
        int _ena;
        int _enb;
        int _currentSpeed;
};

#endif
