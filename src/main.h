#include <Arduino.h>

#include "m2tk.h"
#include "utility\m2ghu8g.h"

#include <Time.h>
#include "constants.h"
#include "wbus.h"


#define uiKeySelectPin  14  //A0
#define uiKeyNextPin  15  //A1
#define uiKeyPrevPin 16  //A2

#define lcdBacklight 5

//buffer for text output of date/time
char textline1[17];
char textline2[17];

//Massive (!) buffer for scrolling text/string output
char value[190];

uint8_t total_lines1 = 0;
uint8_t first_visible_line1 = 0;
uint8_t updateDisplayCounter = 0;
uint8_t total_menu_items = 8;

uint8_t timerEnabled;
uint8_t timerHour;
uint8_t timerMinute;

unsigned long previousMillis;
unsigned long currentMillis;
unsigned long time_since_key_press = 0;
unsigned char heaterMode = WBUS_CMD_OFF;

char clocktext[17];

bool updatedisplay;
uint8_t footerToggle = 0;
wb_sensor_t KeepAlive_wb_sensors;
tmElements_t tm;

/* function definitions */
unsigned long getTimeFunction();
const char *label_clocktext(m2_rom_void_p element);
const char *el_strlist_mainmenu(uint8_t idx, uint8_t msg);
const char *label_footer(m2_rom_void_p element);
const char *label_footer2(m2_rom_void_p element);
void home_menu();
char* ShowError(uint8_t errCode);
void clearBuffer();
void fn_begin_set_date_time();
void updateClockString();
bool heaterOn();
void fn_begin_set_timer();
void keepAlive();
void backlightOn();
void backlightOff();
