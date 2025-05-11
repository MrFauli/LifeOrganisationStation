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
extern struct tm timeinfo;
int lastUpdate = 0;
LvglUi gui;
SoundPlayer player;
Wecker wecker;
Weather wetter;
bool uhrGesetzt = false;
bool funktion = false;


int lastMin = 62;
// Auflösung: 480 × 272 Pixel
  unsigned long lastAttempt = 0;
int16_t w, h, text_size;

int currentMin;


// WLAN-Zugangsdaten
const char* ssid = "FRITZ!Box 7560 MA";
const char* password = "17142637522306500849";

#define TOUCH_SDA 8
#define TOUCH_SCL 4
#define TOUCH_INT 3
#define TOUCH_RST 38
#define TOUCH_WIDTH 480
#define TOUCH_HEIGHT 272

TAMC_GT911 touchController = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

unsigned long lastWheater = 0;
const unsigned long wheaterInterval = 15 * 60 * 1000UL; // 15 Minuten

int button = 18;



void setup() {
  Serial.begin(115200);

  pinMode(button,INPUT_PULLUP);
  unsigned long startAttemptTime = millis();
  
  // Gib maximal 10 Sekunden Zeit, um sich zu verbinden
  while (!Serial && millis() - startAttemptTime < 5000) {
    delay(10);  // Warte, bis USB verbunden ist
  }
  delay(2000);
  Serial.println("Serial OK");


  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);


    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);  // Automatische Wiederverbindung aktivieren
    WiFi.persistent(true);        // Verbindungsdaten speichern
    WiFi.begin(ssid, password);
    startAttemptTime = millis();
  
  // Gib maximal 10 Sekunden Zeit, um sich zu verbinden
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
    delay(500);
    Serial.print(".");
  }
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

  wecker.addAlarm(16, 38);

}

void loop(){
  delay(5);
  lv_timer_handler(); /* let the GUI do its work */
  if(millis()- lastUpdate >= 1000){
  gui.updateTime();
  lastUpdate = millis();
  }

  wecker.checkTimer();
  player.updatePlaying();




  struct tm timeinfo;
  getLocalTime(&timeinfo);
  currentMin = timeinfo.tm_min;
  if(WL_CONNECTED == WiFi.status()){

  if(millis() - lastWheater >= wheaterInterval|| uhrGesetzt != true){
    wetter.getWeatherData();
    Serial.println("Temperatur");
    Serial.println(Temperatur);
    lastWheater = millis();

    }  

      uhrGesetzt = true;
      configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
      struct tm timeinfo;
  }
  else{      
       unsigned long now = millis();
       int currentMin = 62;
      if(now - lastAttempt> 10000){
        Serial.println("Try to reconnect...");
        WiFi.disconnect();  // alte Verbindung sicher trennen
        WiFi.reconnect();   // neue starten
        lastAttempt = now;
        
      }  
  }

  if(digitalRead(button) == LOW ){
    player.stopSound();
  }


}