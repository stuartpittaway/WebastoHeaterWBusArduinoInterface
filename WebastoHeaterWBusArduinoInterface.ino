/*
  WebastoHeaterWBusArduinoInterface

   Copyright 2015 Stuart Pittaway

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


   Original idea and work by Manuel Jander  mjander@users.sourceforge.net
   https://sourceforge.net/projects/libwbus/


  20150422 Compile size 31430 bytes.  1314 bytes RAM used
  20150427 Removed some of the version information screens, Arduino 1.6.1, code size= 31086 bytes + 1274 bytes RAM.
*/

// __TIME__ __DATE__

#include <Arduino.h>

#include "utility.h"

#include <Time.h>
//#include <TimeAlarms.h>

#include <Wire.h>
#include <DS1307RTC.h>

#include "constants.h"
#include "wbus.h"

//Only for u8glib
#include <U8glib.h>
#include <M2tk.h>
#include "utility/m2ghu8g.h"

#define uiKeySelectPin  2
#define uiKeyNextPin  3
#define uiKeyPrevPin 4

//The famous Nokia 5110 display, driven by a PCD8544 chip.  Resolution 84x48 pixels.
//u8g_dev_pcd8544_84x48_hw_spi
//U8GLIB_PCD8544(cs, a0 [, reset])
U8GLIB_PCD8544 u8g(10, 9 , 8);

//Massive (!) buffer for scrolling text/string output
char value[190];

//buffer for text output of date/time
char textline1[15];
char textline2[15];

uint8_t dt_day = 1;
uint8_t dt_month = 1;
uint8_t dt_year = 12;
uint8_t ti_hour = 1;
uint8_t ti_mins = 1;

uint8_t total_lines1 = 0;
uint8_t first_visible_line1 = 0;
uint8_t currentErrorOnScreen = 0;
uint8_t updateDisplayCounter = 0;
uint8_t total_menu_items = 6;

unsigned char ErrorList[32];

unsigned long previousMillis;
unsigned long currentMillis;

bool updatedisplay;
uint8_t footerToggle = 0;
wb_sensor_t KeepAlive_wb_sensors;

unsigned long time_since_key_press = 0;

//Display 84x48 pixels

M2_SPACE(el_menu_space, "w1h2");

/* DISPLAY FOR INFO PAGES */
M2_INFO(el_labelptr, "w76l5" , &first_visible_line1, &total_lines1, value, fn_button_showhome);
M2_VSB(el_vsb, "w4l5" , &first_visible_line1, &total_lines1);
M2_LIST(list_strlist) = { &el_labelptr, &el_vsb };
M2_HLIST(el_strlist_hlist, NULL, list_strlist);
M2_ALIGN(el_infopages_root, "w84w48", &el_strlist_hlist);

/* FAULT DISPLAY*/
M2_INFO(el_labelptr2, "w76l5" , &first_visible_line1, &total_lines1, value, fn_shownextfault);
M2_LIST(list_strlist2) = { &el_labelptr2, &el_vsb };
M2_HLIST(el_strlist_hlist2, NULL, list_strlist2);
M2_ALIGN(top_nextfaulttopelement, "w84h48", &el_strlist_hlist2);

/* CLOCK/IDLE page */
M2_LABELFN(el_big_label_clock, "f8w64", label_clock);
M2_LABELFN(el_big_label_date, "w64f8", label_date);
M2_BUTTON(el_button_clock_close, "w64f8", "Menu", fn_button_showhome);
M2_LIST(list_clocklist) = {&el_big_label_clock, &el_menu_space, &el_menu_space, &el_big_label_date, &el_menu_space, &el_menu_space, &el_button_clock_close };
M2_VLIST(vlist_clocklist, NULL, list_clocklist);
M2_ALIGN(el_bigclock, NULL, &vlist_clocklist);

/* MAIN MENU */
M2_STRLIST(el_strlist_menu, "l4w76" , &first_visible_line1, &total_menu_items, el_strlist_getstr);
M2_VSB(el_vsb_menu, "l4w4" , &first_visible_line1, &total_menu_items);
M2_LIST(list_menu_strlist) = { &el_strlist_menu, &el_vsb_menu };
M2_HLIST(el_menu_hlist, NULL, list_menu_strlist);

