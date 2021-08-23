#pragma once

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

#define INTERNET_MANAGER_DEFAULT_AP_SSID "Labino"
#define INTERNET_MANAGER_DEFAULT_AP_PASSWORD "biologia"
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
        _server->on("/setup/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
            request->send(200, "text/html", String("{\"status\":")+String(_internet));
        });
        _server->on("/setup/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
            if (request->hasParam("ssid") && request->hasParam("password")) {
                memset(_ssid, CRE_SSID_MAX_SIZE, '\0');
                request->getParam("ssid")->value().toCharArray(_ssid, CRE_SSID_MAX_SIZE);
                memset(_password, CRE_PASSWORD_MAX_SIZE, '\0');
                request->getParam("password")->value().toCharArray(_password, CRE_PASSWORD_MAX_SIZE);
                _change = true;
            }
            request->redirect("/setup");
        });
    }
    void begin() {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(INTERNET_MANAGER_DEFAULT_AP_SSID, INTERNET_MANAGER_DEFAULT_AP_PASSWORD);
        PRINT("IPAdress: %s", WiFi.softAPIP().getString().c_str());
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
                if (WiFi.status() != WL_CONNECTED)
                    break;
                delay(500);
                Serial.println(".");
            }
            _internet = WiFi.status() == WL_CONNECTED;
            if (_internet) {
                PRINT(" done\n");
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
        <form action="/setup/config" method="GET">
            ssid: <input type="text" name="ssid" required><br><br>
            password: <input type="text" name="password" required><br><br>
            <input type="submit" value="Set">
        </form>
    </div>
</body>
<script>
    function toggleCheckbox(element) {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if (xhr.readyState == XMLHttpRequest.DONE) {   // XMLHttpRequest.DONE == 4
                let status = JSON.parse(xhr.responseText).status
                let span = document.getElementById("status");
                if (xhr.status == 200) {
                    span.innerHTML = String(Boolean(status));
                } else {
                    span.innerHTML = "unknown";
                }
            }
        };
        xhr.open("GET", "/setup/status", true);
        xhr.send();
    }
</script>
</html>)rawliteral";
};
