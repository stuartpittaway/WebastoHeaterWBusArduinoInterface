/*
 * W-Bus constant definitions
 *
 * Author: Manuel Jander
 * mjander@users.sourceforge.net
 *
 */

/*
 * W-Bus addresses_
 * 0xf : thermo Test Software
 * 0x4 : heating device
 * 0x3 : 1533 Timer
 * 0x2 : Telestart
 */

/* address as client */
#define WBUS_CADDR 0xf
/* address as host */
#define WBUS_HADDR 0x4

/* W-Bus command refresh period in mili seconds */
#define CMD_REFRESH_PERIOD 20000L

/* W-Bus commands */
#define WBUS_CMD_OFF     0x10 /* no data */

#define WBUS_CMD_ON      0x20 /* For all CMD_ON_x : 1 byte = time in minutes */
#define WBUS_CMD_ON_PH   0x21 /* Parking Heating */
#define WBUS_CMD_ON_VENT 0x22 /* Ventilation */
#define WBUS_CMD_ON_SH   0x23 /* Supplemental Heating */
#define WBUS_CMD_CP      0x24 /* Circulation Pump external control */
#define WBUS_CMD_BOOST   0x25 /* Boost mode */

#define WBUS_CMD_SL_RD   0x32 /* Telestart system level read.  */
#define WBUS_CMD_X_RD    0x32 /* read something. 2 bytes 0x03 0x07, returns 0x0A */
#define WBUS_CMD_SL_WR	 0x33 /* Telestart system level write */ 
#define WBUS_CMD_U1      0x38 /* sent no bytes. answer 0B 00 00 00 00 03 BF */

/* Dataset commands */
#define WBUS_CMD_DS_START 0x47 /* 1 byte = 1 start bloc mode. Issued before WBUS_CMD_BAUD and CMD_DS_LSB */
#define WBUS_CMD_DS_R     0x03 /* 1 byte: address LSB. return 1 byte read data. */
#define WBUS_CMD_DS_W     0x04 /* 2 byte: address LSB and data for writing. */
#define WBUS_CMD_DS_MSB   0x05 /* 1 byte: address MSB. No returned data. For either read or write. */
#define WBUS_CMD_DS_STOP  0x09 /* 0 byte: stop block mode. */
#define WBUS_CMD_BAUD     0x34 /* Set block mode baudrate. 2 bytes. 
                                  0x80 0x00 38400 
                                  0x40 0x00 19200
                                  0x20 0x00  9600 */
/* After issuing WBUS_CMD_DS_START and WBUS_CMD_BAUD, the host sents 0x03 <addr> <checksum>
   and gets as response 0x03 <addr> <data> <checksum> */

/* discovered by pczepek, thank a lot ! eeprom[8] = int system_level */
#define WBUS_TS_ERD1     0x32 /* 2 bytes: 0x00 addr */
#define WBUS_TS_EWR1     0x33 /* 3 bytes: 0x00 addr data */
#define WBUS_TS_ERD      0x35 /* READ_EEPROM [address]. 
                                 Response length 2 byte. Reads two bytes from
                                 eeprom memory, from address given as parameter.
                                 The eeprom is accessible from address 0x00 to 0x1f.
                                 In eeprom dump I found customer ID Number,
                                 registered remote fob codes, telestart system level etc. */
#define WBUS_TS_EWR      0x36 /* WRITE_EEPROM [address],[byte1],[byte2]
                                 Write two bytes to eeprom. Address, byte1, byte2
                                 as parameter. */

#define WBUS_TS_REGR     0x40 /* discovered by pczepek, Response length 0 byte.
                                 Parameter 0-0x0f. After issuing command,
                                 press remote fob OFF button to register.
                                 You can register max 3 remote fob. */

#define WBUS_CMD_X	 0x42 /* Several commands */
#define   CMD_X_VCAL 1    /* Calibrate voltage. 3 bytes. 1 byte 0, 2 bytes voltage in mV, big endian. */
#define   CMD_X_FCAL 2    /* Flame detector calibration. 3 bytes. 1 byte 0, 2 bytes mOhm big endian. */
#define   CMD_X_FP   3    /* Fuel prime. data 0x03 <2 bytes time in seconds / 2> */

