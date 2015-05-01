/*
 * W-Bus constant definitions
 *
 * Original Author: Manuel Jander  mjander@users.sourceforge.net
 *
 * Modifications by Stuart Pittaway for Arduino
 */

#include <HardwareSerial.h>
#include <Arduino.h>
#include "wbus_const.h"


typedef struct
{
  unsigned char wbus_ver;
  unsigned char wbus_code[7];
  unsigned char data_set_id[6];
  unsigned char sw_id[6];
  unsigned char hw_ver[2]; // week / year
  unsigned char sw_ver[5]; // day of week / week / year // ver/ver
  unsigned char sw_ver_eeprom[5];
} wb_version_info_t;

typedef wb_version_info_t *HANDLE_VERSION_WBINFO;


typedef struct
{
  char dev_name[9];
  unsigned char dev_id[5];
  unsigned char dom_cu[3]; // day / week / year
  unsigned char dom_ht[3];
  unsigned char customer_id[20]; // Vehicle manufacturer part number
  unsigned char serial[5];
} wb_basic_info_t;

typedef wb_basic_info_t *HANDLE_BASIC_WBINFO;


typedef struct
{
  unsigned char length;
  unsigned char idx;
  unsigned char value[32];
} wb_sensor_t;

typedef wb_sensor_t *HANDLE_WBSENSOR;

typedef struct {
  unsigned char code;
  unsigned char flags;
  unsigned char counter;
  unsigned char op_state[2];
  unsigned char temp;
  unsigned char volt[2];
  unsigned char hour[2];
  unsigned char minute;
} err_info_t;

typedef err_info_t *HANDLE_ERRINFO;

int wbus_sensor_read(HANDLE_WBSENSOR sensor, int idx);
void wbus_init();
//int wbus_get_version_wbinfo(HANDLE_VERSION_WBINFO i);
int wbus_get_basic_info(HANDLE_BASIC_WBINFO i);

int wbus_get_fault_count(unsigned char ErrorList[32]);
int wbus_get_fault(unsigned char ErrorNumber, HANDLE_ERRINFO errorInfo);
int wbus_clear_faults();

int wbus_turnOn(unsigned char mode, unsigned char time);
int wbus_turnOff();
/* Check or keep alive heating process. mode is the argument that was passed as to wbus_turnOn() */
int wbus_check(unsigned char mode);
int wbus_fuelPrime( unsigned char time);