//Footer at bottom of screen
M2_LABELFN(el_footer_label, NULL, label_footer);
M2_LABELFN(el_footer_label2, NULL, label_footer2);
M2_LIST(list_menu2) = { &el_menu_hlist, &el_menu_space, &el_footer_label, &el_footer_label2 };
M2_VLIST(el_menu_vlist, NULL, list_menu2);

M2_ALIGN(top_buttonmenu, "-0|2w84h48", &el_menu_vlist);

/* DIALOG */
M2_LABELFN(el_dialog_label, NULL, fn_value);
M2_BUTTON(el_button_close_dialog, NULL, "Close", fn_button_showhome);
M2_LIST(list_dialog) = { &el_dialog_label, &el_button_close_dialog };
M2_VLIST(el_dialog_vlist, NULL, list_dialog);
M2_ALIGN(top_dialog, NULL, &el_dialog_vlist);

/* SET DATE */


//= Set date menu =
M2_U8NUM(el_dt_day, "c2", 1, 31, &dt_day);
M2_LABEL(el_dt_sep1, NULL, "/");
M2_U8NUM(el_dt_month, "c2", 1, 12, &dt_month);
M2_LABEL(el_dt_sep2, NULL, "/20");
M2_U8NUM(el_dt_year, "c2", 15, 99, &dt_year);

M2_LIST(list_date) = { &el_dt_day, &el_dt_sep1, &el_dt_month, &el_dt_sep2, &el_dt_year };
M2_HLIST(el_date, NULL, list_date);

M2_BUTTON(el_dt_ok, NULL, "OK", fn_button_showhome);
M2_LIST(list_dt_buttons) = { &el_dt_ok };
M2_HLIST(el_dt_buttons, NULL, list_dt_buttons);

M2_LIST(list_dt) = {&el_date, &el_dt_buttons };
M2_VLIST(el_top_dt2, NULL, list_dt);
M2_ALIGN(el_top_dt, NULL, &el_top_dt2);



//= Set time menu =
M2_U8NUM(el_ti_hour, "c2", 0, 23, &ti_hour);
M2_LABEL(el_ti_sep1, NULL, ":");
M2_U8NUM(el_ti_mins, "c2", 0, 59, &ti_mins);

M2_LIST(list_time) = { &el_ti_hour, &el_ti_sep1, &el_ti_mins};
M2_HLIST(el_time, NULL, list_time);

M2_BUTTON(el_ti_ok, NULL, "OK", fn_button_showhome);
M2_LIST(list_ti_buttons) = {&el_ti_ok };
M2_HLIST(el_ti_buttons, NULL, list_ti_buttons);

M2_LIST(list_ti) = {&el_time, &el_ti_buttons };
M2_VLIST(el_top_time2, NULL, list_ti);
M2_ALIGN(el_top_time, NULL, &el_top_time2);


//U8
//M2tk m2(&top_buttonmenu, m2_es_arduino_rotary_encoder , m2_eh_4bd, m2_gh_u8g_bf);
M2tk m2(&el_bigclock, m2_es_arduino_rotary_encoder , m2_eh_4bd, m2_gh_u8g_bfs);

const char *label_date(m2_rom_void_p element)
{
  return textline2;
}
const char *label_clock(m2_rom_void_p element)
{
  return textline1;
}
const char *label_footer(m2_rom_void_p element)
{
  return textline1;
}
const char *label_footer2(m2_rom_void_p element)
{
  return textline2;
}
const char *fn_value(m2_rom_void_p element)
{
  return value;
}

/*
char* getVersion(char *str, unsigned char *d)
{
  strcpy_P(str, (char*)pgm_read_word(&(weekdays_table[d[0]])));
  str += strlen(str);
  str = PrintHexByte(str, d[1]);
  str[0] = '/'; str++;
  str = PrintHexByte(str, d[2]);
  str[0] = ' '; str++;
  str = PrintHexByte(str, d[3]);
  str[0] = '.'; str++;
  str = PrintHexByte(str, d[4]);
  //str += sprintf_P(str, label_getVersion, d[1], d[2], d[3], d[4]);
  return str;
}
*/

