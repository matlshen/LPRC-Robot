#include "motor.h"
#include "Arduino.h"

void Motor::SetSpeed(int speed) {
    // Set the speed of the motor
    if (speed < 0) {
        analogWrite(pin1, 0);
        analogWrite(pin2, std::abs(speed));
    }
    else {
        analogWrite(pin1, std::abs(speed));
        analogWrite(pin2, 0);
    }
}

void Motor::Stop() {
    // Stop the motor
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
}