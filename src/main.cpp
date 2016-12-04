
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
  20150429 Compile size Arduino 1.6.1, 31224 bytes, 1254 RAM.
  20150429 Compile size Arduino 1.6.1, 31160 bytes, 1246 RAM.
  20150429 Compile size Arduino 1.6.1, 30980 bytes, 1226 RAM.   Removed idle clock screen
  20150429 Compile size Arduino 1.6.1, 30926 bytes, 1220 RAM.   Removed el_menu_space
  20150429 Compile size Arduino 1.6.1, 30934 bytes, 1235 RAM.   Added perm. clock display on 1st line of LCD
  20150501 Compile size Arduino 1.6.1, 30784 bytes, 1235 RAM.
  20150501 Added copy_string function to reduce code size. Compile size Arduino 1.6.1, 30752 bytes, 1235 RAM.
  20150501 Compile size Arduino 1.6.1, 30718 bytes, 1233 RAM.
  20150501 Compile size Arduino 1.6.1, 30632 bytes, 1244 RAM. Change wbus to use shared buffer
  20150501 Compile size Arduino 1.6.1, 30632 bytes, 1244 RAM. Change wbus to use shared buffer
  20161204 Compile under Platform.IO with updated external libraries
  20161204 Compile size Platform.IO, 29,730 bytes

  Need to check out the IDENT_WB_CODE reply
*/

// __TIME__ __DATE__


#include <Arduino.h>
#include <EEPROM.h>
#include "utility.h"

#include <Wire.h>
#include <DS1307RTC.h>
#include <Time.h>

//Only for u8glib
#include <U8glib.h>

//The famous Nokia 5110 display, driven by a PCD8544 chip.  Resolution 84x48 pixels.
U8GLIB_PCD8544 u8g(10, 9 , 8);

#include "main.h"


inline void fn_button_confirmdatetime(m2_el_fnarg_p fnarg) {
  tm.Year = y2kYearToTm(tm.Year);
  //Set the DS1307 RTC chip
  RTC.write(tm);
  home_menu();
}

inline void fn_button_canceltimer(m2_el_fnarg_p fnarg) {
  //Disabled
  EEPROM.write(0, 0);
  timerEnabled = 0;
  home_menu();
}

inline void fn_button_confirmtimer(m2_el_fnarg_p fnarg) {
  //Enabled
  EEPROM.write(0, 255);
  EEPROM.write(1, tm.Hour);
  EEPROM.write(2, tm.Minute);

  timerEnabled = 255;
  timerHour = tm.Hour;
  timerMinute = tm.Minute;
  home_menu();
}


//Display 84x48 pixels
//M2_SPACE(el_menu_space, "w1h2");

void fn_button_showhome(m2_el_fnarg_p fnarg) {
  //Close clock and show top menu
  home_menu();
}

/* DISPLAY FOR INFO PAGES */
M2_INFO(el_labelptr, "w76l5" , &first_visible_line1, &total_lines1, value, fn_button_showhome);
M2_VSB(el_vsb, "w4l5" , &first_visible_line1, &total_lines1);
M2_LIST(list_strlist) = { &el_labelptr, &el_vsb };
M2_HLIST(el_strlist_hlist, NULL, list_strlist);
M2_ALIGN(el_infopages_root, NULL, &el_strlist_hlist);


/* CLOCK/IDLE page */
//M2_LABELFN(el_big_label_clock, "f8w64", label_footer);
//M2_LABELFN(el_big_label_date, "f8w64", label_footer2);
//M2_BUTTON(el_button_clock_close, "f8w64", "Menu", fn_button_showhome);
//M2_LIST(list_clocklist) = {&el_big_label_clock, &el_menu_space, &el_menu_space, &el_big_label_date, &el_menu_space, &el_menu_space, &el_button_clock_close };
//M2_VLIST(vlist_clocklist, NULL, list_clocklist);
//M2_ALIGN(el_bigclock, NULL, &vlist_clocklist);

/* SET DATE */
//Date
M2_U8NUM(el_dt_day, "c2", 1, 31, &tm.Day);
M2_LABEL(el_dt_sep1, NULL, "/");
M2_U8NUM(el_dt_month, "c2", 1, 12, &tm.Month);
M2_LABEL(el_dt_sep2, NULL, "/20");
M2_U8NUM(el_dt_year, "c2", 15, 35, &tm.Year);
M2_LIST(list_date) = { &el_dt_day, &el_dt_sep1, &el_dt_month, &el_dt_sep2, &el_dt_year };
M2_HLIST(el_date, NULL, list_date);