#define WBUS_CMD_CHK     0x44 /* Check current command (0x20,0x21,0x22 or 0x23) */
#define WBUS_CMD_TEST    0x45 /* <1 byte sub system id> <1 byte time in seconds> <2 bytes value> */
#define WBUS_CMD_QUERY   0x50 /* Read operational information registers */
#define WBUS_CMD_IDENT   0x51 /* Read device identification information registers */
#define WBUS_CMD_OPINFO  0x53 /* 1 byte, operational info index. */
#define WBUS_CMD_ERR     0x56 /* Error related commands */
#define WBUS_CMD_CO2CAL  0x57 /* CO2 calibration */

#define WBUS_CMD_DATASET 0x58 /* (Not Webasto) data set related commands */

/* 0x50 Command parameters */
/* Status flags. Bitmasks below. STAxy_desc means status "x", byte offset "y", flag called 2desc" */
#define QUERY_STATUS0	0x02
#define 	STA00_SHR	0x10 /*!< Supplemental heater request*/
#define		STA00_MS	0x01 /*!< Main switch */
#define		STA01_S		0x01 /*!< Summer season */
#define		STA02_D		0x10 /*!< Generator signal D+ */
#define		STA03_BOOST	0x10 /*!< boost mode */
#define		STA03_AD	0x01 /*!< auxiliary drive */
#define		STA04_T15	0x01 /*!< ignition (terminal 15) */

#define QUERY_STATUS1	0x03
#define		STA10_CF	0x01 /*!< Combustion Fan	*/
#define		STA10_GP	0x02 /*!< Glühkerze		*/
#define		STA10_FP	0x04 /*!< Fuel Pump             */
#define		STA10_CP	0x08 /*!< Circulation Pump	*/
#define		STA10_VF	0x10 /*!< Vehicle Fan Relay	*/
#define		STA10_NSH	0x20 /*!< Nozzle stock heating  */
#define		STA10_FI	0x40 /*!< Flame indicator       */
       
#define QUERY_OPINFO0		0x04 /* Fuel type, max heat time and factor for shortening ventilation time (but details are unclear) */
#define         OP0_FUEL        0      /*!< 0x0b: gasoline, 0x0d: diesel, 0:neutral */
#define         OP0_TIME        1      /*!< max heating time / 10 in minutes */
#define         OP0_FACT        2      /*!< ventilation shortening factor (?) */

#define QUERY_SENSORS	0x05 /*!< Assorted sensors. 8 bytes. Byte offsets below. */
#define		SEN_TEMP	0	/*!< Temperature with 50�C offset (20�C is represented by 70) */
#define		SEN_VOLT	1	/*!< 2 bytes Spannung in mili Volt, big endian */
#define		SEN_FD		3	/*!< 1 byte Flame detector flag */
#define		SEN_HE		4	/*!< 2 byte, heating power, in percent or watts (semantic seems wrong, heating energy?) */
#define		SEN_GPR		6	/*!< 2 byte, glow plug resistance in mili Ohm. */

#define QUERY_COUNTERS1	0x06
#define		WORK_HOURS	0 /*!< Working hours     */
#define		WORK_MIN	2 /*!< Working minutes   */
#define		OP_HOURS	3 /*!< Operating hours   */
#define		OP_MIN		5 /*!< Operating minutes */
#define		CNT_START	6 /*!< Start counter     */

#define QUERY_STATE	0x07
#define		OP_STATE	0
#define		OP_STATE_N	1
#define		DEV_STATE	2

