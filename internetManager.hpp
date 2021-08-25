#pragma once

#define INTERNET_MANAGER_DEFAULT_AP_SSID "Labino"
#define INTERNET_MANAGER_DEFAULT_AP_PASSWORD "biologia"

/*
    Takes in an AsyncWebServer pointer. It starts the device on access point wifi mode
    with default ssid and password. It adds to the server the path "setup" (and "setup_config"
    and "setup_status"). When accesing this path the user will be prompted a form with a new ssid
    and password. When submitted, it will try to connect to an existing network of that ssid and
    password in station mode. If it fails, the previous access point will be reset.
*/

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#ifdef DEBUG
#define PRINT(...) Serial.printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

#define CRE_SSID_MAX_SIZE 64
#define CRE_PASSWORD_MAX_SIZE 64

class InternetManager {
private:
    AsyncWebServer *_server;
    bool _internet = false;
    bool _change = false;
    char _ssid[CRE_SSID_MAX_SIZE] = {'\0'};
    char _password[CRE_PASSWORD_MAX_SIZE] = {'\0'};
public:
    InternetManager(AsyncWebServer *server) {
        _server = server;
        _server->on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, "text/html", InternetManager::setup_html);
        });
        _server->on("/setup_status", HTTP_GET, [this](AsyncWebServerRequest *request) {
            request->send(200, "application/json", String("{\"status\":")+String(_internet)+String("}"));
        });
        _server->on("/setup_config", HTTP_GET, [this](AsyncWebServerRequest *request) {
            if (request->hasParam("ssid") && request->hasParam("password")) {
                memset(_ssid, '\0', CRE_SSID_MAX_SIZE);
                request->getParam("ssid")->value().toCharArray(_ssid, CRE_SSID_MAX_SIZE);
                memset(_password, '\0', CRE_PASSWORD_MAX_SIZE);
                request->getParam("password")->value().toCharArray(_password, CRE_PASSWORD_MAX_SIZE);
                _change = true;
                PRINT("Changing wifi to\n\tssid: %s\n\tpassword %s\n", _ssid, _password);
            }
            request->send(200, "text/plain", "OK");
        });
    }
    void begin(bool beginServer=true) {
        PRINT("BEGIN\n");
        _internet = false;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(INTERNET_MANAGER_DEFAULT_AP_SSID, INTERNET_MANAGER_DEFAULT_AP_PASSWORD);
        PRINT("IPAdress: %s\n", WiFi.softAPIP().toString().c_str());
        if (beginServer) {
            _server->begin();
        }
    }
    String setupProcessor(String &var) {
        if (var == "STATUS") {
            return String((_internet ? "true" : "false"));
        }
        return String();
    }
    void tick() {
        if (_change) {
            _change = false;
            WiFi.disconnect();
            WiFi.mode(WIFI_STA);
            PRINT("Connecting to %s", _ssid);
            WiFi.begin(_ssid, _password);
            for (uint8_t i = 0; i < 40; i++) {
                if (WiFi.status() == WL_CONNECTED)
                    break;
                delay(500);
                PRINT(".");
            }
            _internet = WiFi.status() == WL_CONNECTED;
            if (_internet) {
                PRINT(" done\nIPAdress: %s\n", WiFi.localIP().toString().c_str());
            } else {
                PRINT(" error\n");
                WiFi.disconnect();
                begin();
            }
        }
    }

private:
    static constexpr char *setup_html PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Setup</title>
    <style>
        body {
            font-family: Calibri, sans-serif;
        }
        .content {
            margin: 15px 20px;
        }
    </style>
</head>
<body>
    <h1>Internet setup</h1>
    <div class="content">
        Internet status: <span id="status"></span><br><br>
        <form action="/setup_config" method="GET">
            ssid: <input type="text" name="ssid" required><br><br>
            password: <input type="password" name="password" required><br><br>
            <input type="submit" value="Set">
        </form>
    </div>
    <script>
        console.log('begin');
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if (xhr.readyState == XMLHttpRequest.DONE) {   // XMLHttpRequest.DONE == 4
                console.log(xhr.responseText);
                let status = JSON.parse(xhr.responseText).status;
                let span = document.getElementById("status");
                if (xhr.status == 200) {
                    span.innerHTML = String(Boolean(status));
                } else {
                    span.innerHTML = "unknown";
                }
                console.log(status);
            }
        };
        xhr.open("GET", "/setup_status", true);
        xhr.send();
    </script>
</body>
</html>)rawliteral";
};