//Time
M2_U8NUM(el_ti_hour, "c2", 0, 23, &tm.Hour);
M2_LABEL(el_ti_sep1, NULL, ":");
M2_U8NUM(el_ti_mins, "c2", 0, 59, &tm.Minute);
M2_LIST(list_time) = { &el_ti_hour, &el_ti_sep1, &el_ti_mins};
M2_HLIST(el_time, NULL, list_time);

//Buttons
M2_BUTTON(el_dt_ok, NULL, "OK", fn_button_confirmdatetime);
M2_BUTTON(el_dt_cancel, NULL, "CANCEL", fn_button_showhome);
M2_LIST(list_dt_buttons) = { &el_date, &el_time, &el_dt_ok, &el_dt_cancel };
//Vertical align/list
M2_VLIST(el_top_dt2, NULL, list_dt_buttons);
M2_ALIGN(el_top_dt, NULL, &el_top_dt2);



//SET TIMER
M2_BUTTON(el_dt_ok2, NULL, "OK", fn_button_confirmtimer);
M2_BUTTON(el_dt_cancel2, NULL, "CANCEL", fn_button_canceltimer);
M2_LIST(list_dt_buttons2) = { &el_time, &el_dt_ok2, &el_dt_cancel2 };
M2_VLIST(el_top_dt3, NULL, list_dt_buttons2);
M2_ALIGN(el_top_settimer, NULL, &el_top_dt3);


const char *fn_value(m2_rom_void_p element)
{
  return value;
}

/* DIALOG */
M2_LABELFN(el_dialog_label, NULL, fn_value);
M2_BUTTON(el_button_close_dialog, NULL, "Close", fn_button_showhome);
M2_LIST(list_dialog) = { &el_dialog_label, &el_button_close_dialog };
M2_VLIST(el_dialog_vlist, NULL, list_dialog);
M2_ALIGN(top_dialog, NULL, &el_dialog_vlist);

/* MAIN MENU */
M2_STRLIST(el_strlist_menu, "l3w76" , &first_visible_line1, &total_menu_items, el_strlist_mainmenu);
M2_VSB(el_vsb_menu, "l3w4" , &first_visible_line1, &total_menu_items);
M2_LIST(list_menu_strlist) = { &el_strlist_menu, &el_vsb_menu };
M2_HLIST(el_menu_hlist, NULL, list_menu_strlist);

const char *label_footer(m2_rom_void_p element)
{
  return textline1;
}
const char *label_footer2(m2_rom_void_p element)
{
  return textline2;
}
//Footer at bottom of screen
M2_LABELFN(el_footer_label, NULL, label_footer);
M2_LABELFN(el_footer_label2, NULL, label_footer2);






char *copy_string(char *buf, const char* progmemstring)
{
  strcat_P(buf, progmemstring);
  buf += strlen_P(progmemstring);
  return buf;
}


void build_info_text_basic()
{
  char* v = value;
  wb_basic_info_t wb_info;

  int err = wbus_get_basic_info(&wb_info);
  if (!err) {
    v = copy_string(v, label_DeviceName);
    strcat(v, wb_info.dev_name); v += strlen(wb_info.dev_name);

    v = copy_string(v, label_DeviceIDNo);
    v = hexdump(v, wb_info.dev_id, 4, false);
    *v++ = wb_info.dev_id[4];

    v = copy_string(v, label_DateOfManufactureControlUnit);
    v = PrintHexByte(v, wb_info.dom_cu[0]);
    *v++ = '.';
    v = PrintHexByte(v, wb_info.dom_cu[1]);
    *v++ = '.';
    v = i2str(2000 + wb_info.dom_cu[2], v);

    v = copy_string(v, label_DateOfManufactureHeater);
    v = PrintHexByte(v, wb_info.dom_ht[0]);
    *v++ = '.';
    v = PrintHexByte(v, wb_info.dom_ht[1]);
    *v++ = '.';
    v = i2str(2000 + wb_info.dom_ht[2], v);

    v = copy_string(v, label_CustomerIdentificationNumber);
    strcat(v, (const char*)wb_info.customer_id);
    v += strlen((const char*)wb_info.customer_id);

    v = copy_string(v, label_SerialNumber);
    v = hexdump(v, wb_info.serial, 5, false);

  } else {
    v = ShowError(err);
  }
}