/* 3 more unknown bytes */
#define WB_STATE_BO	0x00 /* Burn out */
#define	WB_STATE_DEACT1	0x01 /* Deactivation */
#define	WB_STATE_BOADR	0x02 /* Burn out ADR (has something to due with hazardous substances transportation) */
#define	WB_STATE_BORAMP	0x03 /* Burn out Ramp */
#define	WB_STATE_OFF	0x04 /* Off state */
#define	WB_STATE_CPL	0x05 /* Combustion process part load */
#define	WB_STATE_CFL	0x06 /* Combustion process full load */
#define	WB_STATE_FS	0x07 /* Fuel supply */
#define	WB_STATE_CAFS	0x08 /* Combustion air fan start */
#define	WB_STATE_FSI	0x09 /* Fuel supply interruption */
#define	WB_STATE_DIAG	0x0a /* Diagnostic state */
#define	WB_STATE_FPI	0x0b /* Fuel pump interruption */
#define	WB_STATE_EMF	0x0c /* EMF measurement */
#define	WB_STATE_DEB	0x0d /* Debounce */
#define	WB_STATE_DEACTE	0x0e /* Deactivation */
#define	WB_STATE_FDI	0x0f /* Flame detector interrogation */
#define	WB_STATE_FDC	0x10 /* Flame detector cooling */
#define	WB_STATE_FDM	0x11 /* Flame detector measuring phase */
#define	WB_STATE_FDMZ	0x12 /* Flame detector measuring phase ZUE */
#define	WB_STATE_FAN	0x13 /* Fan start up */
#define	WB_STATE_GPRAMP	0x14 /* Glow plug ramp */
#define	WB_STATE_LOCK	0x15 /* Heater interlock */
#define	WB_STATE_INIT	0x16 /* Initialization   */
#define	WB_STATE_BUBLE	0x17 /* Fuel bubble compensation */
#define	WB_STATE_FANC	0x18 /* Fan cold start-up */
#define	WB_STATE_COLDR	0x19 /* Cold start enrichment */
#define	WB_STATE_COOL	0x1a /* Cooling */
#define	WB_STATE_LCHGUP	0x1b /* Load change PL-FL */
#define	WB_STATE_VENT	0x1c /* Ventilation */
#define	WB_STATE_LCHGDN	0x1d /* Load change FL-PL */
#define	WB_STATE_NINIT	0x1e /* New initialization */
#define	WB_STATE_CTRL	0x1f /* Controlled operation */
#define	WB_STATE_CIDDLE	0x20 /* Control iddle period */
#define	WB_STATE_SSTART	0x21 /* Soft start */
#define	WB_STATE_STIME	0x22 /* Savety time */
#define	WB_STATE_PURGE	0x23 /* Purge */
#define	WB_STATE_START	0x24 /* Start */
#define	WB_STATE_STAB	0x25 /* Stabilization */
#define	WB_STATE_SRAMP	0x26 /* Start ramp    */
#define	WB_STATE_OOP	0x27 /* Out of power  */
#define	WB_STATE_LOCK2	0x28 /* Interlock     */
#define WB_STATE_LOCKADR	0x29 /* Interlock ADR (Australian design rules) */
#define	WB_STATE_STABT	0x2a /* Stabilization time */
#define	WB_STATE_CHGCTRL	0x2b /* Change to controlled operation */
#define	WB_STATE_DECIS	0x2c /* Decision state */
#define	WB_STATE_PSFS	0x2d /* Prestart fuel supply */
#define	WB_STATE_GLOW	0x2e /* Glowing */
#define	WB_STATE_GLOWP	0x2f /* Glowing power control */
#define	WB_STATE_DELAY	0x30 /* Delay lowering */
#define	WB_STATE_SLUG	0x31 /* Sluggish fan start */
#define	WB_STATE_AGLOW	0x32 /* Additional glowing */
#define	WB_STATE_IGNI	0x33 /* Ignition interruption */
#define	WB_STATE_IGN	0x34 /* Ignition */
#define	WB_STATE_IGNII	0x35 /* Intermittent glowing */
#define	WB_STATE_APMON	0x36 /* Application monitoring */
#define	WB_STATE_LOCKS	0x37 /* Interlock save to memory */
#define	WB_STATE_LOCKD	0x38 /* Heater interlock deactivation */
#define	WB_STATE_OUTCTL	0x39 /* Output control */
#define	WB_STATE_CPCTL	0x3a /* Circulating pump control */
#define	WB_STATE_INITUC	0x3b /* Initialization uP */
#define	WB_STATE_SLINT	0x3c /* Stray light interrogation */
#define	WB_STATE_PRES	0x3d /* Prestart */
#define	WB_STATE_PREIGN	0x3e /* Pre-ignition */
#define	WB_STATE_FIGN	0x3f /* Flame ignition */
#define	WB_STATE_FSTAB	0x40 /* Flame stabilization */
#define	WB_STATE_PH	0x41 /* Combustion process parking heating */
#define	WB_STATE_SH	0x42 /* Combustion process suppl. heating  */
#define	WB_STATE_PHFAIL	0x43 /* Combustion failure failure heating */
#define	WB_STATE_SHFAIL	0x44 /* Combustion failure suppl. heating  */
#define	WB_STATE_OFFR	0x45 /* Heater off after run */
#define	WB_STATE_CID	0x46 /* Control iddle after run */
#define	WB_STATE_ARFAIL	0x47 /* After-run due to failure */
#define	WB_STATE_ARTCTL	0x48 /* Time-controlled after-run due to failure */
#define	WB_STATE_LOCKCP	0x49 /* Interlock circulation pump */
#define	WB_STATE_CIDPH	0x4a /* Control iddle after parking heating */
#define	WB_STATE_CIDSH	0x4b /* Control iddle after suppl. heating  */
#define	WB_STATE_CIDHCP	0x4c /* Control iddle period suppl. heating with circulation pump */
#define	WB_STATE_CPNOH	0x4d /* Circulation pump without heating function */
#define	WB_STATE_OV	0x4e /* Waiting loop overvoltage */
#define	WB_STATE_MFAULT	0x4f /* Fault memory update */
#define	WB_STATE_WLOOP	0x50 /* Waiting loop */
#define	WB_STATE_CTEST	0x51 /* Component test */
#define	WB_STATE_BOOST	0x52 /* Boost */
#define	WB_STATE_COOL2	0x53 /* Cooling */
#define	WB_STATE_LOCKP	0x54 /* Heater interlock permanent */
#define	WB_STATE_FANIDL	0x55 /* Fan iddle */
#define	WB_STATE_BA	0x56 /* Break away */
#define	WB_STATE_TINT	0x57 /* Temperature interrogation */
#define	WB_STATE_PREUV	0x58 /* Prestart undervoltage */
#define	WB_STATE_AINT	0x59 /* Accident interrogation */
#define	WB_STATE_ARSV	0x5a /* After-run solenoid valve */
#define	WB_STATE_MFLTSV	0x5b /* Fault memory update solenoid valve */
#define	WB_STATE_TCARSV	0x5c /* Timer-controlled after-run solenoid valve */
#define	WB_STATE_SA	0x5d /* Startup attempt */
#define	WB_STATE_PREEXT	0x5e /* Prestart extension */
#define	WB_STATE_COMBP	0x5f /* Combustion process */
#define	WB_STATE_TIARUV	0x60 /* Timer-controlled after-run due to undervoltage */
#define	WB_STATE_MFLTSW	0x61 /* Fault memory update prior switch off */
#define	WB_STATE_RAMPFL	0x62 /* Ramp full load */
    /*byte1 Operating state state number*/
    /*byte2 Device state*/
