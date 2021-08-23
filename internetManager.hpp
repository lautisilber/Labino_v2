#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncWebServer.h>

#define INTERNET_MANAGER_DEFAULT_AP_SSID "Labino"
#define INTERNET_MANAGER_DEFAULT_AP_PASSWORD "biologia"

class InternetManager {
private:
    AsyncWebServer *_server;
    bool _internet = false;

public:
    InternetManager(AsyncWebServer *server) {
        _server = server;
        _server->on("/setup", HTTP_GET, [] (AsyncWebServerRequest *request) {});
    }
    void begin() {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid, password);
        PRINT("IPAdress: %s", WiFi.softAPIP().getString().c_str());
    }
    String setupProcessor(String &var) {
        if (var == "STATUS") {
            return String((_internet ? "true" : "false"));
        }
        return String();
    }

private:
    constexpr char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
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
        Internet status: %STATUS%<br><br>
        <form action="/setup/config" method="GET">
            ssid: <input type="text" name="ssid" required><br><br>
            password: <input type="text" name="password" required><br><br>
            <input type="submit" value="Set">
        </form>
    </div>
</body>
</html>)rawliteral";
};
