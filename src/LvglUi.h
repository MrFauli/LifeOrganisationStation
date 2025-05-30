#ifndef LVGLUI_H_H
#define LVGLUI_H

#include "TAMC_GT911.h" 
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "Wecker.h"
#include "SoundPlayer.h"  // Damit player bekannt ist
#include <Arduino.h> // falls du Arduino-Funktionen brauchst
#include "WiFi.h" 
#include "Weather.h"
#include <Preferences.h>

extern Wecker wecker;
extern SoundPlayer player;
extern Weather wetter;
extern Preferences prefs;
class LvglUi {
public:
    LvglUi();
    static uint32_t millis_cb(); 


    static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
    static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);
    static void overlay_cb(lv_event_t * e);
    void startLVGL();
    static void value_changed_event_cb(lv_event_t * e);
    static void timer_Cb(lv_event_t * e);
    static void reset_Cb(lv_event_t * e);
    static void soundSpecial(lv_event_t * e);
    void TimeBox(lv_obj_t *parent);
    void InfoBox(lv_obj_t *parent);
    void ActionBox(lv_obj_t *parent);
    void Arc(lv_obj_t *parent);
    void Overlay(lv_obj_t *parent);
    void contentSidePage(lv_obj_t *parent);
    void resetScreen();

    void startConfiguration();
    void updateTime();
    void Tabview();

private:
    lv_obj_t* arcbox;
    lv_obj_t* timebox;
    lv_obj_t* overlay;
    lv_obj_t* tabview;
    bool selectDuration = false;
    struct tm timeinfo;
    
};

#endif 