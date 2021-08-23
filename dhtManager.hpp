#pragma once

#include <Arduino.h>
#include <DHT.h>
#include "softwareTimer.hpp"

#ifdef DEBUG
#define PRINT(...) Serial.printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

//#define ABS(X) ((X) > 0 ? (X) : -(X))
#define DHT_MIN_REFRESH_TIME 2100 // in milliseconds


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
    DHTManager(DHT *dht) {
        _dht = dht;
        _timer.writeTimer(DHT_MIN_REFRESH_TIME, true);
        _timer.restart();
        _timer.activate();
    }

    void begin() {
        _dht->begin();
    }

    void read() {
        if (_timer.tick()) {
            float hum = _dht->readHumidity();
            float temp = _dht->readTemperature();
            if (isnan(hum)) { PRINT("Couldn't read dht humidity"); }
            else { _hum = hum; }
            if (isnan(temp)) { PRINT("Couldn't read dht temperature"); }
            else { _temp = temp; }
        }
    }

    float getInfo(DHTInfo *dhtInfo) {
        dhtInfo->hum = _hum;
        dhtInfo->temp = _temp;
    }
};
