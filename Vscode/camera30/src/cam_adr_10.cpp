/*
Platform espressif32 @ 6.5.0
Libs: ArduinoJson 7.2.1, WebSockets 2.6.1

camera position avec occulaire 5000
avec camaera 23704
*/

#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_wifi.h"
#include <ESPmDNS.h>

// CONFIG HARDWARE
#define LED_PIN 33
#define LAMP_PIN 4
#define STREAM_INTERVAL 100

// CAMERA PINS (AI Thinker)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// GLOBALS
WebSocketsServer webSocket(82);
uint8_t wsClient = 255;
bool wsConnected = false;

unsigned long tStream = 0;
unsigned long tData = 0;
unsigned long tLed = 0;
unsigned long tprint = 0;

unsigned long cycleTime = 0;

int lampChannel = 7;
const int pwmFreq = 50000;
const int pwmResolution = 9;
const int pwmMax = (1 << pwmResolution) - 1;
int lampValue = 0;

bool otaInProgress = false;
bool otaRunning = false;

// LAMP CONTROL
void setLamp(int value)
{
  lampValue = constrain(value, 0, 100);
  if (lampValue == 0)
  {
    ledcWrite(lampChannel, 0);
    return;
  }
  int brightness = round((pow(2, (1 + (lampValue * 0.02))) - 2) / 6 * pwmMax);
  ledcWrite(lampChannel, brightness);
}

// CAMERA INIT
void initCamera()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 8;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK)
  {
    Serial.println("Camera init failed");
  }
}

// NETWORK INFO PRINT
void printNetworkInfo()
{
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
}

void initWiFi()
{

  IPAddress local_IP(192, 168, 4, 10);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);

  Serial.println();
  Serial.println("Connexion au point d'acces TELESCOPE...");

  WiFi.begin("TELESCOPE");

  int essais = 0;

  while (WiFi.status() != WL_CONNECTED && essais < 30)
  {
    delay(500);
    Serial.print(".");
    essais++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("ESP-CAM connectee !");
    Serial.print("ESP-CAM IP : ");
    Serial.println(WiFi.localIP());

    Serial.print("Passerelle : ");
    Serial.println(WiFi.gatewayIP());

    Serial.print("Signal RSSI : ");
    Serial.println(WiFi.RSSI());
  }
  else
  {
    Serial.println("ERREUR : impossible de se connecter a TELESCOPE");
  }
}

// WEBSOCKET EVENTS
void onWebSocket(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_CONNECTED:
    wsClient = num;
    wsConnected = true;
    break;
  case WStype_DISCONNECTED:
    wsConnected = false;
    break;
  case WStype_TEXT:
  {
    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, payload, length) != DeserializationError::Ok)
      return;
    if (doc.containsKey("command"))
    {
      const char *cmd = doc["command"];
      const char *p = strstr(cmd, "Cam1=");
      if (p != nullptr)
        setLamp(atoi(p + 5));
    }
    break;
  }
  default:
    break;
  }
}

// OTA INIT
void initOTA()
{
  ArduinoOTA.setHostname("camera"); // ⚠️ sans .local

  ArduinoOTA.onStart([]()
                     {
    otaInProgress = true;
    otaRunning = true;
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("Début OTA : " + type);

    // STOP WebSocket proprement
    if(wsConnected) { webSocket.disconnect(); wsConnected = false; }

    // STOP camera proprement
    esp_camera_deinit();

    // WiFi : priorité
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE); });

  ArduinoOTA.onEnd([]()
                   {
    otaInProgress = false;
    otaRunning = false;
    Serial.println("\nFin OTA.");
    // Réinitialiser caméra et WebSocket après OTA
    initCamera();
    webSocket.begin();
    webSocket.onEvent(onWebSocket); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("OTA Progression : %u%%\r", (progress * 100) / total); });

  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Erreur OTA[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive");
    else if (error == OTA_END_ERROR) Serial.println("End"); });

  ArduinoOTA.begin();
}

// STREAM CAMERA
void sendFrame()
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
    return;
  webSocket.sendBIN(wsClient, fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

// SEND JSON DATA
void sendData()
{
  StaticJsonDocument<128> doc;
  doc["temps_de_cycle"] = cycleTime;
  doc["rssi"] = WiFi.RSSI();
  doc["lampVal"] = lampValue;

  char buffer[128];
  size_t len = serializeJson(doc, buffer);
  webSocket.broadcastTXT(buffer, len);
}

// SETUP
void setup()
{
  uint32_t t0 = millis();
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // désactive brownout
  pinMode(LED_PIN, OUTPUT);

  // PWM LAMP
  ledcSetup(lampChannel, pwmFreq, pwmResolution);
  ledcAttachPin(LAMP_PIN, lampChannel);
  setLamp(0);

  initWiFi();
  WiFi.setSleep(false);

  initOTA();
  initCamera();

  webSocket.begin();
  webSocket.onEvent(onWebSocket);

  Serial.printf("=== Setup terminé en %lu ms ===\n", millis() - t0);
  Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
  Serial.printf("Free OTA space: %u\n", ESP.getFreeSketchSpace());
}

// LOOP
void loop()
{
  uint32_t startCycle = micros();

  ArduinoOTA.handle();
  if (otaRunning)
    delay(10);

  webSocket.loop();
  unsigned long now = millis();

  if (!otaInProgress && wsConnected && now - tStream > STREAM_INTERVAL)
  {
    tStream = now;
    sendFrame();
  }
  if (!otaInProgress && now - tData > 100)
  {
    tData = now;
    sendData();
  }
  if (WiFi.status() == WL_CONNECTED && now - tLed > 500)
  {
    tLed = now;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }

  yield(); // laisse le WiFi respirer
}