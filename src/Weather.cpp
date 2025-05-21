#include "Weather.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include <WiFiUdp.h>
#include "WiFi.h"           
float Temperatur;
Weather::Weather(){};
void Weather::getWeatherData() {
    if(WiFi.status() == WL_CONNECTED){

    
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?lat=52.38438123998307&lon=9.6634259501376&appid=fe1c7ad475e75e1eafbc59f7bc2daef7&units=metric";


    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();


        // JSON parsen
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        Temperatur = doc["main"]["temp"];
        http.end();

    } else {
        Temperatur = 123 ;

    }}

    else{
        Temperatur = 123 ;
    }
}
float Weather::getTemperatur() {
    return Temperatur;
}