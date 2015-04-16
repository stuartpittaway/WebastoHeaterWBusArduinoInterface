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

*/

#include <Arduino.h>

#include <Time.h>
#include <TimeAlarms.h>

#include <Wire.h>
#include <DS1307RTC.h>

#include "constants.h"

#include "wbus.h"


#include <M2tk.h>
//Only for u8glib
#include <U8glib.h>
#include "utility/m2ghu8g.h"

#define uiKeySelectPin  2
#define uiKeyNextPin  3

//Pins for KS0108 based display, this one is 192x64 pixels and needs a lot of pins!
U8GLIB_KS0108_192 u8g(
  //D0-D7
  8, 9, 10, 11, 4, 5, 6, 7,
  //en 18=A4 i2c
  13,
  //cs1
  14,
  //cs2
  15,
  //cs3 19=A5 i2c
  12,
  //di
  17,
  //rw
  16 ); 		// en=18 (A4), cs1=14 (a0) , cs2= (a1),di=17 (a3),rw=16 (a2), cs3= 19 (a5)

/* Global static variables */
wbus wbusObject(Serial);

//Massive buffer for scrolling text/string output
char value[200];

//buffer for text output of date/time
char menuheader_date[13];
char menuheader_clock[6];

//buffer for text output of status
char menufooter[25];

uint8_t total_lines1 = 0;
uint8_t first_visible_line1 = 0;

uint8_t currentErrorOnScreen = 0;
unsigned char ErrorList[32];

unsigned long previousMillis;
unsigned long currentMillis;

wb_sensor_t KeepAlive_wb_sensors;
bool updatedisplay;

/* DISPLAY FOR INFO PAGES */
M2_INFO(el_labelptr, "w178l5f0" , &first_visible_line1, &total_lines1, value, fn_information_close);
//M2_SPACE(el_space, "w1h1");
M2_VSB(el_strlist_vsb, "w4l5" , &first_visible_line1, &total_lines1);
M2_LIST(list_strlist) = { &el_labelptr, &el_strlist_vsb };
M2_HLIST(el_strlist_hlist, NULL, list_strlist);
M2_ALIGN(el_infopages_root, "-1|1W192H64", &el_strlist_hlist);

/* FAULT DISPLAY*/
M2_INFO(el_labelptr2, "w178l5f0" , &first_visible_line1, &total_lines1, value, fn_shownextfault);
M2_LIST(list_strlist2) = { &el_labelptr2, &el_strlist_vsb };
M2_HLIST(el_strlist_hlist2, NULL, list_strlist2);
M2_ALIGN(top_nextfaulttopelement, "-1|1W192H64", &el_strlist_hlist2);

/* MAIN MENU */
//M2_LABELFN(el_clock_label, NULL, label_clock);
//M2_LABELFN(el_date_label, NULL, label_date);
M2_LABELFN(el_footer_label, NULL, label_footer);

M2_BUTTON(el_button_information_basic, "", "Heater info", fn_information_basic);
M2_BUTTON(el_button_information_versions, "", "Version info", fn_information_versions);
M2_BUTTON(el_button_faults, "", "Show faults", fn_faults);
M2_BUTTON(el_button_clear_faults, "", "Clear faults", fn_clear_faults);
M2_SPACE(el_menu_space, "w1h2");

//M2_LIST(list_headerbarlist) = { &el_clock_label, &el_date_label};
//M2_HLIST(el_headerbarlist, NULL, list_headerbarlist);
//M2_LIST(list_buttonlist) = { &el_headerbarlist, &el_menu_space, &el_button_information_basic, &el_button_information_versions, &el_button_faults, &el_button_clear_faults, &el_menu_space, &el_footer_label };
M2_LIST(list_buttonlist) = { &el_button_information_basic, &el_button_information_versions, &el_button_faults, &el_button_clear_faults, &el_menu_space, &el_footer_label };
M2_VLIST(el_buttonmenu, NULL, list_buttonlist);
M2_ALIGN(top_buttonmenu, "-0|2W64H64", &el_buttonmenu);


