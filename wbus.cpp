/* wbus.cpp
 *
 * Original Author: Manuel Jander  mjander@users.sourceforge.net
 *
 * Modifications by Stuart Pittaway for Arduino
 */

#include "wbus.h"
#include <Arduino.h>
#include "wbus_const.h"


/* Data format / unit translation macros */
//static inline void SWAP(unsigned char *out, unsigned short in)
//{
//  out[1] = (in &  0xff);
//  out[0] = (in >> 8) & 0xff;
//}


/*
 * Convert a short representing mili something (volt, ohm) to a decimal string featuring a nice dot :)
 *
 * Why? Because many uC sprintf implementations do not support floats.
 */
//static inline int shortToMili(char *t, short x)
//{
//  sprintf(t, "%06d", x);
//  t[0] = t[1];
//  t[1] = t[2];
//  t[2] = '.';
//  return 6;
//}
//
//static inline int byteToMili(char *t, unsigned char x)
//{
//  sprintf(t, "%05d", x);
//  t[0] = t[1];
//  t[1] = '.';
//  return 5;
//}

//#define PERCENT2WORD(w, p) SWAP(&w, (short)(p)*2)
//#define FREQ2WORD(w, f)    SWAP(&w, (short)((f)*20) )
//#define TEMP2BYTE(x)	  ((x)+50)
//#define VOLT2WORD(w,v)	  SWAP(&w, v)
//#define WORD2FPWR_TEXT(t, w) shortToMili(t, twobyte2word(w))
//#define HOUR2WORD(w, h)	  SWAP(&w, h)
//#define BYTE2GPR_TEXT(t, b)  byteToMili(t, b)
//#define WATT2WORD(w, W)    SWAP(&w, W)
//#define WORD2WATT(w)       twobyte2word(w)
//#define OHM2WORD(w, o)     SWAP(&w, o)
//#define RPM2WORD(w, r)     SWAP(&w, r)
//#define WORD2RPM(w)        twobyte2word(w)


// Constructor takes address of serial port
wbus::wbus(HardwareSerial& serial) : _MySerial(serial)
{

}

wbus::~wbus()
{
  _MySerial.end();
}


void wbus::wbus_init() {
_MySerial.end();
  //Break set

#if defined(__AVR_ATmega328P__)
  //This code is for Arduino ATMEGA328 with single hardware serial port (pins 0 and 1)
  //PORTD=digital pins 0 to 7
  DDRD = DDRD | B00000010;  //Pin1 = output
  PORTD = PORTD | B00000010; // digital 1 HIGH
  delay(25);
  PORTD = PORTD & B11111101; // digital 1 LOW
  delay(25);
#endif


//#if defined(__AVR_ATmega2560__)
//  //This code is for Arduino ATMEGA2560 with hardware serial port 3 connected to WBUS, PORTJ is pin 14 and 15
//  //Signal is inverted by WBUS interface
//  
//  pinMode(_TXpin,OUTPUT);
//  digitalWrite(_TXpin,HIGH);
//  delay(250); 
//  digitalWrite(_TXpin,LOW);
//  delay(25);
//  digitalWrite(_TXpin,HIGH);
//  delay(25); 
//#endif


  // initialize serial communication at 2400 bits per second, 8 bit data, even parity and 1 stop bit
  _MySerial.begin(2400, SERIAL_8E1);
  
  //300ms for timeouts
  _MySerial.setTimeout(1000); 

  // Empty all queues. BRK toggling may cause a false received byte (or more than one who knows).
  while (_MySerial.available()) _MySerial.read();
  
}


/*
 * \param buf pointer to ACK message part
 * \param len length of data
 * \param chk initial value of the checksum. Useful for concatenating.
 */
unsigned char wbus::checksum(unsigned char *buf, unsigned char len, unsigned char chk)
{
  for (; len != 0; len--) {
    chk ^= *buf++;
  }
  return chk;
}