const char *label_clocktext(m2_rom_void_p element)
{
  return clocktext;
}

M2_LABELFN(el_label_clocktext, "b1", label_clocktext);

M2_LIST(list_menu2) = { &el_label_clocktext, &el_menu_hlist, &el_footer_label, &el_footer_label2 };
M2_VLIST(el_menu_vlist, NULL, list_menu2);
M2_ALIGN(top_buttonmenu, "-0|2", &el_menu_vlist);

//U8
M2tk m2(&top_buttonmenu, m2_es_arduino_rotary_encoder , m2_eh_4bd, m2_gh_u8g_bf);


void fn_faults() {
  clearBuffer();
  char* v = value;
  int err;

  unsigned char ErrorList[32];

  //Ensure empty array to start with
  memset(ErrorList, 0, sizeof(ErrorList));

  err = wbus_get_fault_count(ErrorList);

  if (!err) {

    if (ErrorList[0] > 0) {
      err_info_t err_info;

      //  Fault 1
      //  Metering pump interruption 0x88
      //  Code 88h
      //  State: stored, not actual  =0x1
      //  Counter 3
      //  Temp: 22oC
      //  Operating state: Off state  4,0
      //  Voltage: 12.55V
      //  Operating hour counter: 5590:15 (h:m)
      for (int errIndex = 0; errIndex < ErrorList[0]; errIndex++ ) {

        v = copy_string(v, label_ErrorCount);
        v = i2str(errIndex + 1, v);
        *v++ = '\n';

        unsigned char errCode = ErrorList[errIndex * 2 + 1];

        err = wbus_get_fault(errCode, &err_info);

        if (!err) {
          *v++ = 'E';
          *v++ = ':';
          v = PrintHexByte(v, err_info.code);
          *v++ = ' ';

          //01
          *v++ = 'F';
          *v++ = ':';
          v = PrintHexByte(v, err_info.flags);
          *v++ = ' ';

          //03
          *v++ = 'C';
          *v++ = ':';
          v = i2str(err_info.counter, v);

          //04 00
          //          strcat_P(v, label_OperatingState); v += strlen_P(label_OperatingState);
          //          v = PrintHexByte(v, err_info.op_state[0]);
          //          *v++ = '/';
          //          v = PrintHexByte(v, err_info.op_state[1]);
          //          *v++ = '\n';

          //48
          //          strcat_P(v, label_Temperature); v += strlen_P(label_Temperature);
          //          v = i2str(BYTE2TEMP(err_info.temp), v);

          //31 06
          //          strcat_P(v, label_SupplyVoltage); v += strlen_P(label_SupplyVoltage);
          //          v = i2str( twobyte2word(err_info.volt), v);

          //5586:31 (h:m) = 15 d2 1f
          v = copy_string(v, label_OperatingTime);
          v = i2str( WORD2HOUR(err_info.hour), v);
          *v++ = 'h';
          v = i2str( err_info.minute, v);
          *v++ = 'm';
          *v++ = '\n';

        } else {
          v = ShowError(err);
        }
      }

    } else {
      v = copy_string(v, label_NoFaultsFound);
      m2.setRoot(&top_dialog);
      return;
    }

  } else {
    //WBUS error
    v = ShowError(err);
  }

  //Ensure null terminate on string
  *v = 0;
  m2.setRoot(&el_infopages_root);

}


inline void fn_clear_faults() {
  clearBuffer();
  char* v = value;

  int err = wbus_clear_faults();

  if (!err) {
    strcpy_P(v, label_FaultsCleared);
  } else {
    //WBUS error
    v = ShowError(err);
  }

  m2.setRoot(&top_dialog);
}



void clearBuffer() {
  memset(value, 0, sizeof(value));
}



char* ShowError(uint8_t errCode) {
  clearBuffer();
  char* v = value;
  v = copy_string(v, label_wbusError);
  v = PrintHexByte(v, errCode);
  return v;
}