M2_LABELFN(el_big_label_clock, "f1x16y32", label_clock);
M2_LABELFN(el_big_label_date, "x0y0", label_date);
M2_BUTTON(el_button_clock_close, "x98y0", "Menu", fn_button_closeclock);
M2_LIST(list_clocklist) = {&el_big_label_clock, &el_big_label_date, &el_button_clock_close };
M2_XYLIST(el_bigclock, NULL, list_clocklist);
//M2_ALIGN(el_bigclock, "-1|1W64H64", &el_bigclock_vlist);

/*
    element: Root element of the menu.
    m2_es_arduino
    m2_es_arduino_rotary_encoder
    m2_es_arduino_serial
    es: Event source, which connects the menu to hardware buttons.
    m2_eh_2bs
    m2_eh_4bs
    m2_eh_4bd
    m2_eh_6bs
    eh: Event handler, which maps hardware events to some specific actions on the manu.
    m2_gh_u8g_bf
    m2_gh_u8g_bfs
    m2_gh_u8g_fb
    m2_gh_u8g_ffs
    gh: Graphics handler, which writes the menu to an output device.
*/

//GLCD
//M2tk m2(&el_infopages_root, m2_es_arduino_serial , m2_eh_6bs, m2_gh_glcd_bf);

//U8
M2tk m2(&top_buttonmenu, m2_es_arduino , m2_eh_2bs, m2_gh_u8g_bf);


const char *label_date(m2_rom_void_p element)
{
  return menuheader_date;
}
const char *label_clock(m2_rom_void_p element)
{
  return menuheader_clock;
}
const char *label_footer(m2_rom_void_p element)
{
  return menufooter;
}

char c2h(char c)
{
  //TODO: Move string into PROGMEM ?
  return "0123456789ABCDEF"[0x0F & (unsigned char)c];
}

char* PrintHexByte(char *str, unsigned char d) {
  str[0] = c2h(d >> 4);
  str[1] = c2h(d);
  str += 2;
  return str;
}

char* hexdump(char *str, unsigned char *d, int l, bool appendNewLine)
{
  for (l--; l != -1; l--) {
    str = PrintHexByte(str, *d++);
  }

  if (appendNewLine) {
    str[0] = '\n';
    str++;
  }

  return str;
}

char *i2str_zeropad(int i, char *buf) {
  if (i < 10) *buf++ = '0';

  return i2str(i, buf);
}

char *i2str(int i, char *buf) {
  /* integer to string convert */
  byte l = 0;

  if (i < 0) *buf++ = '-';

  if (i == 0) {
    *buf++ = '0';
    return buf;
  }

  boolean leadingZ = true;

  for (int div = 10000, mod = 0; div > 0; div /= 10) {
    mod = i % div;
    i /= div;
    if (!leadingZ || i != 0) {
      leadingZ = false;
      *buf++ = i + '0';
    }
    i = mod;
  }

  return buf;
}


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


