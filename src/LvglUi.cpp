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
bool wifiOn = false;
bool selectDuration = false;
bool statusSelectDuration = false;
int timerArcMin =5;

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
    lv_display_set_flush_cb(disp,  LvglUi::my_disp_flush);
    lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // Create input device (touchpad of the JC4827W543)
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev,  LvglUi::my_touchpad_read);

  }
  
  TimeBox();
  Arc();

  InfoBox();
  ActionBox();
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
lv_obj_t* arc;
void LvglUi::timer_Cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  auto* ui = static_cast<LvglUi*>(lv_event_get_user_data(e));
 
  if (code == LV_EVENT_CLICKED) {
     Serial.println("hey");
      // Serial.println("Button wurde geklickt!");
      // wecker.startTimer(1);
      Serial.println(selectDuration);
      selectDuration = !selectDuration;

      if(selectDuration){
        lv_obj_clear_flag(ui->arcbox, LV_OBJ_FLAG_HIDDEN);  // Zeigt das Element
        lv_obj_add_flag(ui->timebox, LV_OBJ_FLAG_HIDDEN);    // Versteckt das andere
    } 
      else {
          lv_obj_clear_flag(ui->timebox, LV_OBJ_FLAG_HIDDEN);
          lv_obj_add_flag(ui->arcbox, LV_OBJ_FLAG_HIDDEN);
          wecker.startTimer(timerArcMin);
          }
        statusSelectDuration = selectDuration;



  }
}
void LvglUi::value_changed_event_cb(lv_event_t * e)
{

    lv_obj_t * arc = lv_event_get_target_obj(e);
    Serial.println(lv_arc_get_value(arc));
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);

    lv_label_set_text_fmt(label, "%" LV_PRId32 "Min", lv_arc_get_value(arc));
    timerArcMin =lv_arc_get_value(arc);
    /*Rotate the label to the current position of the arc*/
    lv_arc_rotate_obj_to_angle(arc, label, 25);
}
void LvglUi::TimeBox(){
      // Erstelle eine Box (Container für die Uhrzeit)
      timebox = lv_obj_create(lv_screen_active());  // Box auf dem aktuellen Bildschirm erstellen
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
void LvglUi::Arc(){

        arcbox = lv_obj_create(lv_screen_active());  // Box auf dem aktuellen Bildschirm erstellen
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


void LvglUi::InfoBox(){
  lv_obj_t * infobox = lv_obj_create(lv_screen_active());
  lv_obj_set_size(infobox, 160, 112);
  lv_obj_align(infobox,LV_ALIGN_BOTTOM_LEFT,0,0);
  lv_obj_set_style_bg_color(infobox, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
  
  date_label = lv_label_create(infobox);
  lv_obj_align(date_label, LV_ALIGN_CENTER,0,0);
  wheater_label = lv_label_create(infobox);
  lv_obj_align(wheater_label, LV_ALIGN_TOP_LEFT,0,0);
  wifi_label = lv_label_create(infobox);
  lv_obj_align(wifi_label, LV_ALIGN_TOP_RIGHT,0,0);
}

void LvglUi::ActionBox(){
  lv_obj_t * actionbox = lv_obj_create(lv_screen_active());
  lv_obj_set_size(actionbox,320,272);
  lv_obj_align(actionbox,LV_ALIGN_RIGHT_MID,0,0);
  lv_obj_set_style_bg_color(actionbox, lv_color_hex(0x808080), 0); // Box Hintergrundfarbe
  

  action_label= lv_label_create(actionbox);
  lv_obj_align(action_label,LV_ALIGN_CENTER,0,0);
  lv_label_set_text(action_label,"Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor.");
  lv_obj_set_width(action_label, 300); // Breite begrenzen, damit Umbruch stattfinden kann

}

// Sowohl Zeit wie auch Datum und Wetter aktualisieren
void LvglUi::updateTime(){
  struct tm timeinfo;
  // einbauen einer alle 15 min funktion wie main.ino
  char wheater_str[20];
  snprintf(wheater_str,sizeof(wheater_str), "%.2f°C", Temperatur);

  
  lv_label_set_text(wheater_label,wheater_str);
  if(WiFi.status() == WL_CONNECTED && wifiOn != true){
    wifiOn = true;
    lv_label_set_text(wifi_label,"WiFi");
  }
  else if(WiFi.status() != WL_CONNECTED && wifiOn != false){
    wifiOn = false;
    lv_label_set_text(wifi_label,"Nope");
  }
  // zeitrechnung in min.sek und nicht sek.millis
  if(timerOn){
      lv_label_set_text(timer_label, timerDauer.c_str());
      Serial.println(timerDauer.c_str());
  }
  else if (timerOn == false && timerDauer != "#"){
    timerDauer = "#" ;
    lv_label_set_text(timer_label,"");
  }
  if(getLocalTime(&timeinfo) && currentMin != lastMin ){
    char time_str[9];  // Puffer für die Uhrzeit im Format HH:MM:SS
    char date_str[12]; 
    lastMin = currentMin;
    strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);  // Uhrzeit formatieren

    strftime(date_str, sizeof(date_str), "%d.%m.%Y", &timeinfo);  // Uhrzeit formatieren

    // Aktualisiere das Label mit der aktuellen Uhrzeit
    lv_label_set_text(time_label, time_str);
    lv_label_set_text(date_label, date_str  );
  }
  else if(lastMin == 62) {
    lastMin = 61;
    lv_label_set_text(time_label, "--:--");  // Falls die Uhrzeit nicht abgerufen werden konnte
    lv_label_set_text(date_label, "--.--.----");
  }
}

