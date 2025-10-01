/* 
  Data arranged in EEPROM as follows

  Parameter            Offset
  ----------------------------
  FILE_SIGNATURE       0   (2 bytes)
  INIT_FLAG_SYS        2   (1 byte)
  Sys struct data      3   (300 bytes max allocation)

*/

#include "Arduino.h"
#include <EEPROM.h>

#include "config.h"
#include "common.h"
#include "eestore.h"
#include "crc.h"

#define FILE_SIGNATURE  0xBDAC

#define ADR_INT_EE_FILE_SIGNATURE   0
#define ADR_INT_EE_INIT_FLAG_SYS    2
#define ADR_INT_EE_SYS_DATA_START   3

//--------------------------------------------------------------------------------------------------

void eeStoreInit()
{
  uint8_t  eeInitFlagSys = crc8((uint8_t *) &Sys, sizeof(Sys));
  uint16_t fileSignature;
  
  //Check signature and flag
  EEPROM.get(ADR_INT_EE_FILE_SIGNATURE, fileSignature);
  if((fileSignature != FILE_SIGNATURE) || (EEPROM.read(ADR_INT_EE_INIT_FLAG_SYS) != eeInitFlagSys)) 
  {
    //clear 
    for(uint16_t i = ADR_INT_EE_SYS_DATA_START; i < ADR_INT_EE_SYS_DATA_START + 300; i++)
      EEPROM.update(i, 0xFF);
    //write system data
    eeSaveSysConfig(); 
    //write flag
    EEPROM.write(ADR_INT_EE_INIT_FLAG_SYS, eeInitFlagSys);
    //write signature
    fileSignature = FILE_SIGNATURE;
    EEPROM.put(ADR_INT_EE_FILE_SIGNATURE, fileSignature);
  }
}

//--------------------------------------------------------------------------------------------------

void eeReadSysConfig()
{
  EEPROM.get(ADR_INT_EE_SYS_DATA_START, Sys);
}

//--------------------------------------------------------------------------------------------------

void eeSaveSysConfig()
{
  EEPROM.put(ADR_INT_EE_SYS_DATA_START, Sys);
}