void build_info_text_versions()
{
  //28320 code/1452 bytes ram

  char* v = value;
  wb_version_info_t wb_info;

  int err = wbusObject.wbus_get_version_wbinfo(&wb_info);

  if (!err) {
    unsigned char b;

    //28114, 1368 bytes
    //28132, 1368
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
  v[0] = 0;
}


void build_info_text_basic()
{
  char* v = value;
  wb_basic_info_t wb_info;

  int err = wbusObject.wbus_get_basic_info(&wb_info);
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
  v[0] = 0;
}

void fn_information_versions(m2_el_fnarg_p fnarg) {
  clearBuffer();
  build_info_text_versions();
  m2.setRoot(&el_infopages_root);
}

void fn_information_basic(m2_el_fnarg_p fnarg) {
  clearBuffer();
  build_info_text_basic();
  m2.setRoot(&el_infopages_root);
}
void fn_button_closeclock(m2_el_fnarg_p fnarg) {
  //Close clock and show top menu
  m2.setRoot(&top_buttonmenu);
}
void fn_information_close(m2_el_fnarg_p fnarg) {
  //Return to top home menu
  m2.setRoot(&top_buttonmenu);
}

inline void WORDSWAP(unsigned char *out, unsigned char *in)
{
  /* No pointer tricks, to avoid alignment problems */
  out[1] = in[0];  out[0] = in[1];
}

inline short twobyte2word(unsigned char *in)
{
  short result;
  WORDSWAP((unsigned char*)&result, in);
  return result;
}

/*
static inline int shortToMili(char *t, short x)
{
  sprintf(t, "%06d", x);
  t[0] = t[1];
  t[1] = t[2];
  t[2] = '.';
  return 6;
}
*/

#define BYTE2TEMP(x)	  ((unsigned char)((x)-50))
#define WORD2HOUR(w)	  twobyte2word(w)
#define WORD2VOLT_TEXT(t, w) shortToMili(t, twobyte2word(w))


char* faultHeaders(char* v) {
  strcat_P(v, label_ErrorCount); v += strlen_P(label_ErrorCount);
  v = i2str(currentErrorOnScreen + 1, v);
  v++[0] = '/';
  v = i2str(ErrorList[0], v);
  v++[0] = '\n';
  return v;
}

void fn_faults(m2_el_fnarg_p fnarg) {
  clearBuffer();
  char* v = value;
  int err;

  //Ensure empty array to start with
  memset(ErrorList, 0, sizeof(ErrorList));

  err = wbusObject.wbus_get_fault_count(ErrorList);

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
    m2.setRoot(&el_infopages_root);
  }
}


void fn_clear_faults(m2_el_fnarg_p fnarg) {
  clearBuffer();
  char* v = value;

  int err = wbusObject.wbus_clear_faults();

  if (!err) {

    strcat_P(v, label_FaultsCleared); v += strlen_P(label_FaultsCleared);

  } else {
    //WBUS error
    v = ShowError(v, err);
  }

  //Ensure null terminate on string
  v[0] = 0;

  m2.setRoot(&el_infopages_root);
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
    m2.setRoot(&el_infopages_root);
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

  err = wbusObject.wbus_get_fault(errCode, &err_info);
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

    //Only update the clock if the seconds are less than 10, this reduces CPU load by not refreshing the screen all the time
    memset(menuheader_clock, 0, sizeof(menuheader_clock));
    memset(menuheader_date, 0, sizeof(menuheader_date));

    char* v = menuheader_clock;
    v = i2str_zeropad( tm.Hour, v);
    v++[0] = ':';
    v = i2str_zeropad( tm.Minute, v);
    //v++[0] = '.';
    //v = i2str( tm.Second, v);
    v++[0] = 0;

    /* Now the date */
    v = menuheader_date;
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
  //
  //m2.setFont(0,  u8g_font_courR08r);
  //m2.setFont(0, u8g_font_helvR08r);
  m2.setFont(0, u8g_font_profont10r);
  //m2.setFont(0, u8g_font_profont11r);
  //m2.setFont(0, u8g_font_profont12r);

  //m2.setFont(0, u8g_font_lucasfont_alternater);  //Very compact font
  //not good m2.setFont(0, u8g_font_tpssr);
  //m2.setFont(0, u8g_font_6x10r);
  //m2.setFont(0, u8g_font_ncenR08r);

  //Large clock font - numbers only
  //m2.setFont(1, u8g_font_freedoomr25n);
  m2.setFont(1, u8g_font_helvB24n);
  //m2.setFont(1, u8g_font_fub25n);

  //m2_SetU8gToggleFontIcon(u8g_font_m2icon_9, active_encoding, inactive_encoding);
  //m2_SetU8gRadioFontIcon(u8g_font_m2icon_9, active_encoding, inactive_encoding);

  m2.setPin(M2_KEY_SELECT, uiKeySelectPin);
  m2.setPin(M2_KEY_NEXT, uiKeyNextPin);

  wbusObject.wbus_init();

  //Return to top home menu
  //m2.setRoot(&top_buttonmenu);

  m2.setRoot(&el_bigclock);

}