/*
void build_versioninfo_text()
{
  //904 bytes used by this routine

  char* v = value;
  wb_version_info_t wb_info;

  int err = wbus_get_version_wbinfo(&wb_info);

  if (!err) {
    unsigned char b;

    strcat_P(v, label_WBUSVer); v += strlen_P(label_WBUSVer);
    b = (wb_info.wbus_ver >> 4) & 0x0f;
    v = i2str(b, v);
    v++[0] = '.';
    b = (wb_info.wbus_ver & 0x0f);
    v = i2str(b, v);

    strcat_P(v, label_WBusCode); v += strlen_P(label_WBusCode);
    v = hexdump(v, wb_info.wbus_code, 7, true);

    strcat_P(v, label_HWVersion); v += strlen_P(label_HWVersion);
    v = hexdump(v, wb_info.hw_ver, 2, true);

    strcat_P(v, label_SoftwareVersion); v += strlen_P(label_SoftwareVersion);
    v = getVersion(v, wb_info.sw_ver);

    strcat_P(v, label_SoftwareVersionEEPROM); v += strlen_P(label_SoftwareVersionEEPROM);
    v = getVersion(v, wb_info.sw_ver_eeprom);

    strcat_P(v, label_DatasetIDNo); v += strlen_P(label_DatasetIDNo);
    v = hexdump(v, wb_info.data_set_id, 4, false);
    v++[0] = wb_info.data_set_id[4];
    v = hexdump(v, &wb_info.data_set_id[5], 1, true);

    strcat_P(v, label_SoftwareIDNo); v += strlen_P(label_SoftwareIDNo);
    v = hexdump(v, wb_info.sw_id, 5, false);

    //Output how many bytes used of buffer
    //v += sprintf(v, "%i", v - value);
  } else {
    v = ShowError(v, err);
  }
  //Ensure null terminate on string
  //v[0] = 0;
}
*/

void build_info_text_basic()
{
  char* v = value;
  wb_basic_info_t wb_info;

  int err = wbus_get_basic_info(&wb_info);
  if (!err) {
    strcat_P(v, label_DeviceName); v += strlen_P(label_DeviceName);
    strcat(v, wb_info.dev_name); v += strlen(wb_info.dev_name);

    strcat_P(v, label_DeviceIDNo); v += strlen_P(label_DeviceIDNo);
    v = hexdump(v, wb_info.dev_id, 4, false);
    v++[0] = wb_info.dev_id[4];

    strcat_P(v, label_DateOfManufactureControlUnit); v += strlen_P(label_DateOfManufactureControlUnit);
    v = PrintHexByte(v, wb_info.dom_cu[0]);
    v++[0] = '.';
    v = PrintHexByte(v, wb_info.dom_cu[1]);
    v++[0] = '.';
    v = i2str(2000 + wb_info.dom_cu[2], v);

    strcat_P(v, label_DateOfManufactureHeater); v += strlen_P(label_DateOfManufactureHeater);
    v = PrintHexByte(v, wb_info.dom_ht[0]);
    v++[0] = '.';
    v = PrintHexByte(v, wb_info.dom_ht[1]);
    v++[0] = '.';
    v = i2str(2000 + wb_info.dom_ht[2], v);

    strcat_P(v, label_CustomerIdentificationNumber); v += strlen_P(label_CustomerIdentificationNumber);
    strcat(v, (const char*)wb_info.customer_id);
    v += strlen((const char*)wb_info.customer_id);

    strcat_P(v, label_SerialNumber); v += strlen_P(label_SerialNumber);
    v = hexdump(v, wb_info.serial, 5, false);

    //Debug
    //v += sprintf(v, "%i", v - value);
    //Ensure null terminate on string
  } else {
    v = ShowError(v, err);
  }
  //v[0] = 0;
}


void home_menu() {
  first_visible_line1 = 0;
  m2.setRoot(&top_buttonmenu);
}

void fn_button_showhome(m2_el_fnarg_p fnarg) {
  //Close clock and show top menu
  home_menu();
}


char* faultHeaders(char* v) {
  strcat_P(v, label_ErrorCount); v += strlen_P(label_ErrorCount);
  v = i2str(currentErrorOnScreen + 1, v);
  v++[0] = '/';
  v = i2str(ErrorList[0], v);
  v++[0] = '\n';
  return v;
}

void fn_faults() {
  clearBuffer();
  char* v = value;
  int err;

  //Ensure empty array to start with
  memset(ErrorList, 0, sizeof(ErrorList));

  err = wbus_get_fault_count(ErrorList);

  if (!err) {

    if (ErrorList[0] > 0) {
      currentErrorOnScreen = 0;
      v = faultHeaders(v);
      v = PopulateTextForFault(currentErrorOnScreen, v);
    } else {
      strcat_P(v, label_NoFaultsFound); v += strlen_P(label_NoFaultsFound);
    }

  } else {
    //WBUS error
    v = ShowError(v, err);
  }
  //Ensure null terminate on string
  v[0] = 0;

  if (ErrorList[0] > 0) {
    m2.setRoot(&top_nextfaulttopelement);
  } else
  {
    //No more errors
    m2.setRoot(&top_dialog);
  }
}


