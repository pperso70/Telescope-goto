
//=========================================================
// Pour CNC3, choix astre par IR touche 1, OTA, acces stellarium par HTTPClient http, limite ref verticale
//=========================================================
// dans stellarium activer pilotage teleccope et contole a ditance puis redemarrer

/*
    Pour charger le petit fichiers des etoiles et planetes star.cvs   dans VScode, Plateformio: new terminal,   entrer: pio run --target uploadfs
    fonctionne par port com ou OTA.

    Dans Stellarium, configuration generale(F2) puis Plungins, commande a distance,  cocher activer au demarrage,
    puis configurer: cocher serveur activer et activer au demarrage, port 8090


    stellarium tablette  hote: teles.local port 4030

    skysafari choisir meade LX200 classic  teles.local port 4031


    github\Vscode\Telescope27> pio pkg list
    Resolving esp32dev dependencies...
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
    ├── ArduinoJson @ 6.21.5 (required: bblanchon/ArduinoJson @ ^6.21.3)
    ├── AsyncHTTPRequest_Generic @ 1.13.0 (required: khoih-prog/AsyncHTTPRequest_Generic @ ^1.13.0)
    │   ├── AsyncTCP @ 3.3.2 (required: me-no-dev/AsyncTCP @ >=1.1.1)
    │   ├── ESPAsync_WiFiManager @ 1.15.1 (required: khoih-prog/ESPAsync_WiFiManager @ >=1.15.1)
    │   │   └── ESP_DoubleResetDetector @ 1.3.2 (required: khoih-prog/ESP_DoubleResetDetector @ >=1.3.2)
    │   ├── WebServer_ESP32_ENC @ 1.5.3 (required: khoih-prog/WebServer_ESP32_ENC @ >=1.5.3)
    │   ├── WebServer_ESP32_W5500 @ 1.5.3 (required: khoih-prog/WebServer_ESP32_W5500 @ >=1.5.3)
    │   ├── WebServer_ESP32_W6100 @ 1.5.3 (required: khoih-prog/WebServer_ESP32_W6100 @ >=1.5.3)
    │   └── WebServer_WT32_ETH01 @ 1.5.1 (required: khoih-prog/WebServer_WT32_ETH01 @ >=1.5.1)
    ├── AsyncTCP @ 3.3.2+sha.ef448a8 (required: git+https://github.com/me-no-dev/AsyncTCP.git)
    ├── DIYables_IRcontroller @ 1.0.1 (required: diyables/DIYables_IRcontroller @ ^1.0.0)
    │   └── IRremote @ 4.3.0 (required: IRremote @ =4.3.0)
    ├── ESPAsyncWebServer @ 3.6.0+sha.ad3741d (required: git+https://github.com/me-no-dev/ESPAsyncWebServer.git)
    │   └── AsyncTCP @ 3.4.10 (required: ESP32Async/AsyncTCP @ ^3.3.2)
    ├── FastAccelStepper @ 0.30.14 (required: gin66/FastAccelStepper @ 0.30.14)
    ├── JPEGDecoder @ 2.0.0 (required: bodmer/JPEGDecoder @ ^2.0.0)
    ├── NTPClient @ 3.2.1 (required: arduino-libraries/NTPClient @ ^3.2.1)
    ├── SiderealPlanets @ 1.6.0 (required: davidarmstrong/SiderealPlanets @ ^1.4.0)
    ├── TFT_eFEX @ 0.0.8 (required: bodmer/TFT_eFEX @ ^0.0.8)
    ├── TFT_eSPI @ 2.5.43 (required: bodmer/TFT_eSPI @ ^2.5.43)
    └── WebSockets @ 2.7.3 (required: WebSockets)

*/
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h> // ajout le 13/3/25
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <TFT_eFEX.h>
#include "ArduinoJson.h"
#include "webpage.h"
#include <TFT_eSPI.h> // Hardware-specific library
#include "FastAccelStepper.h"
#include <ArduinoOTA.h>
#include <SiderealPlanets.h>
#include <WebSocketsClient.h>
#include <pgmspace.h>
#include <LittleFS.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Preferences.h>
#include <HTTPClient.h>

#include <IRController.h>
#include "esp_wifi.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>

// #include "Profiler.h"
IRController irController(13, 200);

Preferences prefs;

AsyncWebServer server(80); // serveur web

WiFiServer stellariumServer(4030); // Stellarium
WiFiServer skySafariServer(4031);  // SkySafari

WiFiClient stellariumClient;
WiFiClient skySafariClient;

WiFiServer stellariumPcServer(10000); // conservé uniquement pour compatibilité code, NON UTILISÉ
WiFiClient stellariumPcClient;
WebSocketsServer webSocket = WebSocketsServer(81); // comm avec PC

TFT_eSPI tft = TFT_eSPI();               // Invoke custom library
TFT_eSprite sprite1 = TFT_eSprite(&tft); // Sprite object for dial
TFT_eFEX fex = TFT_eFEX(&tft);
SiderealPlanets myAstro;
SiderealPlanets myAstro2;

#define MAX_STARS 50
#define dirPinStepper_azi 16  // 32
#define stepPinStepper_azi 26 // 33

#define dirPinStepper_alt 27  // 26
#define stepPinStepper_alt 25 // 27

#define dirPinStepper_foc 14
#define stepPinStepper_foc 17
#define enablePinStepper 12 // 12 //25

#define IR_RECEIVER_PIN 13 // The Arduino pin connected to IR controller

#define TIME_BLOCK(name, code)       \
    do                               \
    {                                \
        uint32_t t0 = micros();      \
        code;                        \
        uint32_t dt = micros() - t0; \
        if (dt > 10000)              \
        {                            \
            Serial.print(name);      \
            Serial.print(" = ");     \
            Serial.println(dt);      \
        }                            \
    } while (0)

struct Star
{
    String name;
    float ra;
    float dec;
};

Star liste;

Star stars[MAX_STARS];
int starCount = 0;
int star_selected = 0;

struct Coord
{
    float ra;
    float dec;
};

// Valeurs de référence
Coord refPC = {0, 0};
Coord refTab = {0, 0};
Coord refListe = {0, 0};
Coord refFinal = {0, 0};

// Dernière valeur connue pour détecter changement
Coord lastPC = {0, 0};
Coord lastTab = {0, 0};
Coord lastListe = {0, 0};

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *Stepper_azi = NULL;
FastAccelStepper *Stepper_alt = NULL;
FastAccelStepper *Stepper_foc = NULL;

int32_t encoder_azi = 0;
int32_t encoder_alt = 0;

int day = 30, month = 3, year = 2022, hour = 17, minute = 40, second = 0;
float second_ms = 0;
int localUtcOffset = 0; // Offset horaire local par rapport à UTC en heures (ex: +2 pour Paris en été)

unsigned long memo_temps1 = 0, memo_temps2 = 0, memo_temps3 = 0, temps_memo_debut = 0, timer8 = 0, debut_cmd = 0;
unsigned long temps_de_cycle = 0;

double rayon;
float inc_rayon = 5; // 15
float coef_rayon = 0;

double azi_gap_S, alt_gap_S, azi_gap_P, alt_gap_P, azi_gap_G, alt_gap_G, delta = 0.5;
String Dratio = "L";
bool affich_liste = false;

double mes_position_azi = 0;
double mes_position_alt = 0;

String IRcommande, Wcommande, WEBcommande, commande_1;

//   reduction       reducteur  pas/tour mot       step
//     216/12   *       1         *  200       *    16   =    57600
double reducteur = 18, step_moteur_per_rev = 200, diviseur_step_azi = 16, diviseur_step_alt = 16, diviseur_step_foc = 4; // 1,2,4,8,19  TMC2208:8  A4988:1;
double vitesse_max_moteur_tr_mn = 1;
double frequence_max_moteur_hz = vitesse_max_moteur_tr_mn * step_moteur_per_rev;

double frequence_max_moteur_hz_s = frequence_max_moteur_hz / 3;
// double frequence_max_moteur_hz_s = 62.5;

double ref_pos_GI_azi_finale, ref_pos_GI_alt_finale, step_per_rev_azi = step_moteur_per_rev * reducteur * diviseur_step_azi, step_per_rev_alt = step_moteur_per_rev * reducteur * diviseur_step_alt;
double delta_cherche_azi, delta_cherche_alt;
double step_per_deg_azi = step_per_rev_azi / 360;
double step_per_deg_alt = step_per_rev_alt / 360;
double azi_GI_ref_theorique, alt_GI_ref_theorique;

double alt_deg, az_deg;
double alt_speed, azi_speed;
long ecart_azi;
long ecart_alt;

int8_t sensAlt = 0;
int8_t sensAzi = 0;

bool cde_position = false;
bool mvh_mem = false;
bool mvv_mem = false;

long Setpoint_azi = 0;
long Setpoint_alt = 0;

double Ref_altitude = 0;
double Ref_azimuth = 0;

double RA_pointer = 0;  // RA de stellarium PC
double Dec_pointer = 0; // DEC de stellarium PC

bool CalcCoordEtoile = false;

String adresse_IP_PC = "";
String url_stellarium_location = "";
String url_stellarium_location_mem = "";

double actual_RA_D = 0; // en degrés
double actual_RA_H = 0; // en heures
double actual_DEC = 0;

double actual_RA_JNOW = 0;
double actual_DEC_JNOW = 0;

double raToSend = 0; // en degrés
double decToSend = 0;

bool chercheur = false;
unsigned long instant_sortie_chercheur = 0;
unsigned long instant_mem_EQ = 0;
bool chercheur_memo = false; // mémorisation de l'état chercheur
bool mem_EQ = false;
bool Cmde_EQ = false;

double latitude = 45;
double longitude = 4;
double altitude = 542;
double latitude_popup = 45.716976;
double longitude_popup = 4.048397;

double old_latitude; // Valeur précédente pour détecter un changement
double old_longitude;
double consigne_azi = 0;
double consigne_alt = 0;

String mode = "arret";

double cmd_val1 = 0.01; // 0.5
double cmd_val2 = 0;
double cmd_val3 = 10;
double cmd_val4 = 10;
double cmd_val5 = 10;
double cmd_val6 = 0.6;

int Ref_Position = 5000;

int Get_Position = 5100;
bool excursion = false;

String targetRA = "";
String targetDEC = "";
String lxDate = "";

bool aligne = false;
bool StelCommandeHaut = false;
bool StelCommandeGauche = false;
bool StelCommandeBas = false;
bool StelCommandeDroit = false;
bool StelSync = false;
bool StelSyncCM = false;
bool StelSyncMem = false;

const int stellariumPort = 8090;    // Port du plugin RemoteControl (par défaut 8090)
const int stellariumPcPort = 10000; // Port du plugin RemoteControl (par défaut 10000)

bool pc_registered = false;
String pc_name = "";
String pc_role = "";
uint8_t pc_ws_client = 255;

//  fin init--------------------------------------------------------------------------------------------------------------------------------------------------------

uint32_t max_loop_time = 0;
uint32_t last_loop_time = 0;

bool skysafari = false;

//  fonctions:--------------------------------------------------------------------------------------------------------------------------------------------------------
struct DMS
{
    int deg;
    int min;
    float sec;
};

DMS decimalToDMS(float decimal)
{
    DMS dms;
    bool isNegative = decimal < 0;
    decimal = fabs(decimal);
    dms.deg = (int)decimal;
    float remainder = (decimal - dms.deg) * 60;
    dms.min = (int)remainder;
    dms.sec = (remainder - dms.min) * 60;

    if (isNegative)
        dms.deg = -dms.deg;

    return dms;
}

String formatDMS(DMS dms)
{
    String s = "";
    s += String(dms.deg) + " ";
    s += String(dms.min) + " ";
    // s += String(dms.sec, 1) + " ";
    s += String(dms.sec, 0) + " ";
    return s;
}

