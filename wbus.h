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


class wbus
{
  private:
    int wbus_msg_recv(uint8_t *addr, uint8_t *cmd, uint8_t *data, int *dlen, int skip);
    int wbus_msg_send( uint8_t addr, uint8_t cmd, uint8_t *data, int len, uint8_t *data2, int len2);
    unsigned char checksum(unsigned char *buf, unsigned char len, unsigned char chk);
    char* getVersion(char *str, unsigned char *d);
    void OpenSerial();
    char* hexdump(char *str, unsigned char *d, int l);
    int wbus_io( uint8_t cmd, uint8_t *out, uint8_t *out2, int len2, uint8_t *in, int *dlen, int skip);
    HardwareSerial& _MySerial;

  public:
    // Constructor takes address of serial port
    wbus(HardwareSerial& serial);
    int wbus_sensor_read(HANDLE_WBSENSOR sensor, int idx);
    //void wbus_sensor_print(char *str, HANDLE_WBSENSOR s);
    //void wbus_ident_print(char *str, HANDLE_WBINFO i, int line);
    ~wbus();
    void wbus_init();
    int wbus_get_version_wbinfo(HANDLE_VERSION_WBINFO i);
    int wbus_get_basic_info(HANDLE_BASIC_WBINFO i);
    
    int wbus_get_fault_count(unsigned char ErrorList[32]);
    int wbus_get_fault(unsigned char ErrorNumber, HANDLE_ERRINFO errorInfo);
    int wbus_clear_faults();

};