unsigned long getTimeFunction() {
  tmElements_t tm;

  if (RTC.read(tm)) {
    return makeTime(tm);
  }

  //Error so just return what we already have
  return now();
}

void home_menu() {
  first_visible_line1 = 0;
  m2.setRoot(&top_buttonmenu);
}

inline void fn_begin_set_date_time() {
  breakTime(getTimeFunction(), tm);
  tm.Year = tmYearToY2k(tm.Year);
  m2.setRoot(&el_top_dt);
}

void updateClockString() {
  if (timeStatus() != timeNotSet) {
    tmElements_t tm;
    breakTime(now(), tm);

    char* v = clocktext;

    /* Now the date */

    /*
        if (tm.Wday == 1) {
          strcat_P(v, label_Sun);
        } else {
          strcpy_P(v, (char*)pgm_read_word(&(weekdays_table[tm.Wday])));
        }
        v += 4;
    */

    v = i2str_zeropad( tm.Day, v);
    *v++ = '/';
    v = i2str_zeropad( tm.Month, v);
    *v++ = '/';
    v = i2str_zeropad( tmYearToCalendar(tm.Year) - 2000, v);

    *v++ = ' ';
    *v++ = timerEnabled ? 'T' : ' ';
    *v++ = ' ';

    //v = copy_string(v, label_threespaces);

    v = i2str_zeropad( tm.Hour, v);
    *v++ = ':';
    v = i2str_zeropad( tm.Minute, v);

    //Terminator
    //*v = 0;


    if ((timerEnabled == 255) && (tm.Hour == timerHour) && (tm.Minute == timerMinute) && (heaterMode == WBUS_CMD_OFF)) {
      //Switch on HEATER!
      //heaterOn();
      digitalWrite(6, LOW);
    }
    else
    {
      digitalWrite(6, HIGH);
    }


  }
}

bool heaterOn() {
  //WBUS_CMD_ON_PH
  if (int err = wbus_turnOn(WBUS_CMD_ON_SH, 60) == 0) {
    heaterMode = WBUS_CMD_ON_SH;

    return true;
  } else {
    ShowError(err);
    return false;
  }
}



inline void fn_begin_set_timer() {
  tm.Hour = timerHour;
  tm.Minute = timerMinute;
  m2.setRoot(&el_top_settimer);
}

const char *el_strlist_mainmenu(uint8_t idx, uint8_t msg) {
  char *v = value;
  *v = 0;

  switch (idx)
  {
    case 0:
      strcpy_P(v, label_menu_shheaton);
      break;
    case 1:
      strcpy_P(v, label_menu_shheatoff);
      break;
    case 2 :
      strcpy_P(v, label_menu_settimer);
      break;
    case 3 :
      strcpy_P(v, label_menu_heaterinfo);
      break;
    case 4:
      strcpy_P(v, label_menu_showfaults);
      break;
    case 5:
      strcpy_P(v, label_menu_clearfaults);
      break;
    case 6:
      strcpy_P(v, label_menu_setdatetime);
      break;
    case 7:
      strcpy_P(v, label_menu_fuelprime);
      break;
  };

  if ( msg == M2_STRLIST_MSG_SELECT ) {
    clearBuffer();

    switch (idx)
    {
      case 0:
        //Switch Supplimental heater ON

        if (!heaterOn()) {
          v = copy_string(v, label_menu_shheaton);
          m2.setRoot(&top_dialog);
        }
        break;

      case 1:
        //Switch Supplimental heater OFF

        if (int err = wbus_turnOff() == 0) {
          v = copy_string(v, label_menu_shheatoff);
        } else {
          ShowError(err);
        }

        m2.setRoot(&top_dialog);
        break;

      case 2 :
        //Set Timer
        fn_begin_set_timer();
        break;

      case 3 :
        //Basic heater info
        build_info_text_basic();
        m2.setRoot(&el_infopages_root);
        break;

      case 4:
        fn_faults();
        break;

      case 5:
        fn_clear_faults();
        break;

      case 6:
        fn_begin_set_date_time();
        break;

      case 7:
        //Start fuel prime
        strcpy_P(v, label_fuelprime);
        wbus_fuelPrime(10);
        m2.setRoot(&top_dialog);
        break;
    }
  }

  return value;
}


