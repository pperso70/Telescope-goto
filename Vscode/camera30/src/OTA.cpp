#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_wifi.h"

const char *ssid = "freeboxwifi";
const char *password = "1234567890";

IPAddress localIP(192, 168, 0, 30);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

void setup()
{
    Serial.begin(115200);
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Configuration WiFi
    WiFi.mode(WIFI_STA);
    WiFi.config(localIP, gateway, subnet);
    WiFi.begin(ssid, password);

    Serial.print("Connexion WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connecté");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Important pour OTA
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE);

    // Hostname OTA
    ArduinoOTA.setHostname("esp32cam");

    ArduinoOTA.onStart([]()
    {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("Début OTA : " + type);
    });

    ArduinoOTA.onEnd([]()
    {
        Serial.println("\nOTA terminé !");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        Serial.printf("Progression: %u%%\r", (progress * 100) / total);
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
        Serial.printf("Erreur OTA [%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();

    Serial.println("OTA prêt");
}

void loop()
{
    ArduinoOTA.handle();
}