#define	WB_DSTATE_STFL	0x01 /* STFL */
#define	WB_DSTATE_UEHFL	0x02 /* UEHFL */
#define	WB_DSTATE_SAFL	0x04 /* SAFL   */
#define	WB_DSTATE_RZFL	0x08 /* RZFL */
    /*byte3,4,5: Unknown*/

#define QUERY_DURATIONS0 0x0a /* 24 bytes */

#define QUERY_DURATIONS1 0x0b /* 6 bytes*/
#define		DUR1_PH	0 /* Parking heating duration, hh:m */
#define		DUR1_SH 3 /* Supplemental heating duration hh:m */

#define QUERY_COUNTERS2	0x0c
#define		STA3_SCPH	0	/*!< 2 bytes, parking heater start counter  */
#define		STA3_SCSH	2	/*!< 2 bytes, supplemtal heater start counter */
#define		STA34_FD	0x00	/*!< Flame detected  */

#define QUERY_STATUS2	0x0f
#define		STA2_GP		0 /* glow plug (ignition/flame detection)*/
#define		STA2_FP		1 /* fuel pump */
#define		STA2_CAF	2 /* combustion air fan */
#define		STA2_U0		3 /* unknown */
#define		STA2_CP		4 /* (coolant) circulation pump */

#define QUERY_OPINFO1 0x11
#define		OP1_THI 0 /* Lower temperature threshold */
#define		OP1_TLO 1 /* Higher temperature threshold */
#define         OP1_U0  2

