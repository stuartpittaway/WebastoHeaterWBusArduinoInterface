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
*/

#define PROGMEMLABEL(label,value)  const char PROGMEM label[] = value; 

//Version info page
PROGMEMLABEL(label_WBUSVer,"WBus ver:")
PROGMEMLABEL(label_WBusCode,"\nWBus code:")
PROGMEMLABEL(label_DatasetIDNo,"\nDataset id no:")
PROGMEMLABEL(label_SoftwareIDNo,"SW id no:")
PROGMEMLABEL(label_DateOfManufactureControlUnit,"\nDate ctrl unit:")
PROGMEMLABEL(label_HWVersion,"HW ver:")
PROGMEMLABEL(label_SoftwareVersion,"SW ver:")
PROGMEMLABEL(label_SoftwareVersionEEPROM,"\nSW ver EEPROM:")

//Heater info page
PROGMEMLABEL(label_DeviceName,"Name:")
PROGMEMLABEL(label_DeviceIDNo,"\nDevice id no:")
PROGMEMLABEL(label_DateOfManufactureHeater,"\nDate heater:")
PROGMEMLABEL(label_CustomerIdentificationNumber,"\nCust id num:")
PROGMEMLABEL(label_SerialNumber,"\nSerial num:")

PROGMEMLABEL(label_Unknown,"? ")
PROGMEMLABEL(label_Mon,"Mon ")
PROGMEMLABEL(label_Tue,"Tue ")
PROGMEMLABEL(label_Wed,"Wed ")
PROGMEMLABEL(label_Thu,"Thu ")
PROGMEMLABEL(label_Fri,"Fri ")
PROGMEMLABEL(label_Sat,"Sat ")

PROGMEMLABEL(label_Sun,"Sun ")

const char* const weekdays_table[] PROGMEM = {label_Unknown, label_Mon, label_Tue, label_Wed, label_Thu, label_Fri,label_Sat  };

PROGMEMLABEL(label_ErrorCount,"Error ")

/* Fault screen */
PROGMEMLABEL(label_Code,"Code:")
PROGMEMLABEL(label_Flag,"\nFlag:")
PROGMEMLABEL(label_Counter,"\nCounts:")
PROGMEMLABEL(label_OperatingState,"\nOp state:")
PROGMEMLABEL(label_Temperature,"Temp:")
PROGMEMLABEL(label_SupplyVoltage," C\nSupply V:")
PROGMEMLABEL(label_OperatingTime,"\nOp time:")


//PROGMEMLABEL(label_getVersion,"%02x/%02x %02x.%02x\n")

PROGMEMLABEL(label_wbusError,"WBUS err 0x")

PROGMEMLABEL(label_NoFaultsFound,"No faults found")
PROGMEMLABEL(label_FaultsCleared,"All faults cleared")
PROGMEMLABEL(label_NoMoreFaultsFound,"No more faults")


  PROGMEMLABEL(label_OP, "Op:0x")
  PROGMEMLABEL(label_N, " N:")
  PROGMEMLABEL(label_Dev, " Dev:0x")


