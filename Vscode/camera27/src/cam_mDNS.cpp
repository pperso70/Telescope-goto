/*
github\Vscode\camera27> pio pkg list
Resolving esp32cam dependencies...
Platform espressif32 @ 6.5.0 (required: espressif32 @ 6.5.0)
├── framework-arduinoespressif32 @ 3.20014.231204 (required: platformio/framework-arduinoespressif32 @ ~3.20014.0)
├── tool-esptoolpy @ 1.40501.0 (required: platformio/tool-esptoolpy @ ~1.40501.0)
├── tool-mkfatfs @ 2.0.1 (required: platformio/tool-mkfatfs @ ~2.0.0)
├── tool-mklittlefs @ 1.203.210628 (required: platformio/tool-mklittlefs @ ~1.203.0)
├── tool-mkspiffs @ 2.230.0 (required: platformio/tool-mkspiffs @ ~2.230.0)
├── tool-openocd-esp32 @ 2.1100.20220706 (required: platformio/tool-openocd-esp32 @ ~2.1100.0)
├── toolchain-riscv32-esp @ 8.4.0+2021r2-patch5 (required: espressif/toolchain-riscv32-esp @ 8.4.0+2021r2-patch5)
└── toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5 (required: espressif/toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5)

Libraries
├── ArduinoJson @ 7.4.3 (required: bblanchon/ArduinoJson @ ^7.1.0)
├── ESPAsyncWebServer @ 3.6.0+sha.ad3741d (required: git+https://github.com/me-no-dev/ESPAsyncWebServer.git)
│   └── AsyncTCP @ 3.4.10 (required: ESP32Async/AsyncTCP @ ^3.3.2)
└── WebSockets @ 2.7.3 (required: links2004/WebSockets @ ^2.6.0)
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

// WIFI CREDENTIALS
struct WifiCredential
{
  const char *ssid;
  const char *password;
};


// Liste à adapter selon vos réseaux
WifiCredential knownNetworks[] = {
    {"reseau1", "password1"},        //noms des resaux habituels le systeme prend le Wifi le plus fort parmi ceux ci
    {"reseau2", "password2"},
    {"reseau3", "password3"},
    {"reseau4", "password4"},
    {"reseau5", "password5"}};


const int knownCount = sizeof(knownNetworks) / sizeof(knownNetworks[0]);

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

// WIFI + MDNS INIT
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  Serial.println("Scan des réseaux WiFi...");
  int n = WiFi.scanNetworks();

  if (n == 0)
  {
    Serial.println("Aucun réseau trouvé.");
    return;
  }

  int bestRssi = -999;
  int bestIndex = -1;

  // 🔎 Recherche du meilleur réseau connu
  for (int i = 0; i < n; ++i)
  {
    Serial.printf("Réseau trouvé: %s (RSSI: %d dBm)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    for (int j = 0; j < knownCount; ++j)
    {
      if (WiFi.SSID(i) == knownNetworks[j].ssid && WiFi.RSSI(i) > bestRssi)
      {
        bestRssi = WiFi.RSSI(i);
        bestIndex = j;
      }
    }
  }

  if (bestIndex == -1)
  {
    Serial.println("Aucun réseau connu trouvé.");
    return;
  }

  const char *selectedSsid = knownNetworks[bestIndex].ssid;
  const char *selectedPassword = knownNetworks[bestIndex].password;

  Serial.printf("Connexion à %s (RSSI: %d)\n", selectedSsid, bestRssi);
  WiFi.begin(selectedSsid, selectedPassword);

  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Échec de connexion au réseau.");
    return;
  }

  Serial.println("Connecté en DHCP :");
  printNetworkInfo();

  // 🔵 Activation mDNS
  const char *hostname = "camera"; // camera.local
  if (MDNS.begin(hostname))
  {
    Serial.printf("mDNS démarré: http://%s.local\n", hostname);
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 82);
  }
  else
  {
    Serial.println("Erreur démarrage mDNS !");
  }

  WiFi.setSleep(false);
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

  if (now - tprint > 500)
  {
    tprint = now;
    Serial.println(ESP.getFreeHeap());
    Serial.printf("=== T cycle en %lu us ===\n", micros() - startCycle);
  }

  static unsigned long tAlive = 0;
  if (now - tAlive > 2000)
  {
    Serial.println("OTA alive");
    tAlive = now;
  }

  yield(); // laisse le WiFi respirer
}