bool footerToggle;
byte updateDisplayCounter = 0;

void keepAlive() {
  //The heater units controller appears to "sleep" after a few seconds of inactivity
  //so keep pestering it to ensure it keeps awake.
  currentMillis = millis();

  //Only need to communicate every few seconds
  if ((currentMillis - previousMillis) > 1700) {
    previousMillis = currentMillis;

    //Query heater every few seconds to keep it alive
    int err;
    if (footerToggle) {
      err = wbusObject.wbus_sensor_read(&KeepAlive_wb_sensors, QUERY_STATE);
    } else {
      err = wbusObject.wbus_sensor_read(&KeepAlive_wb_sensors, QUERY_SENSORS);
    }

    if (updateDisplayCounter == 0 || err != 0) {

      updatedisplay = true;
      updateClockString();

      memset(menufooter, 0, sizeof(menufooter));
      char* v = menufooter;

      if (err != 0) {
        //What to do with errors?
        strcat_P(v, label_wbusError); v += strlen_P(label_wbusError);
        v = PrintHexByte(v, err);
      } else {

        if (footerToggle) {
          //strcat_P(v, label_NoFaultsFound);
          //v += sprintf(v, "OP:0x%x, N:%d  Dev:0x%x", KeepAlive_wb_sensors.value[0], KeepAlive_wb_sensors.value[1], KeepAlive_wb_sensors.value[2]);

          strcat_P(v, label_OP); v += strlen_P(label_OP);
          v = PrintHexByte(v, KeepAlive_wb_sensors.value[0]);
          strcat_P(v, label_N); v += strlen_P(label_N);
          v = i2str( KeepAlive_wb_sensors.value[1], v);
          strcat_P(v, label_Dev); v += strlen_P(label_Dev);
          v = PrintHexByte(v, KeepAlive_wb_sensors.value[2]);
          v++[0] = 0;
        } else {
          //v += sprintf(str, "Temp = %d, Volt = ", BYTE2TEMP(s->value[0]));
          //v += WORD2VOLT_TEXT(str, s->value + 1);
          //v += sprintf(str, ", FD = %d, HE = %d, GPR = %d",s->value[3], WORD2WATT(s->value + 4), s->value[6]);
          //v+= BYTE2GPR_TEXT(str, s->value[7]);

          //strcat_P(v, label_Temperature); v += strlen_P(label_Temperature);
          v[0] = 'T'; v++;
          v = i2str( BYTE2TEMP(KeepAlive_wb_sensors.value[0]), v);

          v[0] = ' '; v++;
          v[0] = 'V'; v++;
          //strcat_P(v, label_SupplyVoltage); v += strlen_P(label_SupplyVoltage);
          v = i2str( twobyte2word(&KeepAlive_wb_sensors.value[1]), v);

          v[0] = ' '; v++;
          v[0] = 'F'; v++;
          v = i2str( KeepAlive_wb_sensors.value[3], v);

          v[0] = ' '; v++;
          v[0] = 'H'; v++;
          v = i2str( twobyte2word(&KeepAlive_wb_sensors.value[4]), v);

          v[0] = ' '; v++;
          v[0] = 'G'; v++;
          v = i2str( KeepAlive_wb_sensors.value[6], v);
        }
      }

      //Update every 5 refreshes
      updateDisplayCounter = 5 + 1;

      footerToggle = !footerToggle;

    }
    updateDisplayCounter--;

  }
}



void loop() {
  keepAlive();

  // menu management
  m2.checkKey();

  if ( m2.handleKey() | updatedisplay ) {
    updatedisplay = false;
    u8g.firstPage();
    do {
      m2.draw();
    } while ( u8g.nextPage() );
  }
}