void fn_clear_faults() {
  clearBuffer();
  char* v = value;

  int err = wbus_clear_faults();

  if (!err) {
    strcpy_P(v, label_FaultsCleared);
  } else {
    //WBUS error
    v = ShowError(v, err);
  }

  m2.setRoot(&top_dialog);
}


void fn_shownextfault(m2_el_fnarg_p fnarg) {
  /*
  This is called to build up the next fault information for display
  */
  clearBuffer();
  char* v = value;
  int err;

  //Show the next fault
  currentErrorOnScreen++;

  if ((currentErrorOnScreen + 1) < ErrorList[0]) {
    v = faultHeaders(v);
    v = PopulateTextForFault(currentErrorOnScreen, v);

    m2.setRoot(&top_nextfaulttopelement);
  } else {
    strcat_P(v, label_NoMoreFaultsFound); v += strlen_P(label_NoMoreFaultsFound);
    m2.setRoot(&top_dialog);
  }

  //Ensure null terminate on string
  v[0] = 0;
}

void clearBuffer() {
  memset(value, 0, sizeof(value));
}

char* ShowError(char* v, uint8_t errCode) {
  clearBuffer();
  v = value;
  strcat_P(v, label_wbusError); v += strlen_P(label_wbusError);
  v = PrintHexByte(v, errCode);
  return v;
}

char* PopulateTextForFault(uint8_t errIndex, char* v) {
  err_info_t err_info;
  int err;

  /*
  Fault 1
  Metering pump interruption 0x88
  Code 88h
  State: stored, not actual  =0x1
  Counter 3
  Temp: 22oC
  Operating state: Off state  4,0
  Voltage: 12.55V
  Operating hour counter: 5590:15 (h:m)
  */

  unsigned char errCode = ErrorList[errIndex * 2 + 1];

  err = wbus_get_fault(errCode, &err_info);
  if (!err) {

    //v = hexdump(v, &err_info.code, sizeof(err_info), false);

    // 88
    strcat_P(v, label_Code); v += strlen_P(label_Code);
    v = PrintHexByte(v, err_info.code);

    //01
    strcat_P(v, label_Flag); v += strlen_P(label_Flag);
    v = PrintHexByte(v, err_info.flags);

    //03
    strcat_P(v, label_Counter); v += strlen_P(label_Counter);
    v = i2str(err_info.counter, v);

    //04 00
    strcat_P(v, label_OperatingState); v += strlen_P(label_OperatingState);
    v = PrintHexByte(v, err_info.op_state[0]);
    v++[0] = '/';
    v = PrintHexByte(v, err_info.op_state[1]);
    v++[0] = '\n';

    //48
    strcat_P(v, label_Temperature); v += strlen_P(label_Temperature);
    v = i2str(BYTE2TEMP(err_info.temp), v);

    //31 06
    strcat_P(v, label_SupplyVoltage); v += strlen_P(label_SupplyVoltage);
    //v += WORD2VOLT_TEXT(v, err_info.volt);
    v = i2str( twobyte2word(err_info.volt), v);

    //5586:31 (h:m) = 15 d2 1f
    strcat_P(v, label_OperatingTime); v += strlen_P(label_OperatingTime);

    v = i2str( WORD2HOUR(err_info.hour), v);
    v++[0] = 'h';
    v = i2str( err_info.minute, v);
    v++[0] = 'm';
  } else {
    v = ShowError(v, err);
  }

  return v;
}

unsigned long getTimeFunction() {
  tmElements_t tm;

  if (RTC.read(tm)) {
    return makeTime(tm);
  }

  //Error so just return what we already have
  else return now();
}


void updateClockString() {
  if (timeStatus() != timeNotSet) {
    tmElements_t tm;
    breakTime(now(), tm);


    char* v = textline1;
    v = i2str_zeropad( tm.Hour, v);
    v++[0] = ':';
    v = i2str_zeropad( tm.Minute, v);
    //v++[0] = '.';
    //v = i2str( tm.Second, v);
    v++[0] = 0;

    /* Now the date */
    v = textline2;
    if (tm.Wday == 1) {
      strcat_P(v, label_Sun);
    } else {
      strcpy_P(v, (char*)pgm_read_word(&(weekdays_table[tm.Wday])));
    }
    v += 4;

    v = i2str_zeropad( tm.Day, v);
    v++[0] = '-';
    v = i2str_zeropad( tm.Month, v);
    v++[0] = '-';
    v = i2str_zeropad( 1970 + tm.Year - 2000, v);

    //Terminator
    v++[0] = 0;
  }
}

