#include "LvglUi.h"
#include "PINS_JC4827W543.h"
extern float Temperatur;
// Display global variables
uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_display_t *disp;
lv_color_t *disp_draw_buf;
extern TAMC_GT911 touchController;
extern bool uhrGesetzt;
extern unsigned long lastWheater;
const unsigned long wheaterInterval = 15 * 60 * 1000UL; // 15 Minuten
bool WetterGesetzt = false;
#define LV_COLOR_DEPTH 16
LvglUi::LvglUi() {
    // Konstruktor-Inhalt
}
  
uint32_t LvglUi::millis_cb(void)
{
  return millis();
}
extern String timerDauer;
extern bool timerOn;
extern int currentMin;
extern int lastMin;
bool wifiOn;
int timerArcMin =5;
bool statusSelectDuration = false;
extern String localIp;

// LVGL calls this function when a rendered image needs to copied to the display
void LvglUi::my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    // Draw auf dem Display
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

    lv_disp_flush_ready(disp);
  }

// LVGL calls this function to read the touchpad
void LvglUi::my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  // Update the touch data from the GT911 touch controller
  touchController.read();

  // If a touch is detected, update the LVGL data structure with the first point's coordinates.
  if (touchController.isTouched && touchController.touches > 0)
  {
    data->point.x = touchController.points[0].x;
    data->point.y = touchController.points[0].y;
    data->state = LV_INDEV_STATE_PRESSED; // Touch is pressed
    Serial.println(touchController.points[0].x);
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED; // No touch detected
  }
}


void LvglUi::startLVGL() {
      //Einstellen des Monitors
  #ifdef GFX_BL
  // Set the backlight of the screen to High intensity
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif
if(gfx){
  if (!gfx->begin()) {
    Serial.println("gfx->begin() fehlgeschlagen!");
  } else {
    Serial.println("gfx erfolgreich initialisiert");
  }
}
else{
  Serial.println("GFX ist bnu");
}
touchController.begin();
touchController.setRotation(ROTATION_INVERTED); // Change as needed
    lv_init();
  
    // Set a tick source so that LVGL will know how much time elapsed
    lv_tick_set_cb(millis_cb);
     // register print function for debugging
  #if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
  #endif
  screenHeight = 272;
  screenWidth = 480;
  bufSize = screenWidth * 40;
  
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_draw_buf) {
      disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_8BIT);
  }
  if (!disp_draw_buf)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    
    disp = lv_display_create(screenWidth, screenHeight);
    // lv_disp_set_rotation(NULL,LV_DISP_ROTATION_180);
    lv_display_set_flush_cb(disp,  LvglUi::my_disp_flush);
    lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // Create input device (touchpad of the JC4827W543)
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev,  LvglUi::my_touchpad_read);

  }
  prefs.begin("settings",false);
  bool configured = prefs.getBool("Configured",false);
  if(configured == false){
    startConfiguration();
  }
  else{
 Tabview();
  }
  

}    

lv_obj_t *infobox;
lv_obj_t *arcbox;
lv_obj_t *actionbox;
lv_obj_t *time_label;  
lv_obj_t *timer_label;
lv_obj_t *date_label;
lv_obj_t *wheater_label;
lv_obj_t *wifi_label;
lv_obj_t *action_label;
lv_obj_t *select_label;
lv_obj_t *credit_label;
lv_obj_t *reset_label;
lv_obj_t *start_label;
lv_obj_t *arc;
lv_obj_t *credits;

lv_obj_t *resetButton;
void LvglUi::timer_Cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  auto* ui = static_cast<LvglUi*>(lv_event_get_user_data(e));
 
if (code == LV_EVENT_CLICKED) {
    Serial.println("hey");

    ui->selectDuration = !ui->selectDuration;

    if (ui->selectDuration) {
        if (!timerOn) { 
            Serial.println("Tag");  
            lv_obj_clear_flag(ui->overlay, LV_OBJ_FLAG_HIDDEN);     
            lv_obj_clear_flag(ui->arcbox, LV_OBJ_FLAG_HIDDEN);  
            lv_obj_add_flag(ui->timebox, LV_OBJ_FLAG_HIDDEN);    
        }
    } else {  
        Serial.print("nacht");
        lv_obj_add_flag(ui->overlay, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->arcbox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui->timebox, LV_OBJ_FLAG_HIDDEN);
        player.playMP3("/button.mp3");
        wecker.startTimer(timerArcMin);
        Serial.println(timerArcMin);
    }

    statusSelectDuration = ui->selectDuration;
}
}