// === Chargement des étoiles depuis le fichier SPIFFS ===
void loadStars()
{
    File file = SPIFFS.open("/stars.csv", "r");
    if (!file)
    {
        Serial.println("Erreur ouverture fichier stars.csv");
        return;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0 || line.startsWith("Name"))
            continue;

        int idx1 = line.indexOf(',');
        int idx2 = line.lastIndexOf(',');
        if (idx1 == -1 || idx2 == -1)
            continue;

        String name = line.substring(0, idx1);
        float ra = line.substring(idx1 + 1, idx2).toFloat();
        float dec = line.substring(idx2 + 1).toFloat();

        if (starCount < MAX_STARS)
        {
            stars[starCount++] = {name, ra, dec};
        }
    }
    file.close();
}

// === Affichage des coordonnées RA/Dec ===
void showCoordinates(int idx)
{
    sprite1.fillSprite(TFT_BLACK);
    sprite1.setTextColor(TFT_CYAN); // White text, no background colour
    sprite1.drawString("Etoile:", 0, 0);
    sprite1.drawString(stars[idx].name, 0, 20);
    sprite1.drawString("RA: " + String(stars[idx].ra, 3) + " h", 0, 40);
    sprite1.drawString("Dec: " + String(stars[idx].dec, 3) + "d", 0, 60);
    sprite1.pushSprite(0, 0);
    sprite1.fillSprite(TFT_BLACK);
    sprite1.pushSprite(0, 120);
}
// === Affichage de la liste des étoiles ===
void drawMenu()
{
    static long temps_affichage = 0;
    static bool showCoordEtoile = false;
    static bool lastBtn6 = false; // 🔥 mémoire du bout

    if (IRcommande == "3") // UP
    {
        star_selected = (star_selected - 1 + starCount) % starCount;
    }
    else if (IRcommande == "2") // DOWN
    {
        star_selected = (star_selected + 1) % starCount;
    }

    else if (IRcommande == "6")
    {
        showCoordEtoile = true;
        temps_affichage = millis();
    }

    bool btn6 = (IRcommande == "6");

    // Détection front montant
    if (btn6 && !lastBtn6)
    {
        CalcCoordEtoile = true; // 🔥 événement UNIQUE
    }
    lastBtn6 = btn6;

    if (millis() - temps_affichage > 1000) // 1 secondes
    {
        showCoordEtoile = false;
    }

    if (showCoordEtoile)
    {
        showCoordinates(star_selected);
    }
    else // affichage de la liste des étoiles
    {
        static bool affichage_sprite = false;
        static uint32_t temps_affich = 0;
        if (millis() - temps_affich < 200) // 200
            return;
        temps_affich = millis();
        affichage_sprite = !affichage_sprite;
        sprite1.setTextColor(TFT_WHITE); // White text, no background colour

        if (affichage_sprite) // sprite 1
        {
            // Fill it with black
            sprite1.fillSprite(TFT_BLACK);
            sprite1.setTextColor(TFT_WHITE); // White text, no background colour

            // ligne 1
            sprite1.drawString("Selection etoile:", 0, 0);

            for (int i = 0; i < 3 && (i + star_selected) < starCount; i++) // 6
            {
                {
                    int index = i + star_selected;
                    if (i == 0)
                        sprite1.setTextColor(TFT_YELLOW, TFT_BLACK);
                    else
                        sprite1.setTextColor(TFT_WHITE, TFT_BLACK);
                    sprite1.drawString(stars[index].name, 0, 40 + i * 30);
                }
            }
            sprite1.pushSprite(0, 0);
        }

        else // sprite 2
        {
            sprite1.fillSprite(TFT_BLACK);

            for (int i = 3; i < 6 && (i + star_selected) < starCount; i++)
            {
                {
                    int index = i + star_selected;
                    if (i == 0)
                        sprite1.setTextColor(TFT_YELLOW, TFT_BLACK);
                    else
                        sprite1.setTextColor(TFT_WHITE, TFT_BLACK);
                    sprite1.drawString(stars[index].name, 0, -80 + i * 30);
                }
            }
            sprite1.pushSprite(0, 120);
        }
    }
}

void SendData() // Envoi des mesures vers la page web
{
    static unsigned long memo_temps4 = 0;
    if (millis() - memo_temps4 < 200)
        return;
    memo_temps4 = millis();
    // En mode point d'accès autonome (WIFI_AP), WiFi.status()
    // n'est pas WL_CONNECTED. Il ne faut donc surtout pas bloquer
    // l'envoi WebSocket ici. Le serveur WebSocket fonctionne sur 81.

    StaticJsonDocument<1024> doc;
    DMS result;
    String dmsString;

    doc["temps_de_cycle"] = temps_de_cycle;

    result = decimalToDMS(latitude); // latitude DMS
    dmsString = formatDMS(result);
    doc["latitude"] = dmsString;

    result = decimalToDMS(longitude); // longitude en DMS
    dmsString = formatDMS(result);
    doc["longitude"] = dmsString;

    result = decimalToDMS(refFinal.ra); // RA pour DMS
    dmsString = formatDMS(result);
    doc["getRAdec"] = dmsString;

    result = decimalToDMS(refFinal.dec); // DEC pour DMS
    dmsString = formatDMS(result);
    doc["getDeclinationDec"] = dmsString;

    doc["Setpoint_azi"] = Setpoint_azi;
    doc["encoder_azi"] = encoder_azi;
    doc["Setpoint_alt"] = Setpoint_alt;
    doc["encoder_alt"] = encoder_alt;
    doc["heure"] = hour + localUtcOffset;
    doc["minute"] = minute;
    doc["second_ms"] = second;
    // Pos_Gris2 vl14  mode  14 15 16 17 18 19 reserves pour affichage dans gris

    doc["mode"] = mode; // gris

    result = decimalToDMS(consigne_azi); // azi ref
    dmsString = formatDMS(result);
    doc["consigne_azi"] = dmsString;

    result = decimalToDMS(consigne_alt); // alt ref
    dmsString = formatDMS(result);
    doc["consigne_alt"] = dmsString;

    doc["Dratio"] = Dratio; // gris

    result = decimalToDMS((float)encoder_azi / step_per_deg_azi); // azi act
    dmsString = formatDMS(result);
    doc["points_angle_azi"] = dmsString;

    result = decimalToDMS((float)encoder_alt / step_per_deg_alt); // alt act
    dmsString = formatDMS(result);
    doc["points_angle_alt"] = dmsString;

    doc["rayon"] = rayon;
    doc["WiFi_RSSI"] = WiFi.RSSI();
    doc["AziGap"] = String(azi_gap_G, 2);
    doc["AltGap"] = String(alt_gap_G, 2);

    doc["latitudeDec"] = String(latitude, 4);
    doc["longitudeDec"] = String(longitude, 4);

    float position = Get_Position;
    // Serial.println("Envoi de position: " +  String(position, 2));
    doc["Position"] = String(position, 2);

    char buffer[1024];
    size_t len = serializeJson(doc, buffer);
    webSocket.broadcastTXT(buffer, len);
}

void webSocketEventCmd(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    // =====================================================
    // CLIENT CONNECTÉ
    // =====================================================
    case WStype_CONNECTED:

        Serial.printf("WebSocket client connecté #%u\n", num);
        Serial.println("   -> Les autres clients WebSocket restent connectés");

        break;

    // =====================================================
    // CLIENT DÉCONNECTÉ
    // =====================================================
    case WStype_DISCONNECTED:

        Serial.printf("WebSocket client déconnecté #%u\n", num);

        if (num == pc_ws_client)
        {
            pc_registered = false;
            pc_name = "";
            pc_role = "";
            pc_ws_client = 255;

            Serial.println("Client PC enregistré déconnecté");
        }

        // IMPORTANT :
        // ne jamais mettre page_connectee à false ici.
        // Un autre client WebSocket peut encore être connecté.
        break;

    // =====================================================
    // MESSAGE TEXTE
    // =====================================================
    case WStype_TEXT:
    {
        // -------------------------------------------------
        // Protection contre un message trop long
        // -------------------------------------------------
        if (length > 255)
        {
            Serial.println("Payload trop long !");
            return;
        }

        // -------------------------------------------------
        // Document JSON
        // -------------------------------------------------
        StaticJsonDocument<256> doc;

        DeserializationError error =
            deserializeJson(doc, payload, length);

        if (error)
        {
            Serial.print("Erreur JSON: ");
            Serial.println(error.c_str());
            return;
        }

        // =================================================
        // ENREGISTREMENT DU PC
        // =================================================
        if (doc.containsKey("type"))
        {
            String typeMsg = doc["type"].as<String>();

            if (typeMsg == "register")
            {
                pc_registered = true;
                pc_ws_client = num;

                if (doc.containsKey("name"))
                    pc_name = doc["name"].as<String>();

                if (doc.containsKey("role"))
                    pc_role = doc["role"].as<String>();

                Serial.println("==========");
                Serial.println("PC enregistré");
                Serial.println("Nom  : " + pc_name);
                Serial.println("Role : " + pc_role);
                Serial.println("Client WebSocket : " + String(num));
                Serial.println("==========");

                return;
            }
        }

        // =================================================
        // COMMANDES EXISTANTES
        // =================================================
        if (doc.containsKey("command"))
        {
            Wcommande = doc["command"].as<String>();

            Serial.println("Commande reçue WEB: " + Wcommande);
        }

        break;
    }

    // =====================================================
    // AUTRES TYPES DE MESSAGES
    // =====================================================
    case WStype_ERROR:

        Serial.printf("Erreur WebSocket client #%u\n", num);
        break;

    case WStype_BIN:

        Serial.printf("Message binaire ignoré client #%u\n", num);
        break;

    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:

        // Fragments non utilisés dans ton application
        break;

    case WStype_PING:

        // Ping WebSocket
        break;

    case WStype_PONG:

        // Réponse au ping
        break;

    // =====================================================
    // SÉCURITÉ : TYPE INCONNU
    // =====================================================
    default:

        Serial.printf("Type WebSocket non traité : %d\n", type);
        break;
    }
}

void initRTS2Server()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        // 🔹 Récupérer IP AVANT send()
        String ip = request->client()->remoteIP().toString();
        adresse_IP_PC = ip;

        url_stellarium_location = "http://" + adresse_IP_PC + ":" + 
                                  String(stellariumPort) + 
                                  "/api/location/setlocationfields";

        // Sauvegarde NVS
        prefs.begin("monapp", false);
        prefs.putString("url_stellarium", url_stellarium_location);
        prefs.end();

        Serial.print("url_PC_location: ");
        Serial.println(url_stellarium_location);

        // La connexion HTTP à la page PC est indépendante du WebSocket.
        // Elle ne doit pas servir d'indicateur global de connexion.

        // 🔹 Construire page
       // String htmlPage = FPSTR(webpageCont);

        // 🔹 SEND EN DERNIER
       // request->send(200, "text/html", htmlPage); });

            // IMPORTANT : webpageCont est en PROGMEM et peut être volumineuse.
        // Ne pas la copier dans un String local avant send() : avec plusieurs
        // clients HTTP/WebSocket, cette copie peut provoquer une fragmentation
        // du tas et un crash LoadProhibited dans AsyncBasicResponse::_respond().
        // send_P() envoie directement la page depuis la Flash.

          request->send(request->beginResponse(200, "text/html", webpageCont)); });

    server.on("/getip", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String ip = request->client()->remoteIP().toString();
        adresse_IP_PC = ip;

        url_stellarium_location = "http://" + adresse_IP_PC + ":" +
                                  String(stellariumPort) +
                                  "/api/location/setlocationfields";

        request->send(200, "text/plain", ip); });

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    server.on("/settime", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    // Vérification de l'heure
    if (!request->hasParam("epoch"))
    {
        request->send(400, "text/plain", "epoch manquant");
        return;
    }

    // Récupération de l'epoch UTC envoyé par le navigateur
    String value = request->getParam("epoch")->value();

    uint64_t epochMs = strtoull(value.c_str(), nullptr, 10);

    if (epochMs < 1000000000ULL)
    {
        request->send(400, "text/plain", "epoch invalide");
        return;
    }

    // -------------------------------------------------
    // Récupération du décalage local
    // -------------------------------------------------

    if (request->hasParam("offset"))
    {
        String offsetValue = request->getParam("offset")->value();
        localUtcOffset = offsetValue.toInt();
    }
    else
    {
        Serial.println("ATTENTION : décalage local absent");
    }

    // -------------------------------------------------
    // Mise à l'heure UTC de l'ESP32
    // -------------------------------------------------

    time_t epochSec = (time_t)(epochMs / 1000ULL);

    struct timeval tv =
    {
        .tv_sec = epochSec,
        .tv_usec = (suseconds_t)((epochMs % 1000ULL) * 1000ULL)
    };

    settimeofday(&tv, nullptr);

    // -------------------------------------------------
    // Affichage de contrôle
    // -------------------------------------------------

    struct tm ti;

    if (getLocalTime(&ti))
    {
        Serial.printf(
            "Heure ESP32 : %02d/%02d/%04d %02d:%02d:%02d\n",
            ti.tm_mday,
            ti.tm_mon + 1,
            ti.tm_year + 1900,
            ti.tm_hour,
            ti.tm_min,
            ti.tm_sec
        );
    }

    Serial.printf("UTC epoch reçu : %llu\n", epochMs);
    Serial.printf("Décalage local : %+d h\n", localUtcOffset);

    request->send(200, "text/plain", "OK"); });

    server.begin();
    Serial.println("HTTP server started");
}

