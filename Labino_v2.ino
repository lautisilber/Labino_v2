/*
  Hay que crear un archivo que se llame credentials.hpp ne el directorio de este archivo con el siguiente contenido:
  
  """

  #pragma once

  #include <Arduino.h>
  
  const char *ssid = "EL_NOMBRE_DE_LA_RED_WIFI";
  const char *password = "LA_CONSTRASEÑA_DE_LA_RED_WIFI";
  const char *dropbox_token = "EL_TOKEN_DE_LA_APPLICACION_DROPBOX";

  """

  
*/

#include <WiFi.h>
#include <DHT.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// si definimos DEBUG, se escribirán mensajes a la consola
#define DEBUG

// incluimos las librerías del proyecto
#include "credentials.hpp"
#include "debug_utils.hpp"
#include "analogSensor.hpp"
#include "dhtManager.hpp"
#include "dropboxSDK.hpp"
#include "NTPManager.hpp"
#include "web.hpp"


#define SOIL_MOISTURE_AMOUNT  3                 // cantidad de sensores de humedad de suelo
#define DHT_PIN               27                // pin al que está conectado el sensor dht22
#define SENSOR_TIME           60*1000           // tiempo entre mediciones (en milisegundos)
#define LOG_COUNT_UPDATE      1000              // cuantas mediciones se pueden guardar antes de que se traten de subir a dropbox
#define LOG_FILE              "/log.txt"        // nombre interno del archivo donde se guardan las mediciones
#define DBX_LOG_DESTINATION   "/logs/data.log"  // nombre del archivo de dropbox a donde se gardarán los archivos

const uint8_t soilMoisturePins[SOIL_MOISTURE_AMOUNT] = {34, 34, 34}; // número de pines a donde están conectados los sensores de humedad
                                                                     // (el largo de esta lista debe coincidir con SOIL_MOISTURE_AMOUNT

const char *ap_ssid     = "Labino";     // la ssid de la red wifi generada por el esp32
const char *ap_password = "biologia";   // la contraseña de la red wifi generada por el esp32

// objeto de sensor de humedad de suelo, que permite definir una función de calibración para estos sensores. Se puede también definir
class MoistSensor : public AnalogSensor {
public:
  float calibration(uint16_t rawVal) {
    return (rawVal*100) / 1024;
  }
};


/* declaración de funciones y variables */
bool sense(AnalogSensor *mSensor, DHTManager *dhtManager, NTPManager *ntp);
bool log(Dropbox *dbx);

AnalogSensor moistSensors[SOIL_MOISTURE_AMOUNT];

DHT dhtObj(DHT_PIN, DHT22);
DHTManager dht(&dhtObj);

Dropbox dropbox;
bool dbxUpload = true;

NTPManager ntpManager;

SoftwareTimer sensTimer(SENSOR_TIME, true);
uint32_t storedLogs = 0;

AsyncWebServer server(80);

void setup() {
  // si se está en modo debug, iniciar serial
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  // iniciar el sistema de archivos spiffs
  if(!SPIFFS.begin()) {
    PRINT("SPIFFS monut failed\n");
    delay(5000);
    ESP.restart();
  }

  /* WIFI */
  // modo punto de acceso + estacion
  WiFi.mode(WIFI_AP_STA);
  // configuración del punto de acceso (red generada)
  WiFi.softAP(ap_ssid, ap_password);
  // configuración de la red wifi del lugar
  PRINT("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    PRINT(".");
  }
  PRINT("done\nIP address: %s\nsoft AP IP adderss: %s\n", WiFi.localIP().toString().c_str(), WiFi.softAPIP().toString().c_str());


  // inicializar variables
  for (size_t i = 0; i < SOIL_MOISTURE_AMOUNT; i++) {
    moistSensors[i].setPin(soilMoisturePins[i]);
  }
  dht.begin();
  dropbox.begin(dropbox_token);
  ntpManager.begin();
  sensTimer.activate();

  // definir direcciones del servidor
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->redirect("/index"); });
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(200, "text/html", index_html); });
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(200, "text/html", download_html); });
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(200, "text/html", settings_html); });
  server.on("/setsettings", HTTP_GET, [dbxUpload](AsyncWebServerRequest *request){
    int paramsNr = request->params();
    for(int i=0;i<paramsNr;i++){
      AsyncWebParameter* p = request->getParam(i);
      if (p->name() == "dbxUpload") {
        dbxUpload = (bool)p->value().toInt();
      }
    }
    PRINT("Set settings\n\tdbxUpload: %u\n", (uint8_t)dbxUpload);
    request->redirect("/settings");
  });
  server.on("/getsettings", HTTP_GET, [dbxUpload](AsyncWebServerRequest *request){
    String settings = String("{\"dbxUpload\":") + String(dbxUpload) + String(",\"internet\":") + String(WiFi.status() == WL_CONNECTED) + String("}");
    PRINT("Requested settings: '%s'\n", settings.c_str());
    request->send(200, "application/json", settings);
  });
  server.on("/logfile", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("preview")) {
      if (request->getParam("preview")->value().toInt()) {
        request->send(SPIFFS, LOG_FILE, "text/plain", false);
        return;
      }
    }
    if (request->hasParam("download")) {
      if (request->getParam("download")->value().toInt()) {
        if (!SPIFFS.exists(LOG_FILE)) { request->send(200, "text/plain", "Log file is empty"); }
        else { request->send(SPIFFS, LOG_FILE, "text/plain", true); }
        return;
      }
    }
    if (request->hasParam("delete")) {
      if (request->getParam("delete")->value().toInt()) {
        SPIFFS.remove(LOG_FILE);
        request->send(200, "text/plain", "Deleted log file");
        return;
      }
    }
    request->send(400, "text/plain", "error");
  });


  // iniciar servidor
  server.begin();
}

void loop() {
  if (sensTimer.tick()) { // esperar al momento indicado para medir
    storedLogs += sense(moistSensors, &dht, &ntpManager);
  }
  if (storedLogs >= LOG_COUNT_UPDATE && dbxUpload) { // ver si es momento de subir a dropbox (y si hay que hacerlo)
    bool uploadSuccess = log(&dropbox);
    if (uploadSuccess)
      storedLogs = 0;
  }
}

bool sense(AnalogSensor *mSensor, DHTManager *dhtManager, NTPManager *ntp) {
  // mide todos los valores de los sensores y los guarda en formato json en el archivo de nombre LOG_FILE
  
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
  // intenta subir a dropbox al alrchivo DBX_LOG_DESTINATION el archivo local LOG_FILE. si lo logra, borra el archivo local para que no se vuelva demasiado pesado
  
  bool success = dbx->uploadFile(SPIFFS, LOG_FILE, false, DBX_LOG_DESTINATION);
  if (success) {
    SPIFFS.remove(LOG_FILE);
  }
  PRINT("uploading file '%s': %s\n", LOG_FILE, (success ? "success" : "error"));
  return success;
}