//char* wbus::getVersion(char *str, unsigned char *d)
//{
//  static const char *_wd[6] = {   "StrangeDay", "Mon ", "Tue ", "Wed ", "Thu ", "Fri " };
//
//  strcpy(str, _wd[d[0]]);
//  str += strlen(str);
//  str += sprintf(str, "%x/%x  %x.%x", d[1], d[2], d[3], d[4]);
//
//  return str;
//}


/**
 * Send request to heater and one or two consecutive buffers.
 * \param wbus wbus handle
 * \param cmd wbus command to be sent
 * \param data pointer for first buffer.
 * \param len length of first data buffer.
 * \param data pointer to an additional buffer.
 * \param len length the second data buffer.
 * \return 0 if success, 1 on error.
 */
int wbus::wbus_msg_send( uint8_t addr,
                         uint8_t cmd,
                         uint8_t *data,
                         int len,
                         uint8_t *data2,
                         int len2)
{
 
  uint8_t bytes, chksum;
  uint8_t buf[3];

  /* Assemble packet header */
  buf[0] = addr;
  buf[1] = len + len2 + 2;
  buf[2] = cmd;

  chksum = checksum(buf, 3, 0);
  chksum = checksum(data, len, chksum);
  chksum = checksum(data2, len2, chksum);


  //Add a small delay which appears to greatly improve reliability
  //of communications
  //delay(1);

  /* Send message */
  //rs232_flush(wbus->rs232);
  _MySerial.write(buf, 3);
  _MySerial.write(data, len);
  _MySerial.write(data2, len2);
  _MySerial.write(&chksum, 1);

  /* Read and check echoed header */
  bytes = _MySerial.readBytes((char*)buf, 3);
  if (bytes != 3) {
    return -1;
  }

  if (buf[0] != addr || buf[1] != (len + len2 + 2)  || buf[2] != cmd) {
    //PRINTF("wbus_msg_send() K-Line error %x %x %x\n", buf[0], buf[1], buf[2]);
    return -1;
  }

  /* Read and check echoed data */
  {
    int i;

    for (i = 0; i < len; i++) {
      bytes = _MySerial.readBytes((char*)buf, 1);
      if (bytes != 1 || buf[0] != data[i]) {
        //PRINTF("wbus_msg_send() K-Line error. %d < 1 (data2)\n", bytes);
        return -1;
      }
    }
    for (i = 0; i < len2; i++) {
      bytes = _MySerial.readBytes((char*)buf, 1);
      if (bytes != 1 || buf[0] != data2[i]) {
        //PRINTF("wbus_msg_send() K-Line error. %d < 1 (data2)\n", bytes);
        return -1;
      }
    }
  }

  /* Check echoed checksum */
  bytes = _MySerial.readBytes((char*)buf, 1);
  if (bytes != 1 || buf[0] != chksum) {
    //PRINTF("wbus_msg_send() K-Line error. %d < 1 (data2)\n", bytes);
    return -1;
  }

  return 0;
}


/*
 * Read answer from wbus
 * addr: source/destination address pair to be expected or returned in case of host I/O
 * cmd: if pointed location is zero, received client message, if not send client message
 with command *cmd.
 * data: buffer to either receive client data, or sent its content if client (*cmd!=0).
 If *data is !=0, then *data amount of data bytes will  be skipped.
 * dlen: out: length of data.
 */
