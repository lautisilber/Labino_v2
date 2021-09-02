#include <WiFi.h>
#include <DHT.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define DEBUG

#include "credentials.hpp"
#include "debug_utils.hpp"
#include "analogSensor.hpp"
#include "dhtManager.hpp"
#include "dropboxSDK.hpp"
#include "NTPManager.hpp"

#define SOIL_MOISTURE_AMOUNT 3
#define DHT_PIN 27
#define SENSOR_TIME 15*1000//10 * 1000 // milliseconds
#define LOG_TIME 60*1000//10*60 * 1000 // milliseconds
#define LOG_FILE "/log.txt"
#define DBX_LOG_DESTINATION "/logs/data.log"


const uint8_t soilMoisturePins[SOIL_MOISTURE_AMOUNT] = {34, 34, 34};

class MoistSensor : public AnalogSensor {
public:
  float calibration(uint16_t rawVal) {
    return (rawVal*100) / 1024;
  }
};

void sense(MoistSensor *mSensor, DHTManager *dhtManager, NTPManager *ntp);
void log(Dropbox *dbx);

MoistSensor moistSensors[SOIL_MOISTURE_AMOUNT];

DHT dhtObj(DHT_PIN, DHT22);
DHTManager dht(&dhtObj);

Dropbox dropbox;

NTPManager ntpManager;

SoftwareTimer sensTimer(SENSOR_TIME, true);
SoftwareTimer logTimer(LOG_TIME, true);

AsyncWebServer server(80);

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  if(!SPIFFS.begin()) {
    PRINT("SPIFFS monut failed\n");
    delay(5000);
    ESP.restart();
  }

  PRINT("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    PRINT(".");
  }
  PRINT("done\nIP address: %s\n", WiFi.localIP().toString().c_str());

  for (size_t i = 0; i < SOIL_MOISTURE_AMOUNT; i++) {
    moistSensors[i].setPin(soilMoisturePins[i]);
  }

  dht.begin();
  dropbox.begin(dropbox_token);
  ntpManager.begin();

  sensTimer.activate();
  logTimer.activate();
}

void loop() {
  if (sensTimer.tick()) {
    sense(moistSensors, &dht, &ntpManager);
  }
  if (logTimer.tick()) {
    log(&dropbox);
  }
}

void sense(MoistSensor *mSensor, DHTManager *dhtManager, NTPManager *ntp) {
  PRINT("Logging... ");

  TimeStamp timeStamp;
  ntp->getTimeStamp(&timeStamp);
  DHTInfo dhtInfo;
  dhtManager->getInfo(&dhtInfo);
  StaticJsonDocument<512> json;
  json["time"] = timeStamp.timeStr;
  JsonArray moistArray = json.createNestedArray("soil");
  for (size_t i = 0; i < SOIL_MOISTURE_AMOUNT; i++) {
    moistArray.add(mSensor[i].getVal());
  }
  json["hum"] = dhtInfo.hum;
  json["temp"] = dhtInfo.temp;
  char buff[512] = {'\0'};
  serializeJson(json, buff, 512);
  
  File file = SPIFFS.open(LOG_FILE, FILE_APPEND);
  if(!file){
      PRINT("failed to open file for logging");
      return;
  }
  if(!file.println(buff)){
    PRINT("Failed to save sensor values\n");
  } else {
    PRINT("done\n");
  }
  file.close();
}

void log(Dropbox *dbx) {
  bool success = dbx->uploadFile(SPIFFS, LOG_FILE, false, DBX_LOG_DESTINATION);
  if (success) {
    SPIFFS.remove(LOG_FILE);
  }
  PRINT("uploading file: %s", (success ? "success" : "error"));
}
