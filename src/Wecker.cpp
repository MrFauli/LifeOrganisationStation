
#include "Wecker.h"
volatile bool played = false;
int playedMin = 61;
int playedHour = 25;
unsigned long timer;
bool timerOn;
String timerDauer = "";
int timerMin;
int timerSec;
extern bool uhrGesetzt;
#include <Arduino.h>
class singleAlarm{
    public:
        int hour;
        int minute;
        singleAlarm(int h, int m) : hour(h), minute(m) { }
};
static std::vector<singleAlarm> alarmList;
Wecker::Wecker(){};
// {
 

// }
void Wecker::addAlarm(int hour, int minute){
    alarmList.push_back(singleAlarm(hour,minute));
    Serial.println("Added Alarm: ");
    Serial.println(minute);
    
}
void Wecker::checkAlarms(){
    int length = alarmList.size();

    for(int i = 0; i< length; i++){
        makeAlarm(alarmList[i].hour,alarmList[i].minute);

    }
}


void Wecker::makeAlarm(int hour, int minute) {

    if(uhrGesetzt){
    if(getLocalTime(&timeinfo)){
        if(played==false && playedMin!=timeinfo.tm_min && playedHour!=timeinfo.tm_hour ){
            played == false;

            if(timeinfo.tm_min == minute && timeinfo.tm_hour == hour){
                Serial.println("Alarm");
                player.playMP3("/Sponge-Bob.mp3");
                playedMin = minute;
                playedHour = hour;
                played == true;
            }}}}
    }


void Wecker::startTimer(int minute){
    if(!timerOn){
    timer = millis() + 60000 * minute;
    timerOn = true;
    Serial.println("Timer gestartet " + String(minute) );
    Serial.println(timer);
    }

}

void Wecker::checkTimer(){
    
    if(timer< millis() && timerOn == true){
        timerOn = false;
        Serial.println("Timer");
        player.playMP3("/Minion-Wecker.mp3");
        timerDauer = "";
    }
    if(timerOn){
        timerMin =(timer - millis()) / 60000;
        timerSec = ((timer - millis()) % 60000)/1000;
        String timerSecStr;
        if(timerSec < 10){  
    timerSecStr = "0" + String(timerSec);
            
        }
        else{
            timerSecStr = String(timerSec);
        }
        timerDauer = String(timerMin) + ":" + timerSecStr;  getLocalTime(&timeinfo);


    }
}