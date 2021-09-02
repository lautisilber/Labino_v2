#pragma once

#include <Arduino.h>
#include <time.h>

#define NTP_SERVER_URL "pool.ntp.org"
#define NTP_GMT_OFFSET_SEC -3600*3
#define NTP_DAYLIGHT_OFFSET_SEC 0

struct TimeStamp
{
    char timeStr[20] = {'\0'};
    
    void timeFromTM(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
        snprintf(timeStr, 20, "%04d-%02d-%02d %02d:%02d:%02d", year+1900, month+1, day, hour, minute, second);
    }
};


class NTPManager
{
private:
    struct tm _timeInfo;
public:
    void begin();
    bool getTimeStamp(TimeStamp *timeStamp);
};

void NTPManager::begin() {
    configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER_URL);
}

bool NTPManager::getTimeStamp(TimeStamp *timeStamp) {
    if (!getLocalTime(&_timeInfo)) {
        PRINT("Couldn't get ntp time");
        return false;
    }
    timeStamp->timeFromTM(_timeInfo.tm_year, _timeInfo.tm_mon, _timeInfo.tm_mday, _timeInfo.tm_hour, _timeInfo.tm_min, _timeInfo.tm_sec);
    return true;
}
