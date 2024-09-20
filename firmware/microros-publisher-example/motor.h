#pragma once

class Motor {
public:
    Motor(int pin1, int pin2) : 
        pin1(pin1), pin2(pin2) {}
    void SetSpeed(int speed);
    void Stop();
private:
    int pin1;
    int pin2;
};