void setup() {
  getTimeFunction();
  setSyncProvider(getTimeFunction);
  setSyncInterval(600);  //10 minutes

  //  Alarm.timerRepeat(1, Repeats);            // timer for every 15 seconds

  // put your setup code here, to run once:
  /* connect u8glib with m2tklib */
  m2_SetU8g(u8g.getU8g(), m2_u8g_box_icon);

  //m2_SetU8gAdditionalTextXBorder(2);
  //m2_SetU8gAdditionalReadOnlyXBorder(2);

  /* assign u8g font to index 0 */

  //m2.setFont(0,  u8g_font_4x6r);

  //6 pixel fonts are best for small screen
  //m2.setFont(0,  u8g_font_5x7r);
  m2.setFont(0,  u8g_font_5x8r);  //31208

  //7 pixel font
  //m2.setFont(0, u8g_font_lucasfont_alternater);  //332 bytes larger than u8g_font_5x8r

  //Large clock font - numbers only
  //m2.setFont(1, u8g_font_helvB14n);

  //m2_SetU8gToggleFontIcon(u8g_font_m2icon_9, active_encoding, inactive_encoding);
  //m2_SetU8gRadioFontIcon(u8g_font_m2icon_9, active_encoding, inactive_encoding);

  m2.setPin(M2_KEY_SELECT, uiKeySelectPin);
  //m2.setPin(M2_KEY_NEXT, uiKeyNextPin);
  //m2.setPin(M2_KEY_PREV, uiKeyPrevPin);

  m2.setPin(M2_KEY_ROT_ENC_A, uiKeyNextPin);
  m2.setPin(M2_KEY_ROT_ENC_B, uiKeyPrevPin);

  wbus_init();

  //Return to top home menu
  //m2.setRoot(&top_buttonmenu);

  m2.setRoot(&el_bigclock);

}

const char *el_strlist_getstr(uint8_t idx, uint8_t msg) {
  char *v = value;

  switch (idx)
  {
    case 0:
      strcpy_P(v, label_menu_shheaton);
      //v += strlen_P(label_menu_shheaton);
      break;
    case 1 :
      strcpy_P(v, label_menu_heaterinfo);
      //v += strlen_P(label_menu_heaterinfo);
      break;
    //case 2:
      //strcpy_P(v, label_menu_versioninfo);
      //v += strlen_P(label_menu_versioninfo);
      //break;
    case 2:
      strcpy_P(v, label_menu_showfaults);
      //v += strlen_P(label_menu_showfaults);
      break;
    case 3:
      strcpy_P(v, label_menu_clearfaults);
      //v += strlen_P(label_menu_clearfaults);
      break;
    case 4:
      strcpy_P(v, label_menu_settime);
      break;
    case 5:
      strcpy_P(v, label_menu_setdate);
      break;

    default:
      //Null terminate string, just in case
      v[0] = 0;
  };

  if ( msg == M2_STRLIST_MSG_SELECT ) {
    clearBuffer();

    switch (idx)
    {
      case 0:
        //Switch Supplimental heater ON
        strcpy_P(v, label_shheaterswitchedon);
        m2.setRoot(&top_dialog);
        break;
      case 1 :
        //Basic heater info
        build_info_text_basic();
        m2.setRoot(&el_infopages_root);
        break;
//      case 2:
//        //label_menu_versioninfo
//        build_versioninfo_text();
//        m2.setRoot(&el_infopages_root);
//        break;
      case 2:
        //label_menu_showfaults
        fn_faults();
        break;
      case 3:
        //label_menu_clearfaults
        fn_clear_faults();
        break;
      case 4:
        fn_settime();
        break;
      case 5:
        fn_setdate();
        break;
    }

  }

  return value;
}


void fn_settime() {

  tmElements_t tm;
  breakTime(getTimeFunction(), tm);

  ti_hour = tm.Hour;
  ti_mins = tm.Minute;

  m2.setRoot(&el_top_time);
}

