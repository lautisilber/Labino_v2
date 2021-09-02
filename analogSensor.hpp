#pragma once

#include <Arduino.h>

#ifdef ESP32
    #define ADC_RESOLUTION 4096
#else
    #define ADC_RESOLUTION 1024
#endif

class AnalogSensor
{
private:
    uint8_t _pin;
    float _val;
public:
    float calibration(uint32_t rawVal);

    void setPin(uint8_t pin) { _pin = pin; }
    void read();
    float getVal(bool update=true);
};

float AnalogSensor::calibration(uint32_t rawVal) {
    return (rawVal*100) / ADC_RESOLUTION;
}

void AnalogSensor::read() {
    _val = calibration(analogRead(_pin));
}

float AnalogSensor::getVal(bool update) {
    if (update)
        read();
    return _val;
}