void excur()
{
    static bool exmem = false;
    if (excursion)
    {
        exmem = true;
        if (Ref_Position != 0 && Ref_Position != 10000)
        {
            if (Ref_Position > 5000)
            {
                Ref_Position = 10000;
            }
            else
            {
                Ref_Position = 0;
            }
        }

        if (Ref_Position == 0 && Get_Position < 100)
            Ref_Position = 10000;
        if (Ref_Position == 10000 && Get_Position > 9900)
            Ref_Position = 0;
    }
    else
    {
        if (exmem)
        {
            exmem = false;
            Ref_Position = Get_Position;
        }
    }
}

void updateEnablePin() // pin enable stepper
{
    static unsigned long Fin_EnablePin_stepper_memo; // Fin_EnablePin_stepper_memo
    if (mode != "arret" || Stepper_foc->isRunning())
    {
        Fin_EnablePin_stepper_memo = millis(); // Debut meo tant que l'un is running
        digitalWrite(enablePinStepper, LOW);   // Active les moteurs
    }
    if (millis() > Fin_EnablePin_stepper_memo + 2000)
    {
        digitalWrite(enablePinStepper, HIGH); // Désactive  blocage moteurs si aucun n'est en mouvement lus de 1s
    }
}
// stellarium
// =======================
// === Classe Buffer =====
// =======================
class Buffer
{
public:
    static const int bufferSize = 64;
    char cb[bufferSize];
    byte cbp = 0;
    bool checksum = false;
    char seq;

    void flush()
    {
        cb[0] = 0;
        cbp = 0;
    }

    bool add(char c)
    {
        if (c == (char)6 && cbp == 0)
        {
            cb[0] = ':';
            cb[1] = (char)6;
            cb[2] = 'P'; // ou 'A' si mountType == 3
            cb[3] = 0;
            cbp = 3;
            c = '#';
        }

        // ignore spaces/lf/cr
        if (c != (char)32 && c != (char)10 && c != (char)13 && c != (char)6)
        {
            if (cbp > bufferSize - 2)
                cbp = bufferSize - 2;
            cb[cbp] = c;
            cbp++;
            cb[cbp] = 0;
        }

        if (c == '#')
        {
            if (!(cbp > 1) && ((cb[0] == ':') || (cb[0] == ';')) && (cb[cbp - 1] == '#'))
            {
                flush();
                return false;
            }
            if (((cb[0] == ':') || (cb[0] == ';')) && (cb[1] == '#') && (cb[2] == 0))
            {
                flush();
                return false;
            }

            checksum = (cb[0] == ';');
            if (checksum)
            {
                byte len = strlen(cb) - 1;
                if (len < 5)
                {
                    flush();
                    cb[0] = ':';
                    cb[1] = (char)6;
                    cb[2] = '0';
                    cb[3] = '#';
                    cb[4] = 0;
                    cbp = 4;
                    return true;
                }

                byte cks = 0;
                for (int i = 1; i < len - 3; i++)
                {
                    cks += cb[i];
                }

                char chkSum[12];
                sprintf(chkSum, "%02X", cks);
                seq = cb[len - 1];
                if (!(chkSum[0] == cb[len - 3] && chkSum[1] == cb[len - 2]))
                {
                    flush();
                    cb[0] = ':';
                    cb[1] = (char)6;
                    cb[2] = '0';
                    cb[3] = '#';
                    cb[4] = 0;
                    cbp = 4;
                    return true;
                }
                --len;
                --len;
                cb[--len] = 0;
            }

            return true;
        }
        return false;
    }
};

String getFormattedTime(int hourOffset = 0)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return "00:00:00#";

    // Convertir en time_t pour manipuler le temps facilement
    time_t rawTime = mktime(&timeinfo);

    // Ajouter ou soustraire les heures
    rawTime += hourOffset * 3600;

    // Recalculer la structure tm à partir du temps modifié
    localtime_r(&rawTime, &timeinfo);

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return String(buffer) + "#";
}

String getFormattedDate()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return "01/01/70#";

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%m/%d/%y", &timeinfo);
    return String(buffer) + "#";
}

String getVersionDate()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return "Jan 01 1970#";

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%b %d %Y", &timeinfo); // Ex: Jun 25 2025
    return String(buffer) + "#";
}

String getVersionTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return "00:00:00#";

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return String(buffer) + "#";
}

// Convertit une valeur flottante en  "HH*MM:SS"
String formatRA(double raHours)
{
    if (raHours > 24)
        raHours = 24;
    if (raHours < 0)
        raHours = 0;
    int h = int(raHours);
    int m = int((raHours - h) * 60);
    int s = int((((raHours - h) * 60) - m) * 60);
    char buf[32];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d#", h, m, s);
    return String(buf);
}

// Convertit une valeur flottante en DEC "+DD*MM:SS"
String formatDEC(double decDegrees)
{
    if (decDegrees > 90)
        decDegrees = 90;
    if (decDegrees < -90)
        decDegrees = -90;
    char sign = (decDegrees >= 0) ? '+' : '-';
    decDegrees = abs(decDegrees);
    int d = int(decDegrees);
    int m = int((decDegrees - d) * 60);
    int s = int((((decDegrees - d) * 60) - m) * 60);
    char buf[32];
    snprintf(buf, sizeof(buf), "%c%02d*%02d:%02d#", sign, d, m, s);
    return String(buf);
}

// converti +DD*MM:SS en decimal
float decToDecimal(String input)
{
    // Exemple attendu : +45*43:12 ou -10*25:30
    int sign = (input.startsWith("-")) ? -1 : 1;

    // Supprime le signe pour faciliter le découpage
    input.replace("+", "");
    input.replace("-", "");

    // Séparation
    int degIndex = input.indexOf("*");
    int minIndex = input.indexOf(":", degIndex + 1);

    if (degIndex == -1 || minIndex == -1)
        return 0.0;

    int deg = input.substring(0, degIndex).toInt();
    int min = input.substring(degIndex + 1, minIndex).toInt();
    int sec = input.substring(minIndex + 1).toInt();

    float result = deg + (min / 60.0) + (sec / 3600.0);
    return sign * result;
}

// converti HH*MM:SS en decimale
float raToDecimalHours(String input)
{
    // Exemple : "04*35:54" → 4.5983 heures
    int hIndex = input.indexOf("*");
    int mIndex = input.indexOf(":", hIndex + 1);

    if (hIndex == -1 || mIndex == -1)
        return 0.0;

    int hours = input.substring(0, hIndex).toInt();
    int minutes = input.substring(hIndex + 1, mIndex).toInt();
    int seconds = input.substring(mIndex + 1).toInt();

    float result = hours + (minutes / 60.0) + (seconds / 3600.0);
    return result;
}

float sexagesimalToDecimalDEC(const String &input)
{
    int sign = 1;
    if (input.startsWith("-"))
        sign = -1;

    // Retirer le signe si présent
    String clean = input;
    if (input.startsWith("+") || input.startsWith("-"))
    {
        clean = input.substring(1);
    }

    // Uniformiser les séparateurs
    clean.replace('*', ':');

    int firstColon = clean.indexOf(':');
    int secondColon = clean.indexOf(':', firstColon + 1);

    if (firstColon == -1 || secondColon == -1)
    {
        return 0.0; // format invalide
    }

    int d = clean.substring(0, firstColon).toInt();
    int m = clean.substring(firstColon + 1, secondColon).toInt();
    int s = clean.substring(secondColon + 1).toInt();

    return sign * (d + (m / 60.0) + (s / 3600.0));
}

float sexagesimalToDecimalRA(const String &input)
{
    int h = 0, m = 0, s = 0;

    int firstColon = input.indexOf(':');
    int secondColon = input.indexOf(':', firstColon + 1);

    if (firstColon != -1 && secondColon != -1)
    {
        h = input.substring(0, firstColon).toInt();
        m = input.substring(firstColon + 1, secondColon).toInt();
        s = input.substring(secondColon + 1).toInt();
    }
    else
    {
        // Serial.println("Format RA invalide !");
        return 1.0;
    }

    float decimal = (h + (m / 60.0) + (s / 3600.0));
    return decimal;
}