int wbus::wbus_msg_recv(
  uint8_t *addr,
  uint8_t *cmd,
  uint8_t *data,
  int *dlen,
  int skip)
{
  uint8_t buf[3];
  uint8_t chksum;
  int len;

  /* Read address header */
  do {
    if (_MySerial.readBytes((char*)buf, 1) != 1) {

      //if (*cmd != 0) {
        //PRINTF("wbus_msg_recv(): timeout\n");
      //}
      return -1;
    }

    buf[1] = buf[0];

    /* Check addresses */
  }
  while ( buf[1] != *addr );

  //rs232_blocking(wbus->rs232, 0);

  /* Read length and command */
  if (_MySerial.readBytes((char*)buf + 1, 2) != 2) {
//    if (*cmd != 0) {
//     //wbus_msg_recv(): No addr/len error
//    }

    return -1;
  }

  {
    /* client case: check ACK */
    if (buf[2] != (*cmd | 0x80)) {
      /* Message reject happens. Do not be too picky about that. */
      *dlen = 0;
      return 0;
    }
  }

  chksum = checksum(buf, 3, 0);

  /* Read possible data */
  len = buf[1] - 2 - skip;

  if (len > 0 || skip > 0)
  {
    for (; skip > 0; skip--) {
      if (_MySerial.readBytes((char*)buf, 1) != 1) {
        return -1;
      }
      chksum = checksum(buf, 1, chksum);
    }

    if (len > 0) {
      if (_MySerial.readBytes((char*)data, len) != len) {
        return -1;
      }
      chksum = checksum(data, len, chksum);
    }
    /* Store data length */
    *dlen = len;
  }
  else {
    *dlen = 0;
  }

  /* Read and verify checksum */
  //rs232_read(wbus->rs232, buf, 1);
  _MySerial.readBytes((char*)buf, 1);

  /*
  if (*buf != chksum) {
   PRINTF("wbus_msg_recv() Checksum error\n");
   return -1;
   }
   */

  return 0;
}

/*
 * Send a client W-Bus request and read answer from Heater.
 */
int wbus::wbus_io( uint8_t cmd,
                   uint8_t *out,
                   uint8_t *out2,
                   int len2,
                   uint8_t *in,
                   int *dlen,
                   int skip)
{
  int err, tries;
  int len;
  unsigned char addr;

  len = *dlen;

  //rs232_blocking(wbus->rs232, 0);

  tries = 0;
  do {
    if (tries != 0) {
      //PRINTF("wbus_io() retry: %d\n", tries);
      delay(50);
      wbus_init();
    }

    tries++;
    /* Send Message */
    addr = (WBUS_CADDR << 4) | WBUS_HADDR;
    err = wbus_msg_send( addr, cmd, out, len, out2, len2);
    if (err != 0) {
      continue;
    }

    /* Receive reply */
    addr = (WBUS_HADDR << 4) | WBUS_CADDR;
    err = wbus_msg_recv( &addr, &cmd, in, dlen, skip);
    if (err != 0) {
      continue;
    }

  }
  while (tries < 4 && err != 0);

  //Appears that there is a small 70ms delay on the real Thermotest software between requests, see if mimick this here helps...
  delay(30);
  
  return err;
}

int wbus::wbus_get_fault_count(unsigned char ErrorList[32]) {
  int err, len;

  unsigned char tmp[2];

  tmp[0] = ERR_LIST;
  len = 1;
  err = wbus_io(WBUS_CMD_ERR, tmp, NULL, 0, ErrorList, &len, 1);
  //if (err) goto bail;
//bail:
  return err;
}

int wbus::wbus_clear_faults() {
  int err, len;
  unsigned char tmp;

  tmp = ERR_DEL; len = 1; err = wbus_io(WBUS_CMD_ERR, &tmp, NULL, 0, &tmp, &len, 0);
	 
  return err;  
}

int wbus::wbus_get_fault(unsigned char ErrorNumber, HANDLE_ERRINFO errorInfo) {
   int err, len;

  unsigned char tmp[2];

  len = 2;
  tmp[0] = ERR_READ;
  tmp[1] = ErrorNumber;
  err = wbus_io(WBUS_CMD_ERR, tmp, NULL, 0, (unsigned char*)errorInfo, &len, 1);
  
  if (err) goto bail;
  
bail:
  return err;
}

/* Overall info */
int wbus::wbus_get_wbinfo(HANDLE_WBINFO i)
{
  int err, len;
  unsigned char tmp, tmp2[2];

  //wbus_init();

  tmp = IDENT_WB_VER; len = 1; err = wbus_io(WBUS_CMD_IDENT, &tmp, NULL, 0, &i->wbus_ver, &len, 1);
  if (err) goto bail;

  tmp = IDENT_DEV_NAME; len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, (unsigned char*)i->dev_name, &len, 1);
  if (err) goto bail;
  i->dev_name[len] = 0; // Hack: Null terminate this string 

