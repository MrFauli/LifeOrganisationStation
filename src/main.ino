#include <arduino.h>

#include "WiFi.h"           
#include <Wire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "time.h"
#include "TAMC_GT911.h"  
#include "lvgl.h"
#include "SoundPlayer.h"
#include "Wecker.h"
#include "Weather.h"
#include "passwort.h"
#include "LvglUi.h"
#include "WebServer.h"
#include <Arduino.h>
#include <ESPmDNS.h>
#include <Preferences.h>
struct tm timeinfo;
int num = 1;
bool configured;
extern struct tm timeinfo;
int lastUpdate = 0;
LvglUi gui;
SoundPlayer player;
Wecker wecker;
Weather wetter;
bool uhrGesetzt = false;
bool funktion = false;

String home_ssid = "";
String home_password = "";

int lastMin = 62;
// Auflösung: 480 × 272 Pixel
  unsigned long lastAttempt = 0;
int16_t w, h, text_size;

int currentMin;

Preferences prefs;
WebServer server(80);  // Port 80 = HTTP

#define TOUCH_SDA 8
#define TOUCH_SCL 4
#define TOUCH_INT 3
#define TOUCH_RST 38
#define TOUCH_WIDTH 480
#define TOUCH_HEIGHT 272

TAMC_GT911 touchController = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

unsigned long lastWheater;
const unsigned long wheaterInterval = 15 * 60 * 1000UL; // 15 Minuten

int button = 18;

void WiFiReconnect(){

    if (WiFi.status() != WL_CONNECTED && millis() - lastAttempt > 20000 && home_ssid != "") {
    Serial.println("Try to reconnect...");
    WiFi.disconnect();       // alte Verbindung sicher trennen
    delay(50);                // sehr kurzer Delay, nur für Sicherheit
    WiFi.begin(home_ssid, home_password);
    lastAttempt = millis();       // Zeitpunkt merken
    } 
}

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>LOS Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0" charset="UTF-8">
</head>
<body>
  <form action="/submit" method="GET">
    <label for="alarmInput">Wähle die Uhrzeit des Weckers aus</label>
    <input type="time" id="alarmInput" name="alarmInput" required/>
    <button type="submit" >Wecker stellen</button>
  </form>
  <script>
  function sendData(action){
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/" + action, true);
      xhr.send();
  }
  </script>
</body>
</html>
)=====";
const char Start_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Configure LOS</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0" charset="UTF-8">
</head>
<body>
  <form action="/configure" method="GET">
    <label for="wlan">Wählen sie ihr Home-Wlan aus!</label>
    <input type="text" id="ssid" name="ssid" required/>
    <input type="text" id="passwort" name="passwort"/>
    <button type="submit" >WLAN einstellen</button>
  </form>
  <script>
  function sendData(action){
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/" + action, true);
      xhr.send();
  }
  </script>