String formatLatitude(double lat)
{
    char sign = (lat >= 0) ? '+' : '-';
    lat = fabs(lat);

    int deg = int(lat);
    int min = int((lat - deg) * 60.0 + 0.5); // arrondi

    if (min >= 60)
    {
        min = 0;
        deg++;
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%c%02d*%02d#", sign, deg, min);
    return String(buf);
}

String formatLongitude(double lon)
{
    // Normalisation entre -180 et +180
    if (lon > 180.0)
        lon -= 360.0;
    if (lon < -180.0)
        lon += 360.0;

    char sign = (lon >= 0) ? '+' : '-';
    lon = fabs(lon);

    int deg = int(lon);
    int min = int((lon - deg) * 60.0 + 0.5); // arrondi

    if (min >= 60)
    {
        min = 0;
        deg++;
    }

    // Toujours 3 chiffres pour les degrés
    char buf[32];
    snprintf(buf, sizeof(buf), "%c%03d*%02d#", sign, deg, min);
    return String(buf);
}

String handleLX200(String cmd)
{
    // Commande spéciale char(6)
    if (cmd == String(":") + char(6) + "P")
        return "P";
    if (cmd == String(":") + char(6) + "A")
        return "A#";

    // Commandes LX200
    if (cmd == ":GVP")
        return "On-Step#";
    if (cmd == ":GVN")
        return "10.25r#";
    if (cmd == ":GVD")
        return getVersionDate();
    if (cmd == ":GVT")
        return getVersionTime();

    // if (cmd == ":GR") return "04:30:00#";
    if (cmd == ":GR")
        // return formatRA(actual_RA_H); // #:GR# => 04:30:00#"  affichage sur stellarium
        return formatRA(raToSend); // #:GR# => 04:30:00#"  affichage sur stellarium
    if (cmd == ":GD")
        return formatDEC(decToSend); // #:GD# => +14*45:00#"  affichage sur stellarium
    if (cmd == ":Gt")
        return formatLatitude(latitude); // prendre les coordonnées du PC
    // return "+49*47#"; // prendre les coordonnées du PC
    if (cmd == ":Gg")
        return formatLongitude(longitude * -1); // prendre les coordonnées du PC  pour onestep #:Gg# => -004*52#
    // return "-014*50#"; // prendre les coordonnées du PC
    if (cmd == ":GC")
        return getFormattedDate(); // Date courante MM/DD/YY
    // if (cmd == ":GL") return getFormattedTime();  // Heure locale HH:MM:SS
    if (cmd == ":GL")
        return getFormattedTime(localUtcOffset); // Heure locale HH:MM:SS   //1

    if (cmd == ":GG")
    {
        return "-" + String(localUtcOffset) + ".0#"; // "-01.0#"localUtcOffset
        // return "-02.0#"; // "-01.0#"
    }

    // Stellarium envoie la date et l'heure au télescope.
    if (cmd.startsWith(":SC"))
    {
        // La date seule est mémorisée temporairement.
        lxDate = cmd.substring(3);
        return "1";
    }

    if (cmd == ":D")
        return "#";
    if (aligne)
    {
        if (cmd == ":GW")
            return "AN1#";
    }
    else
    {
        if (cmd == ":GW")
            return "AT0#";
    }

    if (cmd == ":RS")
        return ""; // pour skysafary
    if (cmd == ":RC")
        return "+43*12:08#"; // pour skysafary

    // === Commandes pour définir la cible ===
    if (cmd.startsWith(":Sr"))
    {
        targetRA = cmd.substring(3); // retire ":Sr"
        return "1";
    }

    if (cmd.startsWith(":Sd"))
    {
        targetDEC = cmd.substring(3); // retire ":Sd"
        return "1";
    }

    // === Commandes pour définir la cible === a supprimer
    if (cmd.startsWith(":St"))
    {
        return "1";
    }

    // === Commandes pour définir la cible ===  a supprimer
    if (cmd.startsWith(":Sg"))
    {
        return "1";
    }

    if (cmd.startsWith(":SG"))
    {
        return "1#";
    }

    if (cmd == ":MS") // commande de suivit
    {
        mode = "suivit";
        return "0"; // 0 = Slew démarré
    }

    if (cmd == ":Mn")
    { // debut cmd nord
        StelCommandeHaut = true;
        return "";
    }
    if (cmd == ":Qn")
    { // arret cmd nord
        StelCommandeHaut = false;
        return "";
    }
    if (cmd == ":Me")
    { // debut cmd est
        StelCommandeGauche = true;
        return "";
    }
    if (cmd == ":Qe")
    { // arret cmd est
        StelCommandeGauche = false;
        return "";
    }
    if (cmd == ":Ms")
    { // debut cmd sud
        StelCommandeBas = true;
        return "";
    }
    if (cmd == ":Qs")
    { // arret cmd sud
        StelCommandeBas = false;
        return "";
    }
    if (cmd == ":Mw")
    { // debut cmd ouest
        StelCommandeDroit = true;
        return "";
    }
    if (cmd == ":Qw")
    { // arret cmd ouest
        StelCommandeDroit = false;
        return "";
    }

    if (cmd == ":RS")
    { // 4:max
        Dratio = "HHH";
        return "";
    }
    if (cmd == ":RM")
    { // 3:recherche
        Dratio = "MM";
        return "";
    }
    if (cmd == ":RC")
    { // 2:centrage
        Dratio = "MM";
        return "";
    }
    if (cmd == ":RG")
    { // 1:guidage
        Dratio = "L";
        return "";
    }
    if (cmd == ":Q")
    {
        return "";
    }
    if (cmd == ":CM")
    { // synchro
        StelSyncCM = true;
        return "Alignment Successful#";
    }
    return "?#";
}

void sendPosition(float ra_hours, float dec_deg)
{
    static uint32_t lastSend = 0;
    if (millis() - lastSend < 1000)
        return;
    lastSend = millis();

    // Vérification client
    if (!stellariumPcClient || !stellariumPcClient.connected())
    {
        stellariumPcClient.stop(); // nettoyage important
        return;
    }

    uint32_t ra_units = uint32_t(round((ra_hours / 24.0) * 0x40000000)) * 4;
    int32_t dec_units = int32_t(round((dec_deg / 90.0) * 0x40000000));

    uint8_t packet[24] = {0};
    packet[0] = 0x18;

    memcpy(&packet[12], &ra_units, sizeof(uint32_t));
    memcpy(&packet[16], &dec_units, sizeof(int32_t));

    //  Protection anti-blocage
    uint32_t t0 = millis();
    size_t written = stellariumPcClient.write(packet, 24);

    if (millis() - t0 > 10 || written != 24)
    {
        Serial.println("⚠️ Socket bloqué → fermeture");
        stellariumPcClient.stop(); // 🔥 très important
    }
}

void decodePointerPacket(uint8_t *buffer, size_t length) // recu du BP pointer de stellarium
{
    if (length < 20)
        return;

    // Serial.print("Buffer reçu : ");
    for (size_t i = 0; i < length; i++)
    {
        Serial.printf("%02X ", buffer[i]);
    }
    Serial.println();

    uint32_t ra_raw;
    int32_t dec_raw;

    memcpy(&ra_raw, &buffer[12], sizeof(uint32_t));
    memcpy(&dec_raw, &buffer[16], sizeof(int32_t));

    RA_pointer = float(ra_raw) / 3314.01797539 / 3600.0 / 15.0;
    Dec_pointer = float(dec_raw) / 3314.01797539 / 3600.0;
}

// Vérifie si une coordonnée est valide
bool coordValide(double lat, double lon)
{
    return (!isnan(lat) && !isnan(lon) &&
            lat >= -90.0 && lat <= 90.0 &&
            lon >= -180.0 && lon <= 180.0);
}

void initCoordonnees()
{
    // Valeurs par défaut
    const double LAT_DEF = 45.717504;
    const double LON_DEF = 4.041513;

    prefs.begin("monapp", false); // écriture possible

    // ----- Coordonnées -----
    double lat = prefs.getDouble("latitude", NAN);
    double lon = prefs.getDouble("longitude", NAN);

    if (!coordValide(lat, lon))
    {
        Serial.println("Coordonnées invalides -> réinitialisation avec valeurs par défaut");
        prefs.putDouble("latitude", LAT_DEF);
        prefs.putDouble("longitude", LON_DEF);
        lat = LAT_DEF;
        lon = LON_DEF;
    }

    latitude = lat;
    longitude = lon;

    old_latitude = latitude;
    old_longitude = longitude;

    latitude_popup = latitude;
    longitude_popup = longitude;

    Serial.printf("Coordonnées chargées : Latitude = %.10f, Longitude = %.10f\n",
                  latitude, longitude);

    // ----- URL Stellarium (nouveau) -----
    url_stellarium_location_mem = prefs.getString("url_stellarium", "");

    if (url_stellarium_location_mem.length())
    {
        Serial.print("URL Stellarium chargée depuis NVS : ");
        Serial.println(url_stellarium_location_mem);
        // Une URL mémorisée ne signifie pas qu'un client Web est actuellement connecté.
    }
    else
    {
        Serial.println("Aucune URL Stellarium mémorisée");
    }

    prefs.end();
}

// Fonction pour envoyer les coordonnées observateur à Stellarium PC
void sendLocation(double lat, double lon, double alt)
{
    // Vérifier que l'IP du PC a été détectée
    if (adresse_IP_PC.length() == 0)
    {
        Serial.println("sendLocation : IP du PC inconnue");
        return;
    }

    HTTPClient http;

    // Construire l'URL Stellarium PC
    String url = "http://" + adresse_IP_PC +
                 ":" + String(stellariumPort) +
                 "/api/location/setlocationfields";

    Serial.println("================================");
    Serial.println("Envoi position à Stellarium PC");
    Serial.println("PC : " + adresse_IP_PC);
    Serial.println("URL : " + url);

    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String data = "latitude=" + String(lat, 6) +
                  "&longitude=" + String(lon, 6) +
                  "&altitude=" + String(alt, 6);

    Serial.println("DATA : " + data);

    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0)
    {
        String payload = http.getString();

        Serial.println("Code HTTP : " + String(httpResponseCode));
        Serial.println("Réponse Stellarium : " + payload);
    }
    else
    {
        Serial.println("Erreur POST Stellarium : " +
                       String(httpResponseCode));
    }

    http.end();

    Serial.println("================================");
}

void updateGPS()
{
    static bool premierTour = true; // Pour éviter d'écrire juste après le démarrage
    // En mode autonome, la position locale doit être traitée même si
    // aucun navigateur PC n'est connecté. La tablette Stellarium et
    // la page Web sont deux clients indépendants.
    if (millis() > memo_temps3 + 2000) // 100
    {
        memo_temps3 = millis();
        if (isnan(latitude_popup))
        {
            Serial.println("latitude_popup est NaN !");
        }
        else
        {
            latitude = latitude_popup;
        }

        if (isnan(longitude_popup))
        {
            Serial.println("longitude_popup est NaN !");
        }
        else
        {
            longitude = longitude_popup;
        }

        // Détection de changement
        if (!premierTour && (latitude != old_latitude || longitude != old_longitude))
        {
            Serial.println("Changement détecté, sauvegarde...");
            prefs.begin("monapp", false); // Écriture
            prefs.putDouble("latitude", latitude);
            prefs.putDouble("longitude", longitude);
            prefs.end();

            sendLocation(latitude, longitude, altitude);

            old_latitude = latitude;
            old_longitude = longitude;
        }

        // On désactive le "premierTourdecycle" après 2 secondes pour éviter
        // d'enregistrer au démarrage si les valeurs sont identiques
        if (premierTour && millis() > 2000)
        {
            premierTour = false;
        }
    }
}

void updateTime()
{
    static uint32_t memo_temps3 = 0;
    if (millis() < 5000 || millis() - memo_temps3 < 100)
        return;
    memo_temps3 = millis();
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Impossible d'obtenir l'heure");
        return;
    }

    year = timeinfo.tm_year + 1900;
    month = timeinfo.tm_mon + 1;
    day = timeinfo.tm_mday;
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    second_ms = second + (millis() % 1000) / 1000.0;

    myAstro.setLatLong(latitude, longitude);
    myAstro.setGMTdate(year, month, day);
    myAstro.setLocalTime(hour, minute, second_ms);
    myAstro2.setLatLong(latitude, longitude);
    myAstro2.setGMTdate(year, month, day);
    myAstro2.setLocalTime(hour, minute, second_ms);
}

void choixRefPcOuTab() // choix de ref coordonées astre entre PC et tablette
{
    myAstro.setRAdec(refFinal.ra, refFinal.dec); // set de la ref

    if (fabs(refFinal.ra - actual_RA_H) < 1 && fabs(refFinal.dec - actual_DEC) < 1)
    {
        aligne = true;
    }
    else
    {
        aligne = false;
    }

    // consigne de position en  AZ et ALT
    myAstro.doRAdec2AltAz();
    Ref_azimuth = myAstro.getAzimuth();
    Ref_altitude = myAstro.getAltitude();

    // mesure position pour retour
    // conversion points codeur en coordonées terrestre pour retour
    mes_position_azi = (double)encoder_azi / step_per_deg_azi; // retour mis a l'echelle pour affichage
    mes_position_alt = (double)encoder_alt / step_per_deg_alt;

    if (mes_position_alt == 0) // pour eviter Nan
        mes_position_alt = 0.00001;
    if (mes_position_azi == 0)
        mes_position_azi = 0.00001;
    if (mes_position_azi == 180)
        mes_position_azi = 180.00001;

    // Serial.printf("Mesure position → Alt: %.4f°, Azi: %.4f°\n", mes_position_alt, mes_position_azi);

    myAstro.setAltAz(mes_position_alt, mes_position_azi);
    myAstro.doAltAz2RAdec(); // terrestre vers celeste (retour)

    actual_RA_JNOW = myAstro.getRAdec(); // ok  retour de position en J2000 car placé apres le  myAstro.setAltAz()
    actual_DEC_JNOW = myAstro.getDeclinationDec();

    // conversion de la date vers j2000
    myAstro.setGMTdate(year, month, day);
    myAstro.doPrecessTo2000();
    myAstro.setGMTdate(2000, 1, 1);
    myAstro.doPrecessFrom2000(); // 1

    actual_RA_H = myAstro.getRAdec(); // ok  retour de position en J2000 car placé apres le  myAstro.setAltAz()
    actual_RA_D = myAstro.getRAdec() * 360 / 24;
    actual_DEC = myAstro.getDeclinationDec();

    // Serial.println(skysafari);
    // Serial.println(actual_RA_JNOW-actual_RA_H);
    // Serial.println(actual_DEC_JNOW-actual_DEC);
    if (skysafari)
    {
        raToSend = actual_RA_JNOW;
        decToSend = actual_DEC_JNOW;
    }
    else
    {
        raToSend = actual_RA_H;
        decToSend = actual_DEC;
    }
    // Serial.printf("Ref RA: %.4f h, Ref DEC: %.4f° | Actual RA: %.4f h, Actual DEC: %.4f°\n", refFinal.ra, refFinal.dec, actual_RA_H, actual_DEC);
}

