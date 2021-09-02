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
#define SENSOR_TIME 0.25*1000//10 * 1000 // milliseconds
#define LOG_COUNT_UPDATE 1000 // how many logs can be stored before uploading them
#define LOG_FILE "/log.txt"
#define DBX_LOG_DESTINATION "/logs/data.log"


const uint8_t soilMoisturePins[SOIL_MOISTURE_AMOUNT] = {34, 34, 34};

class MoistSensor : public AnalogSensor {
public:
  float calibration(uint16_t rawVal) {
    return (rawVal*100) / 1024;
  }
};

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

bool sense(MoistSensor *mSensor, DHTManager *dhtManager, NTPManager *ntp);
bool log(Dropbox *dbx);

MoistSensor moistSensors[SOIL_MOISTURE_AMOUNT];

DHT dhtObj(DHT_PIN, DHT22);
DHTManager dht(&dhtObj);

Dropbox dropbox;

NTPManager ntpManager;

SoftwareTimer sensTimer(SENSOR_TIME, true);
uint32_t storedLogs = 0;

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
}

void loop() {
  if (sensTimer.tick()) {
    storedLogs += sense(moistSensors, &dht, &ntpManager);
  }
  if (storedLogs >= LOG_COUNT_UPDATE) {
    bool uploadSuccess = log(&dropbox);
    if (uploadSuccess)
      storedLogs = 0;
  }
}

bool sense(MoistSensor *mSensor, DHTManager *dhtManager, NTPManager *ntp) {
  PRINT("Logging... ");

  const TimeStamp *timeStamp = ntp->getTimeStamp();
  const DHTData *dhtData = dhtManager->getData();
  StaticJsonDocument<512> json;
  json["time"] = timeStamp->timeStr;
  JsonArray moistArray = json.createNestedArray("soil");
  for (size_t i = 0; i < SOIL_MOISTURE_AMOUNT; i++) {
    moistArray.add(mSensor[i].getVal());
  }
  json["hum"] = dhtData->hum;
  json["temp"] = dhtData->temp;
  char buff[512] = {'\0'};
  serializeJson(json, buff, 512);
  
  File file = SPIFFS.open(LOG_FILE, FILE_APPEND);
  if(!file){
      PRINT("failed to open file for logging");
      return false;
  }

  bool success;
  if(!file.println(buff)){
    PRINT("Failed to save sensor values\n");
    success = false;
  } else {
    PRINT("done\n");
    success = true;
  }
  file.close();
  return success;
}

bool log(Dropbox *dbx) {
  readFile(SPIFFS, LOG_FILE);
  bool success = dbx->uploadFile(SPIFFS, LOG_FILE, false, DBX_LOG_DESTINATION);
  if (success) {
    SPIFFS.remove(LOG_FILE);
  }
  PRINT("uploading file: %s\n", (success ? "success" : "error"));
  return success;
}