</body>
</html>
)=====";
void startConfiguration(){



  server.on("/",  [](){
    server.send(200, "text/html", Start_page);
  });
  server.on("/configure",[](){
    Serial.println("Submit");
    String ssid = server.arg("ssid");
    String passwort = server.arg("passwort");
    Serial.println(ssid);
    Serial.println(passwort);
    prefs.begin("settings",false);
    Serial.println("Hey");
    prefs.putString("ssid",ssid);
    prefs.putString("passwort",passwort);
    prefs.putBool("Configured",true);

    prefs.end();
    server.send(200,"text/html","WLAN eingerichtet, LOS wird neugestartet...");
    delay(100);
    ESP.restart();
  });
  server.begin();
}
void startServer(){
  Serial.print("Verbunden, IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // Initialize mDNS
  if (!MDNS.begin("los")) {   // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");

  }
  else{
  Serial.println("mDNS responder started");
  }

  server.on("/",  [](){
    server.send(200, "text/html", MAIN_page);
  });
  server.on("/submit",[](){
    Serial.println("Submit");
    String alarmTime = server.arg("alarmInput");

    String hour = String(alarmTime[0])+String(alarmTime[1]);
    String min = String(alarmTime[3]) + String(alarmTime[4]);
    wecker.addAlarm( hour.toInt(),min.toInt());
    Serial.println(min);
    Serial.println(hour);
    prefs.begin("settings",false);
    Serial.println("Hey");
        prefs.remove("alarmHour");       // Nur diesen Key löschen
    prefs.remove("alarmMin"); 
    prefs.putInt("alarmMin",min.toInt());
    prefs.putInt("alarmHour",hour.toInt());

    int hours = prefs.getInt("alarmHour", 0); // 0 = Defaultwert, falls nicht vorhanden
  int minute = prefs.getInt("alarmMin", 0);

    Serial.print("Gespeicherte Weckzeit: ");
  Serial.print(hours);
  Serial.print(":");
  Serial.println(minute);
    prefs.end();
    server.send(200,"text/html","Wecker gestellt. Wenn sie eine neue Uhrzeit wollen,\n überschreiben sie den gestellten Wecker. \n Für mehr Wecker Plätze, kaufen sie LOS Premium ");
      
  });
  server.begin();

}

void setup() {
  Serial.begin(115200);

  pinMode(button,INPUT_PULLUP);
  unsigned long startAttemptTime = millis();
  prefs.begin("settings",false);
  configured = prefs.getBool("Configured",false);
  bool startDanke = prefs.getBool("Danke",false);
  bool startTutorial = prefs.getBool("Tutorial",false);
  // Beispiel: Wert auslesen
  int hour = prefs.getInt("alarmHour", 0); // 0 = Defaultwert, falls nicht vorhanden
  int minute = prefs.getInt("alarmMin", 0);

  Serial.print("Gespeicherte Weckzeit: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);

  if(hour != 0 && minute != 00){
    wecker.addAlarm(hour,minute);
  }
  // Gib maximal 10 Sekunden Zeit, um sich zu verbinden
  while (!Serial && millis() - startAttemptTime < 5000) {
    delay(10);  // Warte, bis USB verbunden ist
  }
  delay(2000);
  Serial.println("Serial OK");

  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);
  Serial.println(configured);
      Serial.println("true");


  if(!configured){
    Serial.println("false");
  WiFi.disconnect(true);         
  delay(1000);
  
  WiFi.mode(WIFI_AP);            
  delay(100);        
    IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);

  bool result = WiFi.softAP("LOS", "LOSBGT11", 6);

  if (!result) {
    Serial.println("Fehler beim Starten des Access Points!");
    return;
  }
    Serial.println("Verbunden!");


    prefs.end();
    startConfiguration();
    if(!startDanke){
      player.playMP3("/danke.mp3");
      prefs.begin("settings",false);
      prefs.putBool("Danke",true);
      prefs.end();

    }


  }
  else{

    String home_ssid = prefs.getString("ssid", "nö"); // 0 = Defaultwert, falls nicht vorhanden
  String home_password = prefs.getString("passwort", "fuck");

    WiFi.disconnect(true);         
    delay(1000);
    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.setAutoReconnect(true);  // Automatische Wiederverbindung aktivieren
    WiFi.persistent(true);        // Verbindungsdaten speichern
    WiFi.begin(home_ssid, home_password);
  // Gib maximal 10 Sekunden Zeit, um sich zu verbinden
  
    startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
    delay(500);
    Serial.print(".");
  }

    prefs.end();
    Serial.println(home_ssid);
    Serial.println(home_password);
    startServer();
      if(!startTutorial){
      player.playMP3("/tutorial.mp3");
      prefs.begin("settings",false);
      prefs.putBool("Tutorial",true);
      prefs.end();

    }
}


    startAttemptTime = millis();
  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){
    Serial.println(&timeinfo, "Uhrzeit: %H:%M:%S - %d.%m.%Y");

  }
  else{
    Serial.println("Fehler beim zeit aufrufen");
  }

  // init LVGL

  gui.startLVGL();

  Serial.println("Hello");
  Serial.println(configured);

}

void loop(){



  lv_timer_handler(); /* let the GUI do its work */
  if (WiFi.getMode() == WIFI_AP || WiFi.status() == WL_CONNECTED) {

  server.handleClient();}


  wecker.checkTimer();
  player.updatePlaying();
  wecker.checkAlarms();


  if(configured ){    
    if(millis()- lastUpdate >= 1000&& WiFi.getMode() == WIFI_STA){
    gui.updateTime();
    lastUpdate = millis();

    }}



  if(WL_CONNECTED == WiFi.status() && WiFi.getMode() == WIFI_STA){

    if(!uhrGesetzt){
        int currentMin = timeinfo.tm_min;
      wetter.getWeatherData();
      Serial.println("Temperatur");
      Serial.println(Temperatur);
      lastWheater = millis();
    }
  if(!uhrGesetzt && WL_CONNECTED == WiFi.status()){
    struct tm timeinfo;


    configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
      
      getLocalTime(&timeinfo);

      currentMin = timeinfo.tm_min;

    
      uhrGesetzt = true;
  }
  if(millis() - lastWheater >= wheaterInterval|| uhrGesetzt != true){
    wetter.getWeatherData();
    Serial.println("Temperatur");
    Serial.println(Temperatur);
    lastWheater = millis();
    configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
    }  
  }
  if(uhrGesetzt){
    getLocalTime(&timeinfo);

    currentMin = timeinfo.tm_min;

  }
  if(digitalRead(button) == LOW ){
    player.stopSound();
  }
    // Serial.println("Temperatur");
    // Serial.println(Temperatur);

}