// Met à jour refFinal selon la dernière source qui change
void updateRefFinal()
{
    if (refListe.ra != lastListe.ra || refListe.dec != lastListe.dec)
    {
        refFinal = refListe;
        lastListe = refListe;
        return; // Liste change => priorité max
    }
    if (refTab.ra != lastTab.ra || refTab.dec != lastTab.dec)
    {
        refFinal = refTab;
        lastTab = refTab;
        return;
    }
    if (refPC.ra != lastPC.ra || refPC.dec != lastPC.dec)
    {
        refFinal = refPC;
        lastPC = refPC;
        return;
    }
}

void updateAstroReference()
{
    if (affich_liste)
    {
        if (CalcCoordEtoile)
        {
            Serial.println(star_selected);
            liste.name = stars[star_selected].name;
            liste.ra = stars[star_selected].ra;
            liste.dec = stars[star_selected].dec;

            Serial.printf("Étoile sélectionnée: %s | RA: %.4f h, DEC: %.4f°\n", liste.name.c_str(), liste.ra, liste.dec);

            if (liste.name == "lune")
                myAstro2.doMoon();
            else if (liste.name == "venus")
                myAstro2.doVenus();
            else if (liste.name == "jupiter")
                myAstro2.doJupiter();
            else if (liste.name == "saturne")
                myAstro2.doSaturn();
            else if (liste.name == "mars")
                myAstro2.doMars();
            else if (liste.name == "mercure")
                myAstro2.doMercury();
            else if (liste.name == "uranus")
                myAstro2.doUranus();
            else if (liste.name == "neptune")
                myAstro2.doNeptune();
            else // pour les étoiles
            {
                myAstro2.setRAdec(liste.ra, liste.dec);
                myAstro2.setGMTdate(2000, 1, 1);
                myAstro2.doPrecessTo2000();
                myAstro2.setGMTdate(year, month, day);
                myAstro2.doPrecessFrom2000();
            }

            refListe.ra = myAstro2.getRAdec();
            refListe.dec = myAstro2.getDeclinationDec();

            Serial.printf("Liste sélectionnée: %s | RA: %.4f h, DEC: %.4f°\n", liste.name.c_str(), refListe.ra, refListe.dec);
            CalcCoordEtoile = false;
        }
    }

    myAstro.setRAdec(RA_pointer, Dec_pointer); // ref stellarium PC
    myAstro.setGMTdate(2000, 1, 1);
    myAstro.doPrecessTo2000();
    myAstro.setGMTdate(year, month, day);
    myAstro.doPrecessFrom2000();
    refPC.ra = myAstro.getRAdec();
    refPC.dec = myAstro.getDeclinationDec();

    refTab.ra = sexagesimalToDecimalRA(targetRA); // ref tablette
    refTab.dec = sexagesimalToDecimalDEC(targetDEC);
    myAstro.setRAdec(refTab.ra, refTab.dec);
    if (!skysafari)
    {
        myAstro.setGMTdate(2000, 1, 1);
        myAstro.doPrecessTo2000();
        myAstro.setGMTdate(year, month, day);
        myAstro.doPrecessFrom2000();
    }
    refTab.ra = myAstro.getRAdec();
    refTab.dec = myAstro.getDeclinationDec();

    choixRefPcOuTab();
    updateRefFinal();
}

void focuseur()
{
    const int Inc_Pos_P = 10; // petit
    const int Inc_Pos_G = 80; // grand
    // commande vers focuseur
    if (WEBcommande == "Up")
    {
        excursion = false;
        if (Ref_Position <= 10000 - Inc_Pos_P)
            Ref_Position += Inc_Pos_P;
    }
    if (WEBcommande == "UpFast" || IRcommande == "CH-")
    {
        excursion = false;
        if (Ref_Position <= 10000 - Inc_Pos_G)
            Ref_Position += Inc_Pos_G;
    }
    if (WEBcommande == "Down")
    {
        excursion = false;
        if (Ref_Position >= Inc_Pos_P)
            Ref_Position -= Inc_Pos_P;
    }
    if (WEBcommande == "DownFast" || IRcommande == "-")
    {
        excursion = false;
        if (Ref_Position >= Inc_Pos_G)
            Ref_Position -= Inc_Pos_G;
    }
    if (WEBcommande == "Excur")
    {
        excursion = true;
    }
    if (WEBcommande == "Stop")
    {
        excursion = false;
    }
    if (WEBcommande == "Raz5000")
    {
        Ref_Position = 5000;
        excursion = false;
        Stepper_foc->setCurrentPosition(5000 * diviseur_step_foc);
    }

    Stepper_foc->moveTo(Ref_Position * diviseur_step_foc);
    Get_Position = Stepper_foc->getCurrentPosition() / diviseur_step_foc;

    excur();
}

void affichage_TFT()
{
    static bool affichage_sprite = false;
    static bool clignotant = false;
    static unsigned long memo_temps_clignotant = 0;
    static bool oldCmd = false;
    bool newCmd = (IRcommande == "1" || WEBcommande == "Aff");

    // Gestion de la commande d'affichage
    if (newCmd && !oldCmd)
    {
        affich_liste = !affich_liste;
    }
    oldCmd = newCmd;

    if (affich_liste)
    {
        drawMenu();
        return; // Sortie rapide si on affiche le menu
    }

    // Limitation de la fréquence d'affichage (200ms)
    static uint32_t temps_affich = 0;
    if (millis() - temps_affich < 200)
    {
        return;
    }
    temps_affich = millis();
    affichage_sprite = !affichage_sprite;

    // Pré-calcul des couleurs et positions pour éviter les répétitions
    sprite1.fillSprite(TFT_BLACK);

    if (affichage_sprite)
    {
        // --- Sprite 1 ---
        sprite1.setTextColor(TFT_WHITE);
        if (millis() < 20000)
        {
            // sprite1.drawString(WiFi.localIP().toString(), 0, 0);
            sprite1.drawString(WiFi.softAPIP().toString(), 0, 0);
        }
        else
        {
            sprite1.drawString(formatDMS(decimalToDMS(refFinal.ra)), 0, 0);
            sprite1.drawString(formatDMS(decimalToDMS(refFinal.dec)), 120, 0);
        }

        // Ligne 2 : RSSI + Heure
        sprite1.setTextColor(TFT_MAGENTA);
        sprite1.drawString("RSSI ", 0, 30);
        sprite1.drawFloat(WiFi.RSSI(), 0, 60, 30);

        sprite1.setTextColor(TFT_YELLOW);
        sprite1.drawNumber(hour + localUtcOffset, 120, 30);
        sprite1.drawNumber(minute, 160, 30);
        sprite1.drawNumber(second, 200, 30);

        // Ligne 3 : Coordonnées GPS
        sprite1.setTextColor(TFT_CYAN);
        sprite1.drawString(formatDMS(decimalToDMS(latitude)), 0, 60);
        sprite1.drawString(formatDMS(decimalToDMS(longitude)), 120, 60);

        // Ligne 4 : Azimuth/Altitude
        sprite1.setTextColor(TFT_GREEN);
        sprite1.drawFloat(Ref_azimuth, 2, 0, 90);
        sprite1.drawFloat(Ref_altitude, 2, 120, 90);

        sprite1.pushSprite(0, 0);
    }
    else
    {
        // --- Sprite 2 ---
        sprite1.fillSprite(TFT_BLACK);

        // Ligne 5 : Coordonnées célestes + IRcommande
        sprite1.setTextColor(TFT_WHITE);
        sprite1.drawFloat(azi_gap_G, 2, 0, 0);
        sprite1.drawFloat(alt_gap_G, 2, 100, 0);
        sprite1.drawString(IRcommande, 200, 0);

        // Ligne 6 : Mode + DRatio + Indicateur WiFi
        sprite1.setTextColor(TFT_MAGENTA);
        sprite1.drawString(mode, 0, 25);

        // Couleur en fonction de DRatio
        if (Dratio == "L")
            sprite1.setTextColor(TFT_GREEN);
        else if (Dratio == "MM")
            sprite1.setTextColor(TFT_YELLOW);
        else if (Dratio == "HHH")
            sprite1.setTextColor(TFT_RED);
        sprite1.drawString(Dratio, 140, 25);

        if (WiFi.softAPgetStationNum() > 0)
        {
            if (millis() - memo_temps_clignotant >= 250)
            {
                clignotant = !clignotant;
                memo_temps_clignotant = millis();
            }
            sprite1.fillRect(200, 25, 10, 12, clignotant ? TFT_GREEN : TFT_BLACK);
        }
        else
        {
            sprite1.fillRect(200, 25, 10, 12, TFT_RED);
        }

        // Ligne 7 : Coordonnées de référence
        sprite1.setTextColor(TFT_WHITE);
        sprite1.drawFloat(myAstro.getRAdec(), 2, 0, 50);
        sprite1.drawFloat(myAstro.getDeclinationDec(), 2, 140, 50);

        // Ligne 8 : Codeurs (couleur selon chercheur)
        sprite1.setTextColor(chercheur ? TFT_RED : TFT_CYAN);
        sprite1.drawNumber(encoder_azi, 0, 75);
        sprite1.drawNumber(encoder_alt, 140, 75);

        // Ligne 9 : Position calculée
        sprite1.setTextColor(TFT_WHITE);
        sprite1.drawString("azi ", 0, 100);
        sprite1.drawFloat(mes_position_azi, 2, 40, 100);
        sprite1.drawString(" alt ", 110, 100);
        sprite1.drawFloat(mes_position_alt, 2, 170, 100);

        sprite1.pushSprite(0, 120);
    }
}

