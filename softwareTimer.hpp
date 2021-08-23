#pragma once

#include <Arduino.h>

#define S_to_mS_FACTOR 1000
#define MAX_UINT32 (uint32_t)-1

class SoftwareTimer
{
private:
    uint32_t maxTime;
    uint32_t lastTime;
    bool autoReboot = false;
    bool active = false;

public:
    SoftwareTimer() {};
    SoftwareTimer(uint16_t time, bool reload);
    
    void writeTimer(uint16_t time, bool reload);
    void activate(bool state=true);
    void restart();
    bool tick();

    uint16_t getCurrMaxTime() const { return (int)(maxTime / S_to_mS_FACTOR); }
};

SoftwareTimer::SoftwareTimer(uint16_t time /* in seconds */, bool reload) {
    writeTimer(time, reload);
    lastTime = millis();
}

void SoftwareTimer::writeTimer(uint16_t time /* in seconds */, bool reload) {
    maxTime = time * S_to_mS_FACTOR;
    autoReboot = reload;
}

void SoftwareTimer::activate(bool state) {
    if (state == true && active == false)
        restart();
    active = state;
}

void SoftwareTimer::restart() {
    lastTime = millis();
}

bool SoftwareTimer::tick() {
    uint32_t currTime = millis();
    bool unfilteredTick;
    if (currTime > lastTime)
        // everything is normal
        unfilteredTick = currTime - lastTime > maxTime;
    else
        // millis() looped back to 0
        unfilteredTick = currTime + (MAX_UINT32 - lastTime) > maxTime;

    bool tick = unfilteredTick && active && (currTime != lastTime);

    if (tick) {
        lastTime = millis();
        if (!autoReboot)
            activate(false);
    }

    return tick;
}
