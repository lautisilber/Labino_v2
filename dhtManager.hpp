#pragma once

#include <Arduino.h>
#include <DHT.h>
#include "softwareTimer.hpp"

//#define ABS(X) ((X) > 0 ? (X) : -(X))
#define DHT_MIN_REFRESH_TIME 2100; // in milliseconds


struct DHTInfo {
    float hum;
    float temp;
};


class DHTManager {
private:
    DHT *_dht;
    float _hum = -127;
    float _temp = -127;
    SoftwareTimer _timer;
public:
    DHTManager(const DHT *dht) {
        _dht = dht;
        _timer.writeTime(DHT_MIN_REFRESH_TIME, true);
        _timer.restart();
        _timer.activate();
    }

    void begin() {
        _dht->begin();
    }

    void read() {
        if (timer.tick()) {
            float hum = _dht->readHumidity();
            float temp = _dht->readTemperature();
            if (isnan(hum)) { PRINT("Couldn't read dht humidity"); }
            else { _hum = hum; }
            if (isnan(temp)) { PRINT("Couldn't read dht temperature"); }
            else { _temp = temp; }
            _lastRead = millis();
        }
    }

    float getInfo(DHTInfo *dhtInfo) {
        dhtInfo->hum = _hum;
        dhtInfo->temp = _temp;
    }
};