void moniteur()
{
    static bool validation_affichage = true;
    static unsigned long memo_temps1 = 0;
    // affichage mointeur serie
    if (millis() > 5000 && validation_affichage)
    { // affichage sur moniteur apres demarrage

        // traceur serie
        if (millis() - memo_temps1 < 200) // 200
            return;
        memo_temps1 = millis();

        // Serial.printf("Flash: %u/%u bytes | RAM: %u bytes\n",
        // ESP.getSketchSize(),
        // ESP.getFlashChipSize(),
        // ESP.getFreeHeap());

        Serial.print(">ecart_azi:");
        Serial.println(ecart_azi);

        Serial.print(">ecart_alt:");
        Serial.println(ecart_alt);

        // Serial.print(">Ref_azimuth:");
        // Serial.println(Ref_azimuth);

        // Serial.print(">Ref_altitude:");
        // Serial.println(Ref_altitude);

        Serial.print(">encoder_azi:");
        Serial.println(encoder_azi);

        Serial.print(">encoder_alt:");
        Serial.println(encoder_alt);

        // Serial.print(">consigne_azi:");
        // Serial.println(consigne_azi * 1000);

        // Serial.print(">consigne_alt:");
        // Serial.println(consigne_alt * 1000);

        // Serial.print(">mes_position_azi:");
        // Serial.println(mes_position_azi);

        Serial.print(">azi_GI_ref_theorique:");
        Serial.println(azi_GI_ref_theorique);

        Serial.print(">alt_GI_ref_theorique:");
        Serial.println(alt_GI_ref_theorique);

        Serial.print(">setpoint_azi:");
        Serial.println(Setpoint_azi);

        Serial.print(">setpoint_alt:");
        Serial.println(Setpoint_alt);

        // Serial.print(">azi_conti:");
        // Serial.println(Stepper_azi->isRunningContinuously());
        // Serial.print(">alt_conti:");
        // Serial.println(Stepper_alt->isRunningContinuously());

        // Serial.print(">vit_mhz_azi:");
        // Serial.println(Stepper_azi->getCurrentSpeedInMilliHz(false));
        // Serial.print(">vit_mhz_alt:");
        // Serial.println(Stepper_alt->getCurrentSpeedInMilliHz(false));

        // Serial.print(">getPosition_azi:");
        // Serial.println(Stepper_azi->getCurrentPosition());
        // Serial.print(">getPosition_alt:");
        // Serial.println(Stepper_alt->getCurrentPosition());

        // Serial.print(">queue_entries_azi:");
        // Serial.println(Stepper_azi->queueEntries());
        // Serial.print(">queue_entries_alt:");
        // Serial.println(Stepper_alt->queueEntries());

        // Serial.print(">accel_azi:");
        // Serial.println(Stepper_azi->getCurrentAcceleration());
        // Serial.print(">accel_alt:");
        // Serial.println(Stepper_alt->getCurrentAcceleration());

        // Serial.print(">ticks_azi:");
        // Serial.println(Stepper_azi->ticksInQueue());
        // Serial.print(">ticks_alt:");
        // Serial.println(Stepper_alt->ticksInQueue());

        // Serial.print(">rampState_azi:");
        // Serial.println(Stepper_azi->rampState());
        // Serial.print(">rampState_alt:");
        // Serial.println(Stepper_alt->rampState());

        // Serial.print(">azi_run:");
        // Serial.println(Stepper_azi->isRunning());
        // Serial.print(">alt_run:");
        // Serial.println(Stepper_alt->isRunning());
        // Serial.print(">foc_run:");
        // Serial.println(Stepper_foc->isRunning());

        // Serial.print(">mem_EQ:");
        // Serial.println(mem_EQ);

        // Serial.print(">chercheur:");
        // Serial.println(chercheur);

        // Serial.print(">cherchMem:");
        // Serial.println(chercheur_memo);

        // Serial.print(">delta_azi:");
        // Serial.println(delta_cherche_azi);

        // Serial.print(">delta_alt:");
        // Serial.println(delta_cherche_alt);

        // Serial.print(">en_position:");
        // Serial.println(en_position);

        // Serial.print(">temps_de_cycle:");
        // Serial.println(temps_de_cycle);

        // Serial.print(">PinOutput:");
        // Serial.println(digitalRead(enablePinStepper));

        // Serial.println("                       ");
        // Serial.printf("Mémoire libre (RAM) : %d octets\n", ESP.getFreeHeap());
        // Serial.printf("Nombre de cœurs : %d\n", ESP.getChipCores());
        // Serial.printf("Fréquence du CPU : %d MHz\n", ESP.getCpuFreqMHz());
        // Serial.printf("Taille de la Flash : %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
        // Serial.printf("Version de la puce : %d\n", ESP.getChipRevision());
        // Serial.printf("Identifiant du Chip : %04X\n", ESP.getEfuseMac());
        // Serial.printf("Version de l'ESP-IDF : %s\n", ESP.getSdkVersion());
        // Serial.printf("Modèle de la puce : %s\n", ESP.getChipModel());
        // Serial.printf("Cycle CPU : %lu\n", ESP.getCycleCount());
        // Serial.printf("Espace libre pour le sketch : %d octets\n", ESP.getFreeSketchSpace());
        // Serial.printf("Taille totale de la Heap : %d octets\n", ESP.getHeapSize());
        // Serial.printf("Taille actuelle du Sketch : %d octets\n", ESP.getSketchSize());
        // Serial.printf("Version de l'ESP-IDF (Firmware) : %s\n", ESP.getSdkVersion());
        // Serial.printf("Cycle CPU : %ld\n", temps_de_cycle);
        // Serial.println(alt_GI_ref_theorique);

        // Serial.printf("Cycle CPU : %ld\n", temps_de_cycle);
        // Serial.printf("ALT: %.2f AZ: %.2f | vALT: %.5f vAZ: %.5f\n", alt, az, refFinal.dec, refFinal.ra);

        // cycle affichage
        // fin de traceur serie
    } // fin affichage sur moniteur
}

// ===============================
// Stellarium Tab / SkySafari
// ===============================

void handleLX200Client(WiFiServer &server, WiFiClient &client, const char *name)
{
    static Buffer buffer;
    static bool clientAnnounced = false;

    if (!client || !client.connected())
    {
        WiFiClient newClient = server.available();
        if (newClient)
        {
            client = newClient;
            clientAnnounced = false;
        }
    }

    if (!client || !client.connected())
        return;

    if (!clientAnnounced)
    {
        Serial.print("Client connecté sur ");
        Serial.println(name);
        clientAnnounced = true;
        if (name == "SkySafari")
            skysafari = true;
        else
            skysafari = false;
    }

    while (client.available())
    {
        char c = client.read();

        if (!buffer.add(c))
            continue;

        String cmd = buffer.cb;

        if (cmd.endsWith("#"))
            cmd.remove(cmd.length() - 1);

        cmd.trim();
        if (cmd.isEmpty())
        {
            buffer.flush();
            continue;
        }

        String response = handleLX200(cmd);
        // if (!(cmd == ":GR" || cmd == ":GD" || cmd == ":D" || cmd == ":GW"))
        if (!(cmd == ":GR" || cmd == ":GD" || cmd == ":D"))
        {
            Serial.print("Commande brute reçue  : ");
            Serial.println(cmd);
            Serial.print("  Réponse envoyée   : ");
            Serial.println(response);
        }
        client.print(response);
        buffer.flush();
    }

    if (!client.connected())
    {
        Serial.print(name);
        Serial.println(" déconnecté");
        client.stop();
        client = WiFiClient();
        clientAnnounced = false;
    }
}

// ===============================
// Stellarium PC
// ===============================

void handleStellariumPC()
{
    // --- Connexion ---
    if (!stellariumPcClient || !stellariumPcClient.connected())
    {
        stellariumPcClient = stellariumPcServer.available();

        if (stellariumPcClient)
        {
            // Récupération automatique de l'adresse IP du PC
            adresse_IP_PC = stellariumPcClient.remoteIP().toString();

            // Construction de l'URL Stellarium PC
            url_stellarium_location =
                "http://" + adresse_IP_PC +
                ":" + String(stellariumPort) +
                "/api/location/setlocationfields";

            Serial.println("================================");
            Serial.println("Stellarium PC connecté");
            Serial.println("IP du PC : " + adresse_IP_PC);
            Serial.println("URL location : " + url_stellarium_location);
            Serial.println("================================");
        }

        return;
    }

    // --- Lecture des données ---
    if (!stellariumPcClient.available())
        return;

    uint8_t buf[64];
    size_t len = stellariumPcClient.read(buf, sizeof(buf));

    if (len == 0)
        return;

    Serial.printf("Paquet Stellarium PC : %u octets\n",
                  (unsigned)len);

    Serial.print("Data : ");

    for (size_t i = 0; i < len; i++)
        Serial.printf("%02X ", buf[i]);

    Serial.println();

    decodePointerPacket(buf, len);
}

// commandes mouvements
void handleMenuIR()
{
    irController.update();

    IRcommande = "";

    if (irController.hasNewCommand())
    {
        IRcommande = irController.getCommand();
        irController.clearCommand();
        Serial.print("IRcommande  ");
        Serial.println(IRcommande);
    }
}
void handleWebCommandPulse()
{
    if (Wcommande != commande_1)
    {
        WEBcommande = Wcommande; // une seule pulse prise en compte
        commande_1 = WEBcommande;
        debut_cmd = millis();
    }

    if (millis() - debut_cmd > 10) // 1000
    {
        WEBcommande = "";
        Wcommande = "";
    }
}
void decodeWebCommands()
{
    static double ref_de_test = 0;
    if (Wcommande.startsWith("ana1"))
        sscanf(Wcommande.c_str(), "ana1 %lf", &cmd_val1); // Extraire le nombre %d  pour int
    if (Wcommande.startsWith("ana2"))
        sscanf(Wcommande.c_str(), "ana2 %lf", &cmd_val2); // Extraire le nombre
    if (Wcommande.startsWith("ana3"))
        sscanf(Wcommande.c_str(), "ana3 %lf", &cmd_val3); // Extraire le nombre
    if (Wcommande.startsWith("ana4"))
        sscanf(Wcommande.c_str(), "ana4 %lf", &cmd_val4); // Extraire le nombre

    if (Wcommande.startsWith("latDecimal"))
        sscanf(Wcommande.c_str(), "latDecimal %lf", &latitude_popup); // Extraire le nombre

    if (Wcommande.startsWith("lonDecimal"))
        sscanf(Wcommande.c_str(), "lonDecimal %lf", &longitude_popup); // Extraire le nombre

    // ref de test
    if (millis() - timer8 >= cmd_val4)
    { // simulation
        // if (ref_a_1){
        ref_de_test = cmd_val6;
        // ref_a_1=0;
        timer8 = millis();
    }

    if (millis() - timer8 >= cmd_val5)
    { // simulation
        // if (ref_a_1){
        ref_de_test = 0;
        // ref_a_1=0;
    } // fin de ref de test
}
void handleModes()
{
    // ---------------- SYNCHRO STELLARIUM ----------------
    static unsigned long temps_debut_pulse = 0;
    if (StelSyncCM && !StelSyncMem)
    {
        StelSync = true;
        temps_debut_pulse = millis();
    }

    if (StelSyncCM && millis() - temps_debut_pulse > 1000)
    {
        StelSync = false;
        StelSyncCM = false;
    }

    StelSyncMem = StelSyncCM;

    // ---------------- INIT PARKING ----------------
    bool autorise_parking = (mode == "arret" || mode == "P_nord" || mode == "P_sud");

    if (autorise_parking &&
        ((IRcommande == "2" && !affich_liste) || WEBcommande == "init parking nord"))
    {
        // init parking nord: Raz codeurs
        encoder_azi = 0;
        encoder_alt = 0;
        azi_gap_P = alt_gap_P = azi_gap_S = alt_gap_S = 0;
    }

    if (autorise_parking &&
        ((IRcommande == "3" && !affich_liste) || WEBcommande == "init parking sud"))
    {
        // init parking sud: Raz codeurs
        encoder_azi = step_per_rev_azi / 2;
        encoder_alt = 0;
        azi_gap_P = alt_gap_P = azi_gap_S = alt_gap_S = 0;
    }

    if ((IRcommande == ">>" || WEBcommande == "EQ" || StelSync || Cmde_EQ) && !mem_EQ)
    {
        mem_EQ = true;
        instant_mem_EQ = millis();
    }

    if (mem_EQ)
    {
        azi_GI_ref_theorique = Ref_azimuth * step_per_deg_azi;
        alt_GI_ref_theorique = Ref_altitude * step_per_deg_alt;

        encoder_azi = azi_GI_ref_theorique;
        encoder_alt = alt_GI_ref_theorique;

        if (millis() > instant_mem_EQ + 1000)
        {
            mem_EQ = false;
            Cmde_EQ = false;
        }
    }

    if (IRcommande == "7" || WEBcommande == "suivit")
        mode = "suivit";
    else if (IRcommande == "5" || WEBcommande == "parking nord")
        mode = "P_nord";
    else if ((IRcommande == "6" && !affich_liste) || WEBcommande == "parking sud")
        mode = "P_sud";
    else if (WEBcommande == "360")
        mode = "360";
    else if (WEBcommande == "zenith")
        mode = "zenith";
    else if (IRcommande == "4" || WEBcommande == "pointage")
        mode = "pointage";
    else if (WEBcommande == "tour")
        mode = "tour";
    else if (IRcommande == "9" || WEBcommande == "arret")
        mode = "arret";
}

void handleSpeedChoice()
{
    if (IRcommande == "0" || WEBcommande == "lent")
        Dratio = "L";
    else if (IRcommande == "100" || WEBcommande == "moyen")
        Dratio = "MM";
    else if (IRcommande == "200" || WEBcommande == "rapide")
        Dratio = "HHH";

    if (Dratio == "L")
    {
        delta = 0.03;
        coef_rayon = 0.7;
    }
    else if (Dratio == "MM")
    {
        delta = 0.1;
        coef_rayon = 2;
    }
    else if (Dratio == "HHH")
    {
        delta = 0.25;
        coef_rayon = 3;
    }
}
void handleManualOffsets()
{
    if (IRcommande == "<<" || WEBcommande == "gauche" || StelCommandeGauche)
        azi_gap_S -= delta;

    if (IRcommande == ">|" || WEBcommande == "droit" || StelCommandeDroit)
        azi_gap_S += delta;

    if (IRcommande == "CH" || WEBcommande == "haut" || StelCommandeHaut)
        alt_gap_S += delta;

    if (IRcommande == "+" || WEBcommande == "bas" || StelCommandeBas)
        alt_gap_S -= delta;

    if (IRcommande == ">>" || IRcommande == "EQ" || WEBcommande == "EQ" || WEBcommande == "raz_delta" || StelSync)
    {
        azi_gap_P = alt_gap_P = azi_gap_S = alt_gap_S = 0; // RAZ AZI GAP suivi et pointage
    }
}