void LvglUi::value_changed_event_cb(lv_event_t * e)
{
    auto* ui = static_cast<LvglUi*>(lv_event_get_user_data(e));
    lv_obj_t * arc = lv_event_get_target_obj(e);
    Serial.println(lv_arc_get_value(arc));
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);

    lv_label_set_text_fmt(label, "%" LV_PRId32 "Min", lv_arc_get_value(arc));
    timerArcMin =lv_arc_get_value(arc);
    Serial.println("timer");
    Serial.println(timerArcMin);
    /*Rotate the label to the current position of the arc*/
    lv_arc_rotate_obj_to_angle(arc, label, 25);
}

void LvglUi::overlay_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    auto* ui = static_cast<LvglUi*>(lv_event_get_user_data(e));
    if (code == LV_EVENT_CLICKED) {
    Serial.println("overlay");
    auto* ui = static_cast<LvglUi*>(lv_event_get_user_data(e));
    lv_obj_clear_flag(ui->timebox, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->overlay,LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->arcbox, LV_OBJ_FLAG_HIDDEN);
    ui->selectDuration = false;
    }
}

void LvglUi::reset_Cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    auto* ui = static_cast<LvglUi*>(lv_event_get_user_data(e));
    if (code == LV_EVENT_CLICKED) {
      player.playMP3("discord.mp3");
      Serial.println("reset...");

      lv_obj_add_flag(ui->tabview,LV_OBJ_FLAG_HIDDEN);
      ui->resetScreen();
      lv_timer_handler(); /* let the GUI do its work */
      unsigned long start = millis();
      while (millis() - start < 5000) {
        player.updatePlaying();
        lv_timer_handler(); /* let the GUI do its work */
        delay(10);  // minimal blockieren, aber nicht komplett
      }
      prefs.begin("settings",false);
      prefs.putBool("Configured",false);
      prefs.putBool("Danke",false);
      prefs.putBool("Tutorial",false);
      prefs.end();
      ESP.restart();
    }
}
void LvglUi::soundSpecial(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
      player.playMP3("/error.mp3");
      delay(100);
    }
}
void LvglUi::TimeBox(lv_obj_t *parent){
      // Erstelle eine Box (Container für die Uhrzeit)
      timebox = lv_obj_create(parent);  // Box auf dem aktuellen Bildschirm erstellen
      lv_obj_add_event_cb(timebox, LvglUi::timer_Cb, LV_EVENT_ALL, this);
      lv_obj_set_size(timebox, 160,160);      // Boxgröße festlegen
      lv_obj_align(timebox, LV_ALIGN_TOP_LEFT, 0, 0);  // Box mittig ausrichten
      lv_obj_set_style_bg_color(timebox, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
      // lv_style_set_text_font(&style, LV_STATE_DEFAULT, &lv_font_montserrat_20); // Schriftgröße 20

      // Erstelle das Label für die Uhrzeit
      time_label = lv_label_create(timebox);    // Label innerhalb der Box erstellen
      lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);  // Label mittig in der Box ausrichten
      timer_label = lv_label_create(timebox);    // Label innerhalb der Box erstellen
      lv_obj_align(timer_label, LV_ALIGN_CENTER, 0, 20);  // Label mittig in der Box ausrichten
      Serial.println(selectDuration);

    }
