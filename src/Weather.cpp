#include "Weather.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include <WiFiUdp.h>
float Temperatur;
Weather::Weather(){};
void Weather::getWeatherData() {

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


    } else {
        Temperatur = 123 ;

    }
    http.end();
}
float Weather::getTemperatur() {
    return Temperatur;
}