void computeConsignes()
{
    static double consigne_azi_mem = 0;
    static double consigne_alt_mem = 0;
    static bool mem_pointage = 0;
    static bool cmd22mem = false;
    static bool mem_tour = false;
    static int compteTour = 0;

    if (mode == "suivit")
    {
        consigne_azi = Ref_azimuth + azi_gap_S;
        consigne_alt = Ref_altitude + alt_gap_S;
    }

    if (mode == "P_nord")
    {
        consigne_azi = 0;
        consigne_alt = 0;
        azi_gap_P = alt_gap_P = azi_gap_S = alt_gap_S = 0;
        if ((((mes_position_azi > 359) || (mes_position_azi < 1)) && !Stepper_azi->isRunning()) && ((mes_position_alt < 2) && !Stepper_alt->isRunning()))
        {
            mode = "arret";
        }
    }

    if (mode == "P_sud")
    {
        consigne_azi = 180;
        consigne_alt = 0;
        azi_gap_P = alt_gap_P = azi_gap_S = alt_gap_S = 0;
        if ((((mes_position_azi > 179) && (mes_position_azi < 181)) && !Stepper_azi->isRunning()) && ((mes_position_alt < 2) && !Stepper_alt->isRunning()))
        {
            mode = "arret";
        }
    }

    if (mode == "zenith")
    {
        consigne_alt = 90;

        if (mes_position_alt > 88 && !Stepper_alt->isRunning())
        {
            mode = "arret";
        }
    }

    // correction pointage terrestre
    if (mode == "pointage")
    {
        if (!mem_pointage)
        {
            consigne_azi_mem = encoder_azi / step_per_deg_azi; // memorisation des consignes actuelles
            consigne_alt_mem = encoder_alt / step_per_deg_alt;
            mem_pointage = true;
        }

        if (IRcommande == "<<" || WEBcommande == "gauche" || StelCommandeGauche)
            azi_gap_P -= delta * 2;
        if (IRcommande == ">|" || WEBcommande == "droit" || StelCommandeDroit)
            azi_gap_P += delta * 2;
        if (IRcommande == "CH" || WEBcommande == "haut" || StelCommandeHaut)
            alt_gap_P += delta * 2;
        if (IRcommande == "+" || WEBcommande == "bas" || StelCommandeBas)
            alt_gap_P -= delta * 2;

        if (cmd_val1 > 360)
            cmd_val1 = 360;
        if (cmd_val1 < -360)
            cmd_val1 = -360;

        if (WEBcommande == "+ xx" && !cmd22mem)
        {
            azi_gap_P += cmd_val1;
            if (azi_gap_P > 360)
                azi_gap_P -= 360;
            if (azi_gap_P < -360)
                azi_gap_P += 360;
            cmd22mem = true;
        }

        if (WEBcommande = !"+ xx")
            cmd22mem = false;

        consigne_azi = consigne_azi_mem + azi_gap_P;
        consigne_alt = consigne_alt_mem + alt_gap_P;

        azi_gap_G = azi_gap_P;
        alt_gap_G = alt_gap_P;
    }
    else
    {
        mem_pointage = false;
        azi_gap_G = azi_gap_S;
        alt_gap_G = alt_gap_S;
    }

    if (mode != "pointage")
    {
        if (consigne_alt < -5) // limite de mouvement
            consigne_alt = -5;
        if (consigne_alt > 90)
            consigne_alt = 90;
    }

    static bool mem_360;
    static unsigned long memo_temps1 = 0;

    //  cmd de mouvements,  360°:
    if (mode == "360" && !mem_360)
    {
        memo_temps1 = millis();
        Stepper_azi->setSpeedInMilliHz(frequence_max_moteur_hz * 1000 * diviseur_step_azi); // 12000
        // Stepper_azi->setSpeedInHz(cmd_val1* diviseur_step_azi);   // 12000
        Stepper_azi->setAcceleration(frequence_max_moteur_hz_s * diviseur_step_azi); // 4000
        // Stepper_azi->setAcceleration(cmd_val2 * diviseur_step_azi); // 4000
        encoder_azi = 0;
        Stepper_azi->move(step_per_rev_azi); // lancement du mouvement continu un tour complet
        // Stepper_azi->move(cmd_val3 * diviseur_step_azi); // lancement du mouvement continu
        // Stepper_azi->move(cmd_val3); // lancement du mouvement continu
        encoder_alt = 0;
        consigne_azi = 0;
        mem_360 = true;
    }

    if ((mem_360 && millis() - memo_temps1 > 1000 && !Stepper_azi->isRunning()) || mode == "arret") // 360000 pour 360° à 1000Hz
    {
        mem_360 = false;
        consigne_azi = 0;
        mode = "arret";
    }

    static unsigned long memo_temps2 = 0;
    if (mode == "tour" && !mem_tour)
    {
        memo_temps2 = millis();
        Stepper_azi->setSpeedInMilliHz(frequence_max_moteur_hz * 1000 * diviseur_step_azi); // 12000
        Stepper_azi->setAcceleration(frequence_max_moteur_hz_s * diviseur_step_azi);        // 4000
        encoder_azi = 0;
        Stepper_azi->move(step_per_rev_azi * cmd_val1); // lancement du mouvement continu
        encoder_alt = 0;
        consigne_azi = 0;
        compteTour = 0;
        mem_tour = true;
    }

    static bool top_tour = false;
    if ((mem_tour && millis() - memo_temps2 > 1000 && !Stepper_azi->isRunning()) || mode == "arret")
    {
        mem_tour = false;
        consigne_azi = 0;
        mode = "arret";
    }

    if (mes_position_azi > 1 && mes_position_azi < 2 && !top_tour)
    {
        top_tour = true;
        compteTour++;
    }

    if (mes_position_azi > 3)
    {
        top_tour = false;
    }

    // Serial.print(">compteTour:");
    // Serial.println(compteTour);

    Setpoint_azi = consigne_azi * step_per_deg_azi;
    Setpoint_alt = consigne_alt * step_per_deg_alt;
}

void handleChercheur()
{
    static unsigned long memo_temps4 = 0;
    static double delta_cherche_azi_prev = 0;
    static double delta_cherche_alt_prev = 0;
    static double alpha = 0;
    static double theta = 0;
    double angle_rot = 720;
    double angle_arret = 100; // angle d'arret du chercheur en degrés

    // Serial.print(">rayon:");
    // Serial.println(rayon);

    if (IRcommande == "7" || WEBcommande == "suivit" || rayon > angle_rot + angle_arret) // arret raz et  retour en suivit
    {                                                                                    // raz et  retour en suivit
        // instant_sortie_chercheur = millis();
        rayon = 0;
        chercheur = false;
    }

    if ((IRcommande == "CH+" || WEBcommande == "cherche") && millis() > instant_sortie_chercheur + 500)
    {
        chercheur = true; // demarrage chercheur
    }
    if (chercheur)
    {
        chercheur_memo = true;
        instant_sortie_chercheur = millis();                                                // mémorise le dernier moment où A était true
        Stepper_azi->setSpeedInMilliHz(frequence_max_moteur_hz * 1000 * diviseur_step_azi); // 12000
        Stepper_azi->setAcceleration(frequence_max_moteur_hz_s * diviseur_step_azi);        // 4000
        Stepper_alt->setSpeedInMilliHz(frequence_max_moteur_hz * 1000 * diviseur_step_alt); // 12000
        Stepper_alt->setAcceleration(frequence_max_moteur_hz_s * diviseur_step_alt);        // 4000

        if (millis() - memo_temps4 > 200) // 100
        {
            rayon += inc_rayon;
            memo_temps4 = millis();

            if (rayon < angle_rot)
                alpha = rayon;
            else
                alpha = angle_rot;

            if (rayon < angle_rot)
                theta = rayon / angle_rot;
            else
                theta = (angle_rot + angle_arret - rayon) / angle_arret;

            // Serial.print(">alpha:");
            // Serial.println(alpha);

            // Serial.print(">theta:");
            // Serial.println(theta);

            delta_cherche_alt = (coef_rayon * theta) * sin(0.01745 * alpha) * 20 * diviseur_step_azi; // 1000 points
            delta_cherche_azi = (coef_rayon * theta) * cos(0.01745 * alpha) * 20 * diviseur_step_alt;

            Stepper_azi->move(delta_cherche_azi - delta_cherche_azi_prev);
            Stepper_alt->move(delta_cherche_alt - delta_cherche_alt_prev);

            // Serial.print(">delta:");
            // Serial.println(delta_cherche_azi - delta_cherche_azi_prev);

            delta_cherche_alt_prev = delta_cherche_alt;
            delta_cherche_azi_prev = delta_cherche_azi;
        }

        if (rayon > 10 && (IRcommande == "CH+" || WEBcommande == "cherche")) // sortie du chercheur apres 2eme coup
        {                                                                    // 2eme coup  sortie du chercheur
            rayon = 0;
            chercheur = false;
            Cmde_EQ = true;
            delta_cherche_azi = 0;
            delta_cherche_alt = 0;
            delta_cherche_alt_prev = 0;
            delta_cherche_azi_prev = 0;
        }
    }
    else
    {
        delta_cherche_azi = 0;
        delta_cherche_alt = 0;
        delta_cherche_alt_prev = 0;
        delta_cherche_azi_prev = 0;
        if (millis() - instant_sortie_chercheur >= 2000)
        {
            chercheur_memo = false;
        }
    }
    // Serial.print(">delta_cherche_azi:");
    // Serial.println(delta_cherche_azi);
}

void updateEncoder(FastAccelStepper *stepper,
                   int32_t &encoder,
                   int32_t &brut,
                   int32_t &brut_prev)
{
    brut_prev = brut;
    brut = stepper->getCurrentPosition();
    encoder += (brut - brut_prev);
}

void normalizeAzi()
{
    if (encoder_azi < 0)
        encoder_azi += step_per_rev_azi;

    if (encoder_azi > step_per_rev_azi)
        encoder_azi -= step_per_rev_azi;
}

void normalizeEcartAzi()
{
    if (ecart_azi >= step_per_rev_azi / 2)
        ecart_azi -= step_per_rev_azi;

    if (ecart_azi <= -step_per_rev_azi / 2 - 10000)
        ecart_azi += step_per_rev_azi;
}

void gestionJeu(long &setpoint,
                long ecart,
                bool &neg_mem,
                double step_per_deg,
                double depassement,
                double seuil,
                bool negatif)
{
    double seuil_points = step_per_deg * seuil;
    double depassement_points = step_per_deg * depassement;

    if (negatif)
    {
        // Rattrapage de jeu dans le sens négatif
        if (ecart < -seuil_points && !neg_mem)
        {
            setpoint -= depassement_points;
            neg_mem = true;
            // Serial.println("JEU NEGATIF");
        }

        // Réarmement lorsque l'erreur repasse positive
        if (ecart > 0)
        {
            neg_mem = false;
        }
    }
    else
    {
        // Rattrapage de jeu dans le sens positif
        if (ecart > seuil_points && !neg_mem)
        {
            setpoint += depassement_points;
            neg_mem = true;
            // Serial.println("JEU POSITIF");
        }

        // Réarmement lorsque l'erreur repasse négative
        if (ecart < 0)
        {
            neg_mem = false;
        }
    }
}

void gestionStepperMoveto(FastAccelStepper *stepper,
                          long setpoint, float diviseur_step)

{

    stepper->setSpeedInMilliHz(frequence_max_moteur_hz * 1000 * diviseur_step);
    stepper->setAcceleration(frequence_max_moteur_hz_s * diviseur_step);

    stepper->moveTo(setpoint);
}

