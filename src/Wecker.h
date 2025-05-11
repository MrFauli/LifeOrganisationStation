#ifndef WECKER_H
#define WECKER_H

#include "Wecker.h"
#include <Arduino.h>
#include <vector>  // Wichtig: für std::vector
#include "SoundPlayer.h"  // Damit player bekannt ist
#include <time.h>

// Externes Objekt, das du in main.ino deklariert hast
extern SoundPlayer player;



class Wecker {
public:
    Wecker();
    
    void addAlarm(int hour, int minute);
    void checkAlarms();
    void makeAlarm(int hour, int minute);
    void startTimer(int minute);
    void checkTimer();
    
};

#endif