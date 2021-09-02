#pragma once

#include <Arduino.h>

class AnalogSensor
{
private:
    uint8_t _pin;
    float _val;
public:
    float calibration(uint16_t rawVal);

    void setPin(uint8_t pin) { _pin = pin; }
    void read();
    float getVal(bool update=true);
};

float AnalogSensor::calibration(uint16_t rawVal) {
    return (rawVal*100) / 1024;
}

void AnalogSensor::read() {
    _val = calibration(analogRead(_pin));
}

float AnalogSensor::getVal(bool update) {
    if (update)
        read();
    return _val;
}
