#pragma once

#include <Arduino.h>

class AnalogSensor
{
private:
    uint8_t _pin;
    float _val;
public:
    AnalogSensor();
    AnalogSensor(uint8_t pin) { _pin = pin; }

    virtual float calibration(uint16_t rawVal) = 0;

    void setPin(uint8_t pin) { _pin = pin; }
    void read();
    float getVal(bool update=true);
};

void AnalogSensor::read() {
    _val = calibration(analogRead(_pin));
}

float AnalogSensor::getVal(bool update) {
    if (update)
        read();
    return _val;
}