void fn_setdate() {
  tmElements_t tm;
  breakTime(getTimeFunction(), tm);

  dt_day = tm.Day;
  dt_month = tm.Month;
  dt_year = 1970 + tm.Year - 2000;

  m2.setRoot(&el_top_dt);
}

void keepAlive() {
  //The heater units controller appears to "sleep" after a few seconds of inactivity
  //so keep pestering it to ensure it keeps awake.
  currentMillis = millis();

  //Only need to communicate every few seconds
  if ((currentMillis - previousMillis) > 1700) {
    previousMillis = currentMillis;

    //Query heater every few seconds to keep it alive
    int err;
    if (footerToggle == 0) {
      err = wbus_sensor_read(&KeepAlive_wb_sensors, QUERY_STATE);
    } else {
      err = wbus_sensor_read(&KeepAlive_wb_sensors, QUERY_SENSORS);
    }

    if (updateDisplayCounter == 0 || err != 0) {
      //Force menu system to redraw to show our changes
      //Use updatedisplay instead so we can timeout if needed
      //m2.setKey(M2_KEY_REFRESH);

      updatedisplay = true;

      //Clear text strings
      memset(textline1, 0, sizeof(textline1));
      memset(textline2, 0, sizeof(textline2));

      //Update every 5 refreshes (about 8 seconds)
      updateDisplayCounter = 4 + 1;

      char* v = textline1;

      if (err != 0) {
        //What to do with errors?
        //Have to use textline2 as we dont have full font for clock display
        v = textline2;
        strcat_P(v, label_wbusError);
        v += strlen_P(label_wbusError);
        v = PrintHexByte(v, err);
      } else {

        if (m2.getRoot() == &el_bigclock) {
          //Its the idle clock menu thats visible
          updateClockString();
        } else if (m2.getRoot() == &top_buttonmenu) {
          //Must be the menu, switch text in a cycle

          switch (footerToggle) {
            case 0:
              //QUERY_STATE results
              strcat_P(v, label_OP); v += strlen_P(label_OP);
              v = PrintHexByte(v, KeepAlive_wb_sensors.value[OP_STATE]);
              strcat_P(v, label_N); v += strlen_P(label_N);
              v = i2str( KeepAlive_wb_sensors.value[OP_STATE_N], v);
              //v[0] = 0;

              v = textline2;

              strcat_P(v, label_Dev); v += strlen_P(label_Dev);
              v = PrintHexByte(v, KeepAlive_wb_sensors.value[DEV_STATE]);
              //v++[0] = 0;

              break;

            case 1:
              //QUERY_SENSORS results
              //Temperature
              v[0] = 'T'; v++;
              v = i2str( BYTE2TEMP(KeepAlive_wb_sensors.value[SEN_TEMP]), v);

              //SupplyVoltage
              v[0] = ' '; v++;
              v[0] = 'V'; v++;
              v = i2str( twobyte2word(&KeepAlive_wb_sensors.value[SEN_VOLT]), v);
              //v[0] = 0;

              v = textline2;

              //Flame detect
              v[0] = 'F'; v++;
              v = i2str( KeepAlive_wb_sensors.value[SEN_FD], v);

              //Heat energy
              v[0] = ' '; v++;
              v[0] = 'H'; v++;
              v = i2str( twobyte2word(&KeepAlive_wb_sensors.value[SEN_HE]), v);

              //glow plug resistance in mili Ohm
              v[0] = ' '; v++;
              v[0] = 'G'; v++;
              v = i2str( KeepAlive_wb_sensors.value[SEN_GPR], v);
              break;

            case 2:
              updateClockString();
              footerToggle = -1;
              break;
          }

          footerToggle++;
        }
      }

    }
    updateDisplayCounter--;
  }
}


void loop() {
  keepAlive();

  // menu management
  m2.checkKey();
  m2.checkKey();

  //Redraw screen if key press event or updatedisplay flag is true
  if (m2.handleKey() | updatedisplay) {

    if (!updatedisplay) {
      //Key was pressed so reset clock timer to now+45 seconds
      time_since_key_press = millis() + 45000;
    }

    updatedisplay = false;

    u8g.firstPage();
    do {
      m2.draw();
    } while ( u8g.nextPage() );
  }

  //Must be a way to save us 4 bytes instead of using an unsigned long for time_since_key_press
  if (millis() > time_since_key_press) {
    m2.setRoot(&el_bigclock);
  }

}