#define QUERY_DURATIONS2 0x12 /* 3 bytes */
#define		DUR2_VENT 0 /* Ventilation duration hh:m */
                    
#define	QUERY_FPW	0x13 /*!< Fuel prewarming. May not be available. See wbcode */
#define		FPW_R		0	/*!< 2 bytes: Current fuel prewarming PTC resistance in mili ohm, big endian */
#define		FPW_P		2	/*!< 2 bytes: Currently applied fuel prewarming power in watts, big endian */


/* 0x51 Command parameters */
#define IDENT_DEV_ID		0x01 /*!< Device ID Number */
#define IDENT_HWSW_VER		0x02 /*!< Hardware version (KW/Jahr), Software version, Software version EEPROM, 6 bytes */
#define IDENT_DATA_SET		0x03 /*!< Data Set ID Number */
#define IDENT_DOM_CU		0x04 /*!< Control Unit Herstellungsdatum (Tag monat jahr je ein byte) */
#define IDENT_DOM_HT		0x05 /*!< Heizer Herstellungsdatum (Tag monat jahr je ein byte) */
#define IDENT_TSCODE		0x06 /*!< Telestart code */
#define IDENT_CUSTID		0x07 /*!< Customer ID Number (Die VW Teilenummer als string und noch ein paar Nummern dran) + test sig */
#define IDENT_U0                0x08 /*!< ? */
#define IDENT_SERIAL		0x09 /*!< 5 bytes: Serial Number. 2 bytes Test signature. */
#define IDENT_WB_VER		0x0a /*!< W-BUS version. Antwort ergibt ein byte. Jedes nibble dieses byte entspricht einer Zahl (Zahl1.Zahl2) */
#define IDENT_DEV_NAME		0x0b /*!< Device Name: Als character string zu interpretieren. */
#define IDENT_WB_CODE		0x0c /*!< W-BUS code. 7 bytes. This is sort of a capability bit field */


/* W-Bus code bits */
#define     WB_CODE_0     0 /* Unknown supplemental heater feature */
#define     WB_CODE_ON    3 /* on/off switch capability */
#define     WB_CODE_PH    4 /* Parking heater capability */
#define     WB_CODE_SH    5 /* Supplemental heater capability */
#define     WB_CODE_VENT  6 /* Ventilation capability */
#define     WB_CODE_BOOST 7 /* Boost capability */

#define     WB_CODE_ECPC  9 /* External circulation pump control */
#define     WB_CODE_CAV  10 /* Combustion air fan (CAV) */
#define     WB_CODE_GP   11 /* Glow Plug (flame detector) */
#define     WB_CODE_FP   12 /* Fuel pump (FP) */
#define     WB_CODE_CP   13 /* Circulation pump (CP) */
#define     WB_CODE_VFR  14 /* Vehicle fan relay (VFR) */
#define     WB_CODE_LEDY 15 /* Yellow LED */