void keepAlive() {
  //The heater units controller appears to "sleep" after a few seconds of inactivity so keep pestering it to ensure it keeps awake.
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

      //Force check of heater to keep it running if its switched on
      if (heaterMode != WBUS_CMD_OFF) {
        wbus_check(heaterMode);
      }

      updatedisplay = true;

      //Clear text strings
      memset(textline1, 0, sizeof(textline1));
      memset(textline2, 0, sizeof(textline2));

      //Update every 3 refreshes (about 8 seconds)
      updateDisplayCounter = 2 + 1;

      updateClockString();

      char* v = textline1;
      if (err != 0) {
        //What to do with errors?
        v = copy_string(v, label_wbusError);
        v = PrintHexByte(v, err);
      } else {

        if (m2.getRoot() == &top_buttonmenu) {
          //Must be the menu, switch text in a cycle

          switch (footerToggle) {
            case 0:

              //QUERY_STATE results
              //Op:
              v = copy_string(v, label_op);
              v = PrintHexByte(v, KeepAlive_wb_sensors.value[OP_STATE]);

              //OP_STATE value maps to the WB_STATE_ values, for instance 0x04 = WB_STATE_OFF

              v = copy_string(v, label_N);
              v = i2str( KeepAlive_wb_sensors.value[OP_STATE_N], v);
              *v = 0;

              v = textline2;
              v = copy_string(v, label_Dev);
              v = PrintHexByte(v, KeepAlive_wb_sensors.value[DEV_STATE]);
              *v = 0;

              break;

            case 1:
              //QUERY_SENSORS results
              //Temperature
              *v++ = 't';
              v = i2str( BYTE2TEMP(KeepAlive_wb_sensors.value[SEN_TEMP]), v);

              //SupplyVoltage
              *v++ = ' ';
              *v++ = 'v';
              v = i2str( twobyte2word(&KeepAlive_wb_sensors.value[SEN_VOLT]), v);

              //Null terminator
              //*v = 0;

              //Change to second line
              v = textline2;

              //Flame detect
              *v++ = 'f';
              v = i2str( KeepAlive_wb_sensors.value[SEN_FD], v);

              //Heat energy
              *v++ = ' ';
              *v++ = 'h';
              v = i2str( twobyte2word(&KeepAlive_wb_sensors.value[SEN_HE]), v);

              //glow plug resistance in mili Ohm
              *v++ = ' ';
              *v++ = 'g';
              v = i2str( KeepAlive_wb_sensors.value[SEN_GPR], v);

              //Null terminator
              //*v = 0;

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


void backlightOn() {
  digitalWrite(lcdBacklight, HIGH);
}

void backlightOff() {
  digitalWrite(lcdBacklight, LOW);
}


void loop() {
  keepAlive();

  // menu management
  m2.checkKey();
  m2.checkKey();

  //Redraw screen if key press event or updatedisplay flag is true
  if (m2.handleKey() | updatedisplay) {

    if (!updatedisplay) {
      backlightOn();
      //Key was pressed so reset clock timer to 45 seconds
      time_since_key_press = millis() + (30000);
    }

    updatedisplay = false;

    u8g.firstPage();
    do {
      m2.draw();
    } while ( u8g.nextPage() );
  }

  //Must be a way to save us 4 bytes instead of using an unsigned long for time_since_key_press
  if (millis() > time_since_key_press) {
    backlightOff();
  }
}


void setup() {
  pinMode(lcdBacklight, OUTPUT);
  //Red LED on
  pinMode(6, OUTPUT);

  backlightOn();

  timerEnabled = EEPROM.read(0);
  timerHour = EEPROM.read(1);
  timerMinute = EEPROM.read(2);

  getTimeFunction();
  setSyncProvider(getTimeFunction);
  setSyncInterval(600);  //10 minutes

  updateClockString();

  // put your setup code here, to run once:
  /* connect u8glib with m2tklib */
  m2_SetU8g(u8g.getU8g(), m2_u8g_box_icon);

  //m2_SetU8gAdditionalTextXBorder(2);
  //m2_SetU8gAdditionalReadOnlyXBorder(2);

  /* assign u8g font to index 0 */

  //m2.setFont(0,  u8g_font_5x7r);
  m2.setFont(0,  u8g_font_5x8r);

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

  //  m2.setRoot(&el_bigclock);
  home_menu();
}