void gestionMoveAzi(FastAccelStepper *stepper, long ecart, float diviseur_step)
{
    stepper->setAcceleration(frequence_max_moteur_hz_s * diviseur_step);
    stepper->setSpeedInMilliHz(frequence_max_moteur_hz * 1000 * diviseur_step);

    static bool mouvementEnCours = false;

    // ------------------------------------------------
    // Un move() est en cours :
    // on ignore complètement les nouveaux écarts
    // ------------------------------------------------
    if (mouvementEnCours)
    {
        if (!stepper->isRunning())
        {
            mouvementEnCours = false;
        }

        return;
    }

    // ------------------------------------------------
    // Aucun mouvement en cours :
    // on prend UNE SEULE FOIS l'écart actuel
    // ------------------------------------------------
    if (fabs(ecart) > 1)
    {
        long pas = lround(ecart);

        if (pas != 0)
        {
            stepper->move(pas);

            mouvementEnCours = true;
        }
    }
}

void gestionMoveAlt(FastAccelStepper *stepper, long ecart, float diviseur_step)
{
    stepper->setAcceleration(frequence_max_moteur_hz_s * diviseur_step);
    stepper->setSpeedInMilliHz(frequence_max_moteur_hz * 1000 * diviseur_step);

    static bool mouvementEnCours = false;

    // ------------------------------------------------
    // Un move() est en cours :
    // on ignore complètement les nouveaux écarts
    // ------------------------------------------------
    if (mouvementEnCours)
    {
        if (!stepper->isRunning())
        {
            mouvementEnCours = false;
        }

        return;
    }

    // ------------------------------------------------
    // Aucun mouvement en cours :
    // on prend UNE SEULE FOIS l'écart actuel
    // ------------------------------------------------
    if (fabs(ecart) > 1)
    {
        long pas = lround(ecart);

        if (pas != 0)
        {
            stepper->move(pas);

            mouvementEnCours = true;
        }
    }
}

void driveSteppers()
{
    static int32_t encoder_azi_brut = 0;
    static int32_t encoder_azi_brut_prev = 0;

    static int32_t encoder_alt_brut = 0;
    static int32_t encoder_alt_brut_prev = 0;

    static bool azi_neg_mem = false;
    static bool alt_neg_mem = false;

    updateEncoder(Stepper_azi,
                  encoder_azi,
                  encoder_azi_brut,
                  encoder_azi_brut_prev);

    updateEncoder(Stepper_alt,
                  encoder_alt,
                  encoder_alt_brut,
                  encoder_alt_brut_prev);

    normalizeAzi();

    double depassement = 2;       // en degrés
    double seuil_detection = 0.3; // en degrés

    ecart_azi = Setpoint_azi - encoder_azi; // avant jeu
    ecart_alt = Setpoint_alt - encoder_alt;

    normalizeEcartAzi();

    gestionJeu(Setpoint_azi,
               ecart_azi,
               azi_neg_mem,
               step_per_deg_azi,
               depassement,
               seuil_detection,
               true);

    gestionJeu(Setpoint_alt,
               ecart_alt,
               alt_neg_mem,
               step_per_deg_alt,
               depassement,
               seuil_detection,
               true);

    ecart_azi = Setpoint_azi - encoder_azi; // apres jeu
    ecart_alt = Setpoint_alt - encoder_alt;

    normalizeEcartAzi();

    if (mode == "arret" || millis() < 1500)
    {
        Stepper_azi->stopMove();
        Stepper_alt->stopMove();
        return;
    }

    if ((mode == "suivit" || mode == "pointage" || mode == "P_nord" ||
         mode == "P_sud" || mode == "zenith") &&
        !chercheur_memo)
    {
        gestionMoveAzi(Stepper_azi,
                       ecart_azi,
                       diviseur_step_azi);

        gestionMoveAlt(Stepper_alt,
                       ecart_alt,
                       diviseur_step_alt);
    }
}
// fin commandes mouvements

// fonctions de setup
void initSerial()
{
    Serial.begin(115200);
    delay(50);
    Serial.println("=== Démarrage ESP32 ===");
    Serial.printf("Heap libre au boot: %u\n", ESP.getFreeHeap());
}
void initAstro()
{
    myAstro.begin();
    myAstro.rejectDST();
    myAstro.setTimeZone(0);
    myAstro2.begin();
    myAstro2.rejectDST();
    myAstro2.setTimeZone(0);
}
void initSteppers()
{
    engine.init();

    Stepper_azi = engine.stepperConnectToPin(stepPinStepper_azi);
    if (Stepper_azi)
    {
        Stepper_azi->setDirectionPin(dirPinStepper_azi, false);
        Stepper_azi->setAutoEnable(true);
        Stepper_azi->setDelayToDisable(500);
        Stepper_azi->setCurrentPosition(0);
    }

    Stepper_alt = engine.stepperConnectToPin(stepPinStepper_alt);
    if (Stepper_alt)
    {
        Stepper_alt->setDirectionPin(dirPinStepper_alt, false);
        Stepper_alt->setAutoEnable(true);
        Stepper_alt->setDelayToDisable(500);
        Stepper_alt->setCurrentPosition(0);
    }

    Stepper_foc = engine.stepperConnectToPin(stepPinStepper_foc);
    if (Stepper_foc)
    {
        Stepper_foc->setDirectionPin(dirPinStepper_foc, false);
        Stepper_foc->setAutoEnable(true);
        Stepper_foc->setDelayToDisable(500);
        Stepper_foc->setSpeedInHz(12000);   // 8000
        Stepper_foc->setAcceleration(8000); // 16000
        Stepper_foc->setCurrentPosition(5000 * diviseur_step_foc);
    }

    pinMode(enablePinStepper, OUTPUT);
    digitalWrite(enablePinStepper, HIGH); // Désactiver au départ
}

void initWiFi()
{
    // =========================================================
    // MODE AUTONOME : l'ESP32 crée son propre réseau WiFi.
    // Aucun routeur, aucune box et aucun accès Internet nécessaires.
    // La tablette se connecte directement à ce réseau.
    // =========================================================
    WiFi.mode(WIFI_AP);

    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    WiFi.softAPConfig(local_IP, gateway, subnet);

    const char *ssid = "TELESCOPE";

    if (!WiFi.softAP(ssid, NULL, 6, false, 4))
    {
        Serial.println("ERREUR : impossible de démarrer le point d'accès WiFi");
        return;
    }

    WiFi.setSleep(false);
    esp_wifi_set_max_tx_power(20);

    Serial.println();
    Serial.println("========================================");
    Serial.println(" WiFi AUTONOME ESP32");
    Serial.println("========================================");
    Serial.print("SSID : ");
    Serial.println(ssid);
    Serial.print("IP   : ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Port Stellarium : 4030");
    Serial.println("========================================");
}

void initWebSocket()
{
    webSocket.begin();
    webSocket.onEvent(webSocketEventCmd);
}
void initIR()
{
    irController.begin();
}
void initOTA()
{
    ArduinoOTA.setHostname("teles");

    ArduinoOTA.onStart([]()
                       {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
        else type = "filesystem";
        Serial.println("Début de la mise à jour : " + type); });

    ArduinoOTA.onEnd([]()
                     { Serial.println("\nFin de la mise à jour."); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progression : %u%%\r", (progress / (total / 100))); });

    ArduinoOTA.onError([](ota_error_t error)
                       {
        Serial.printf("Erreur[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Erreur d'authentification");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Erreur au début");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Erreur de connexion");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Erreur de réception");
        else if (error == OTA_END_ERROR) Serial.println("Erreur de fin"); });

    ArduinoOTA.begin();
}
void initTft()
{
    tft.init();
    tft.setRotation(2);

    Serial.print("avant   ");
    Serial.println(ESP.getFreeHeap());

    sprite1.createSprite(240, 120);

    Serial.print("apres   ");
    Serial.println(ESP.getFreeHeap());

    sprite1.setTextSize(2);
    sprite1.setTextDatum(TL_DATUM);
}

void initFilesystemAndStars()
{
    if (!LittleFS.begin())
    {
        Serial.println("Erreur lors du montage LittleFS");
        return;
    }

    File file = LittleFS.open("/stars.csv", FILE_READ);
    if (!file)
    {
        Serial.println("Fichier stars.csv introuvable");
        return;
    }
    file.close();

    loadStars();
}

void initStellarium()
{
    stellariumServer.begin();
    skySafariServer.begin();
    stellariumPcServer.begin();
    stellariumPcServer.setNoDelay(true);
    Serial.println("Serveur TCP Stellarium démarré (port 10000)");

    // if (stellariumPcClient.connect(adresse_IP_PC.c_str(), stellariumPcPort))
    // {
    //     Serial.println("Connecté à Stellarium !");
    // }
}

void initLocation()
{
    initCoordonnees();
    Serial.printf("Position observateur : %.6f / %.6f / %.1f m\n",
                  latitude, longitude, altitude);
}

// =========================
// TEMPS SIDÉRAL
// =========================
double getJulianDate(int year, int month, int day, int hour, int minute, double second)
{
    if (month <= 2)
    {
        year -= 1;
        month += 12;
    }

    int A = year / 100;
    int B = 2 - A + (A / 4);

    double JD = int(365.25 * (year + 4716)) + int(30.6001 * (month + 1)) + day + B - 1524.5;

    double dayFraction = (hour + minute / 60.0 + second / 3600.0) / 24.0;

    return JD + dayFraction;
}

double getLST(double JD, double longitude)
{
    double T = (JD - 2451545.0) / 36525.0;

    double GMST = 280.46061837 + 360.98564736629 * (JD - 2451545.0) + 0.000387933 * T * T - (T * T * T) / 38710000.0;

    double LST = GMST + longitude;

    while (LST < 0)
        LST += 360;
    while (LST >= 360)
        LST -= 360;

    return LST;
}

// debut du setup -----------------------------
void setup()
{
    uint32_t t0 = millis();
    initSerial();
    initTft();
    initAstro();
    initSteppers();
    initWiFi();
    // delay(500); // 🔑 clé
    initWebSocket();
    initRTS2Server();
    initIR();
    initOTA();
    initFilesystemAndStars();
    initStellarium();
    initLocation();

    Serial.printf("=== Setup terminé en %lu ms ===\n", millis() - t0);
}

void loop()
{
    temps_memo_debut = micros();
    // uint32_t loopStart = micros();
    TIME_BLOCK("updateEnablePin", updateEnablePin());
    TIME_BLOCK("updateGPS", updateGPS());
    TIME_BLOCK("updateTime", updateTime());
    TIME_BLOCK("updateAstroReference", updateAstroReference());
    TIME_BLOCK("focuseur", focuseur());
    TIME_BLOCK("moniteur", moniteur());
    TIME_BLOCK("handleMenuIR", handleMenuIR());
    TIME_BLOCK("handleWebCommandPulse", handleWebCommandPulse());
    TIME_BLOCK("decodeWebCommands", decodeWebCommands());
    TIME_BLOCK("handleModes", handleModes());
    TIME_BLOCK("handleSpeedChoice", handleSpeedChoice());
    TIME_BLOCK("handleManualOffsets", handleManualOffsets());
    TIME_BLOCK("computeConsignes", computeConsignes());
    TIME_BLOCK("handleChercheur", handleChercheur());
    TIME_BLOCK("driveSteppers", driveSteppers());
    TIME_BLOCK("ArduinoOTA", ArduinoOTA.handle());
    TIME_BLOCK("webSocket", webSocket.loop());

    // Clients LX200
    TIME_BLOCK("LX200 Stellarium", handleLX200Client(stellariumServer, stellariumClient, "Stellarium"));
    TIME_BLOCK("LX200 SkySafari", handleLX200Client(skySafariServer, skySafariClient, "SkySafari"));

    TIME_BLOCK("handleStellariumPC", handleStellariumPC());

    TIME_BLOCK("affichage_TFT", affichage_TFT());
    TIME_BLOCK("SendData", SendData());
    TIME_BLOCK("sendPosition", sendPosition(actual_RA_H, actual_DEC));

    temps_de_cycle = micros() - temps_memo_debut;
    // last_loop_time = micros() - loopStart;
    // if (last_loop_time > max_loop_time)
    //     max_loop_time = last_loop_time;
    yield(); // ou delay(0)
} // fin de loop