#define     WB_CODE_LEDG 16 /* Green LED present */
#define     WB_CODE_ST   17 /* Spark transmitter. Implies no Glow plug and thus no resistive flame detection */
#define     WB_CODE_SV   18 /* Solenoid valve present (coolant circuit switching) */
#define     WB_CODE_DI   19 /* Auxiliary drive indicator (whatever that means) */
#define     WB_CODE_D    20 /* Generator signal D+ present */
#define     WB_CODE_CAVR 21 /* Combustion air fan level is in RPM instead of percent */
#define     WB_CODE_22   22 /* (ZH) */
#define     WB_CODE_23   23 /* (ZH) */

#define     WB_CODE_CO2  25 /* CO2 calibration */
#define     WB_CODE_OI   27 /* Operation indicator (OI) */

#define     WB_CODE_32   32 /* (ZH) */
#define     WB_CODE_33   33 /* (ZH) */
#define     WB_CODE_34   34 /* (ZH) */
#define     WB_CODE_35   35 /* (ZH) */
#define     WB_CODE_HEW  36 /* Heating energy is in watts (or if not set in percent and the value field must be divided
                               by 2 to get the percent value) */
#define     WB_CODE_37   37 /* (ZH) */
#define     WB_CODE_FI   38 /* Flame indicator (FI) */
#define     WB_CODE_NSH  39 /* Nozzle Stock heating (NSH) */

#define     WB_CODE_T15  45 /* Ignition (T15) flag present */
#define     WB_CODE_TTH  46 /* Temperature thresholds available, command 0x50 index 0x11 */
#define     WB_CODE_VPWR 47 /* Fuel prewarming resistance and power can be read. */

#define     WB_CODE_SET  57 /* 0x02 Set value flame detector resistance (FW-SET),
                               set value combustion air fan revolutions
                               (BG-SET), set value output temperature (AT-SET) */

#define IDENT_SW_ID		0x0d /*!< Software ID */

/* Dataset commands are custom and proprietary to this library */
#define DATASET_COUNT	0x01 /* Amount of data set entries. */
#define DATASET_READ    0x02 /* Read given data set entry. */
#define DATASET_WRITE   0x03 /* Write given data set entry. */

/* 053 operational info indexes */
#define OPINFO_LIMITS 02
/* 
  data format:
 1 byte:  no idea
 2 bytes: Minimum voltage threshold in milivolts
 4 bytes: no idea
 1 byte:  minimum voltage detection delay (seconds)
 2 bytes: maximum voltage threshold in milivolts
 4 bytes: no idea
 1 byte:  maximum voltage detection delay (seconds
 */

/* 0x56 Error code operand 0 */
#define ERR_LIST   1 /* send not data. answer is n, code0, counter0-1, code1, counter1-1 ... coden, countern-1 */
#define ERR_READ   2 /* send code. answer code, flags, counter ... (err_info_t) */
#define ERR_DEL    3 /* send no data. answer also no data. */
#define ERR_TS_LIST 5 /* Telestart error list. Return 5 bytes: n, code0, counter0-1, code1, counter1-1 */

#define CO2CAL_READ	1 /* 3 data bytes: current value, min value, max value. */
#define CO2CAL_WRITE_C  2 /* 2 data byte: new current value for cold CO2 cal. */
#define CO2CAL_WRITE	3 /* 1 data byte: new current value for hot CO2 cal. */

/* Component test device definitions */
#define WBUS_TEST_CF 1 /*!< Combustion Fan */
#define WBUS_TEST_FP 2 /*!< Fuel Pump */
#define WBUS_TEST_GP 3 /*!< Glow Plug */
#define WBUS_TEST_CP 4 /*!< Circulation Pump */
#define WBUS_TEST_VF 5 /*!< Vehicle Fan Relays */
#define WBUS_TEST_SV 9 /*!< Solenoid Valve */
#define WBUS_TEST_NSH 13 /*!< Nozzel Stock Heating */
#define WBUS_TEXT_NAC 14 /*!< Nozzle air compressor (not standart, POELI specific) */
#define WBUS_TEST_FPW 15 /*!< Fuel Prewarming */