/*  
#if 0
  tmp = 3;              len = 1; err = wbus_io( WBUS_CMD_SL_RD, &tmp, NULL, 0, i->strange2, &len, 1);
#else
  tmp2[0] = 3; tmp2[1] = 7; len = 2; err = wbus_io( WBUS_CMD_SL_RD, tmp2, NULL, 0, i->strange2, &len, 2);
#endif
*/
  tmp = IDENT_WB_CODE;  len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->wbus_code, &len, 1);
  if (err) goto bail;

//  len = 0; err = wbus_io( WBUS_CMD_U1,  NULL, NULL, 0, i->strange, &len, 0);
//  if (err) goto bail;

  tmp = IDENT_DEV_ID;  len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->dev_id, &len, 1);
  if (err) goto bail;
  tmp = IDENT_HWSW_VER; len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->hw_ver, &len, 1);
  if (err) goto bail;
  tmp = IDENT_DATA_SET; len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->data_set_id, &len, 1);
  if (err) goto bail;
  tmp = IDENT_DOM_CU;  len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->dom_cu, &len, 1);
  if (err) goto bail;
  tmp = IDENT_DOM_HT;  len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->dom_ht, &len, 1);
  if (err) goto bail;
  //tmp = IDENT_U0;      len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->u0, &len, 1);
  //if (err) goto bail;
  tmp = IDENT_CUSTID;  len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->customer_id, &len, 1);
  if (err) goto bail;
  tmp = IDENT_SERIAL;  len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->serial, &len, 1);
  if (err) goto bail;
  tmp = IDENT_SW_ID;   len = 1; err = wbus_io( WBUS_CMD_IDENT, &tmp, NULL, 0, i->sw_id, &len, 1);

bail:
  return err;
}


/* Sensor access */
int wbus::wbus_sensor_read(HANDLE_WBSENSOR sensor, int idx)
{
  int err  = 0;
  int len;
  unsigned char sen;

  /* Tweak: skip some addresses since reading them just causes long delays. */
  switch (idx) {
    default:
      break;
    case 0:
    case 1:
    case 8:
    case 9:
    case 13:
    case 14:
    case 16:
      sensor->length = 0;
      sensor->idx = 0xff;
      return -1;
      break;
  }

  //wbus_init();

  sen = idx; len = 1;
  err = wbus_io(WBUS_CMD_QUERY, &sen, NULL, 0, sensor->value, &len, 1);
  if (err != 0)
  {
    //PRINTF("Reading sensor %d failed\n", );
    //(F("Read sensor fail")); _DebugSerial.println(idx);
    sensor->length = 0;
    return -1;
  }

  /* Store length of received value */
  sensor->length = len;
  sensor->idx = idx;
  return err;
}

#define BOOL(x) (((x)!=0)?1:0)

