#pragma once

#include <Arduino.h>

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
    SoftwareTimer(uint32_t time, bool reload);
    
    void writeTimer(uint32_t time, bool reload);
    void activate(bool state=true);
    void restart();
    bool tick();

    uint32_t getCurrMaxTime() const { return maxTime; }
};

SoftwareTimer::SoftwareTimer(uint32_t time /* in milliseconds */, bool reload) {
    writeTimer(time, reload);
    lastTime = millis();
}

void SoftwareTimer::writeTimer(uint32_t time /* in milliseconds */, bool reload) {
    maxTime = time;
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
