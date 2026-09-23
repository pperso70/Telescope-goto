#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid="freeboxwifi";
const char* password="1234567890";

void setup()
{
Serial.begin(115200);

WiFi.begin(ssid,password);

while(WiFi.status()!=WL_CONNECTED) delay(500);

ArduinoOTA.setHostname("esp32cam");
ArduinoOTA.begin();

Serial.println("OTA ready");
}

void loop()
{
ArduinoOTA.handle();
}