/*
void wbus::wbus_sensor_print(char *str, HANDLE_WBSENSOR s)
{
  unsigned char v;

  if (s->idx == 0xff)
  {
    strcpy(str, "skipped");
    return;
  }

  switch (s->idx) {
    case QUERY_STATUS0:
      str += sprintf(str, "SHR: %d, MS: %d, S: %d, D: %d, BOOST: %d AD: %d, T15: %d",
                     s->value[0] & STA00_SHR, s->value[0] & STA00_MS,
                     s->value[0] & STA01_S, s->value[0] & STA02_D,
                     s->value[0] & STA03_BOOST, s->value[0] & STA03_AD,
                     s->value[0] & STA04_T15);
      break;
    case QUERY_STATUS1:
      v = s->value[0]; str += sprintf(str, "State CF=%d GP=%d CP=%d VFR=%d",
                                      BOOL(v & STA10_CF), BOOL(v & STA10_GP), BOOL(v & STA10_CP), BOOL(v & STA10_VF));
      break;
    case QUERY_OPINFO0:
      str += sprintf(str, "Fuel type %x, Max heating time %d [min], factors %d %d",
                     s->value[0], s->value[1] * 10, s->value[2], s->value[3]);
      break;
    case QUERY_SENSORS:
      str += sprintf(str, "Temp = %d, Volt = ", BYTE2TEMP(s->value[0]));
      str += WORD2VOLT_TEXT(str, s->value + 1);
      str += sprintf(str, ", FD = %d, HE = %d, GPR = %d",
                     s->value[3], WORD2WATT(s->value + 4), s->value[6]);
      str += BYTE2GPR_TEXT(str, s->value[7]);
      break;
    case QUERY_COUNTERS1:
      str += sprintf(str, "Working hours %d:%d  Operating hours %u:%u Start Count %u",
                     WORD2HOUR(s->value), s->value[2], WORD2HOUR(s->value + 3), s->value[5], twobyte2word(s->value + 6));
      break;
    case QUERY_STATE:
      str += sprintf(str, "OP state: 0x%x, N: %d, Dev state: 0x%x", s->value[0], s->value[1], s->value[2]);
      break;
    case QUERY_DURATIONS0:
      
         //PH=parking heating, SH= supplemental heatig.
         //format: hours:minutes, 1..33%,34..66%,67..100%,>100%
       
      str += sprintf(str, "Burning duration PH %u:%u %u:%u %u:%u %u:%u SH %u:%u %u:%u %u:%u %u:%u",
                     WORD2HOUR(s->value), s->value[2], WORD2HOUR(s->value + 3), s->value[5], WORD2HOUR(s->value + 6), s->value[8],
                     WORD2HOUR(s->value + 9), s->value[11],
                     WORD2HOUR(s->value + 12), s->value[14], WORD2HOUR(s->value + 15), s->value[17], WORD2HOUR(s->value + 18), s->value[20],
                     WORD2HOUR(s->value + 21), s->value[23]);
      break;
    case QUERY_DURATIONS1:
      str += sprintf(str, "Working duration PH %u:%u SH %u:%u",
                     WORD2HOUR(s->value), s->value[2], WORD2HOUR(s->value + 3), s->value[5]);
      break;
    case QUERY_COUNTERS2:
      str += sprintf(str, "Start Counter PH %d SH %d other %d",
                     twobyte2word(s->value), twobyte2word(s->value + 2), twobyte2word(s->value + 4));
      break;
    case QUERY_STATUS2:
      // Level in percent. Too bad the % sign cant be printed reasonably on an 14 segment LCD. 
      str += sprintf(str, "Level GP:%d FP:%d Hz CF:%d %02x CP: %02x",
                     s->value[0] >> 1, s->value[1] / 20, s->value[2] >> 1,
                     s->value[3], s->value[4] >> 1);
      break;
    case QUERY_OPINFO1: \
      str += sprintf(str, "Low temp thres %d, High temp thres %d, Unknown 0x%x",
                     BYTE2TEMP(s->value[OP1_TLO]),
                     BYTE2TEMP(s->value[OP1_THI]),
                     s->value[OP1_U0]);
      break;
    case QUERY_DURATIONS2:
      str += sprintf(str, "Ventilation duration %d:%d",
                     WORD2HOUR(s->value), s->value[2]);
      break;
    case QUERY_FPW:
      str += sprintf(str, "FPW = ");
#if 0
      str += WORD2FPWR_TEXT(str, s->value);
      str += sprintf(str, "Ohm, %d Watt", WORD2WATT(s->value + 2));
#else
      str += sprintf(str, "%d 'C, %d Watt", WORD2WATT(s->value), WORD2WATT(s->value + 2));
#endif
      break;
    default:
      str += sprintf(str, "Sensor %d, value = ", s->idx); str = hexdump(str, s->value, s->length);
      break;
  }
}


*/
