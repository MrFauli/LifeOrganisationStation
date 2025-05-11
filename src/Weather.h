#ifndef WEATHER_H
#define WEATHER_H
extern float Temperatur;


class Weather {
public:
    Weather();
    void getWeatherData();
    float getTemperatur();
};

#endif