void LvglUi::Arc(lv_obj_t *parent){

        arcbox = lv_obj_create(parent);  // Box auf dem aktuellen Bildschirm erstellen
        lv_obj_set_size(arcbox, 160,160);      // Boxgröße festlegen
        lv_obj_align(arcbox, LV_ALIGN_TOP_LEFT, 0, 0);  // Box mittig ausrichten
        lv_obj_set_style_bg_color(arcbox, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
    
        lv_obj_t * label = lv_label_create(arcbox);
        /*Create an Arc*/
        arc = lv_arc_create(arcbox);
        lv_obj_set_size(arc, 100, 100);
        lv_arc_set_rotation(arc, 135);
        lv_arc_set_bg_angles(arc, 0, 270);

        lv_arc_set_range(arc,1,60);
        lv_arc_set_value(arc, timerArcMin);
        lv_obj_center(arc);
        lv_obj_add_event_cb(arc, LvglUi::value_changed_event_cb, LV_EVENT_VALUE_CHANGED, label);

        lv_obj_align(arcbox, LV_ALIGN_TOP_LEFT, 0, 0);  // Box mittig ausrichten
      
        /*Manually update the label for the first time*/
        lv_obj_send_event(arc, LV_EVENT_VALUE_CHANGED, NULL);
        //   // Interaktivität sicherstellen
        // lv_obj_add_state(arc, LV_STATE_DEFAULT); // Aktivieren der interaktiven Standard-Eigenschaft
        select_label = lv_btn_create(arcbox);
        lv_obj_center(select_label);
        lv_obj_add_event_cb(select_label, LvglUi::timer_Cb, LV_EVENT_ALL, this);
        lv_obj_add_flag(arcbox, LV_OBJ_FLAG_HIDDEN);
      }


void LvglUi::InfoBox(lv_obj_t *parent){
  lv_obj_t * infobox = lv_obj_create(parent);
  lv_obj_set_size(infobox, 160, 112);
  lv_obj_align(infobox,LV_ALIGN_BOTTOM_LEFT,0,0);
  lv_obj_set_style_bg_color(infobox, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
  
  date_label = lv_label_create(infobox);
  lv_obj_align(date_label, LV_ALIGN_CENTER,0,0);
  wheater_label = lv_label_create(infobox);
  lv_obj_align(wheater_label, LV_ALIGN_TOP_LEFT,0,0);
  wifi_label = lv_label_create(infobox);
  lv_obj_align(wifi_label, LV_ALIGN_TOP_RIGHT,0,0);
  lv_obj_add_event_cb(infobox, LvglUi::soundSpecial, LV_EVENT_ALL, this);
}

void LvglUi::ActionBox(lv_obj_t *parent){
  lv_obj_t *actionbox = lv_obj_create(parent);
  lv_obj_set_size(actionbox,320,272);
  lv_obj_align(actionbox,LV_ALIGN_RIGHT_MID,0,0);
  lv_obj_set_style_bg_color(actionbox, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
  

  action_label= lv_label_create(actionbox);
  
  lv_label_set_text(action_label,"   " LV_SYMBOL_HOME "   Next Termin: Training 17:00\n" "   "LV_SYMBOL_EDIT "   To-Do: Lernen\n" );

  lv_obj_set_width(action_label, 300); // Breite begrenzen, damit Umbruch stattfinden kann
  lv_obj_align(action_label, LV_ALIGN_CENTER, 0, 0);  // Label mittig in der Box ausrichten

}
void LvglUi::Overlay(lv_obj_t *parent){
  overlay = lv_obj_create(parent);
  lv_obj_set_size(overlay, LV_HOR_RES, LV_VER_RES);
lv_obj_set_style_bg_opa(overlay, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);  // Keine Scrollbarkeit

  // Event hinzufügen, um bei Klick zu schließen
  lv_obj_add_event_cb(overlay, LvglUi::overlay_cb, LV_EVENT_ALL, this);
  lv_obj_add_flag(overlay,LV_OBJ_FLAG_HIDDEN);
}

void LvglUi::contentSidePage(lv_obj_t *parent){
  lv_obj_t *credits=lv_obj_create(parent);

    lv_obj_align(credits, LV_ALIGN_CENTER, 0, 0);  // Box mittig ausrichten
    lv_obj_set_style_bg_color(credits, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
    lv_obj_set_size(credits,250,200);
    credit_label = lv_label_create(credits);
    lv_obj_align(credit_label,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(credit_label,"Coded by Gianluca C. Rossi \n" 
    "LifeOrganisationStation\n"
    "Last Update: May 2025\n"
    "los.local" );
    lv_obj_t *resetButton = lv_button_create(parent);
    lv_obj_set_style_bg_color(resetButton, lv_color_hex(0xff0000), 0); // Box Hintergrundfarbe
    lv_obj_set_size(resetButton,50,50);
    // Positioniere den Button unterhalb der Box "credits"
    lv_obj_align_to(resetButton, credits, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // 10 Pixel Abstand nach unten

    reset_label = lv_label_create(resetButton);
    lv_label_set_text(reset_label,"Reset");
lv_obj_add_event_cb(resetButton, LvglUi::reset_Cb, LV_EVENT_ALL,this);
        
}
void LvglUi::resetScreen(){
  lv_obj_t *startScreen = lv_obj_create(lv_screen_active());
  lv_obj_align(startScreen,LV_ALIGN_CENTER,0,0);
  lv_obj_set_style_bg_color(startScreen, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
  lv_obj_set_size(startScreen,300,250);
  start_label = lv_label_create(startScreen);
  lv_obj_align(start_label,LV_ALIGN_CENTER,0,0);

  lv_label_set_text(start_label,"LOS wird neugestartet...\n"
  "Dies kann einige Zeit in Anspruch nehmen!\n" 
  "Bitte trennen sie das Gerät nicht vom Strom."
  );
}
void LvglUi::startConfiguration(){
  lv_obj_t *startScreen = lv_obj_create(lv_screen_active());
  lv_obj_align(startScreen,LV_ALIGN_CENTER,0,0);
  lv_obj_set_style_bg_color(startScreen, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
  lv_obj_set_size(startScreen,300,250);
  start_label = lv_label_create(startScreen);
  lv_obj_align(start_label,LV_ALIGN_CENTER,0,0);
  String startText = "Welcome to LOS! \n" 
  "Please connect to following WiFi:\n"
  "Name: " + WiFi.softAPSSID() + "\n" +
  "Passwort: LOSBGT11\n"
  "Then go in your Browser and search:\n"
  + WiFi.softAPIP().toString();
  lv_label_set_text(start_label,startText.c_str());
}
void LvglUi::Tabview(){
  tabview = lv_tabview_create(lv_screen_active());
    lv_obj_t *header = lv_tabview_get_tab_bar(tabview);
  lv_obj_add_flag(header,LV_OBJ_FLAG_HIDDEN);
  // lv_tabview_set_tab_bar_position(tabview, LV_DIR_NONE);

  lv_obj_t * mainPage = lv_tabview_add_tab(tabview,"MainPage");
  lv_obj_set_style_pad_all(mainPage, 0, 0);
  lv_obj_set_style_border_width(mainPage, 0, 0);
  lv_obj_set_style_radius(mainPage, 0, 0);
  TimeBox(mainPage);
  InfoBox(mainPage);
  ActionBox(mainPage);
   Overlay(mainPage);
  
 
  Arc(mainPage);

  lv_obj_t * sidePage = lv_tabview_add_tab(tabview,"sidePage");
  lv_obj_set_style_pad_all(sidePage, 0, 0);
  lv_obj_set_style_border_width(sidePage, 0, 0);
  lv_obj_set_style_radius(sidePage, 0, 0);
  contentSidePage(sidePage);
}
// Sowohl Zeit wie auch Datum und Wetter aktualisieren
void LvglUi::updateTime(){
    char wheater_str[20];
  // // einbauen einer alle 15 min funktion wie main.ino
    if(millis() - lastWheater >= wheaterInterval || uhrGesetzt && !WetterGesetzt){
      Serial.print("LVGL");
      Serial.println(Temperatur);
    WetterGesetzt = true;
    snprintf(wheater_str,sizeof(wheater_str), "%.2f°C", Temperatur);

    lv_label_set_text(wheater_label, wheater_str);
  }

  if(WiFi.status() == WL_CONNECTED && wifiOn != true){
    wifiOn = true;
    Serial.println("Wifi on");
    lv_label_set_text(wifi_label,"WiFi" LV_SYMBOL_WIFI);
  }
  else if((WiFi.status() != WL_CONNECTED && wifiOn != false)) {
    wifiOn = false;
    Serial.println("Wifi false");
    lv_label_set_text(wifi_label,LV_SYMBOL_WARNING);
  }
  // zeitrechnung in min.sek und nicht sek.millis
  if(timerOn){
      lv_label_set_text(timer_label, timerDauer.c_str());
      Serial.println(timerDauer.c_str());
      Serial.println("Timer on");
  }
  else if (timerOn == false && timerDauer != "#"){
    timerDauer = "#" ;
    lv_label_set_text(timer_label,"");
    Serial.println("Timer off");
  }
  if( uhrGesetzt){

  if(getLocalTime(&timeinfo) && currentMin != lastMin ){
    char time_str[9];  // Puffer für die Uhrzeit im Format HH:MM:SS
    char date_str[12]; 
    lastMin = currentMin;
    strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);  // Uhrzeit formatieren

    strftime(date_str, sizeof(date_str), "%d.%m.%Y", &timeinfo);  // Uhrzeit formatieren

    // Aktualisiere das Label mit der aktuellen Uhrzeit
    lv_label_set_text(time_label, time_str);
    lv_label_set_text(date_label, date_str  );
    Serial.println("get Clock");
  }}
  else if(lastMin == 62) {
    lastMin = 61;
    lv_label_set_text(time_label, "--:--");  // Falls die Uhrzeit nicht abgerufen werden konnte
    lv_label_set_text(date_label, "--.--.----");
    Serial.println("set clock default");
    if(WiFi.status()!=WL_CONNECTED){
      lv_label_set_text(wifi_label,LV_SYMBOL_WARNING);
      lv_label_set_text(wheater_label,"123C");
    }


  }
}

