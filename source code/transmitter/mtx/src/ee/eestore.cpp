/* 
  Data arranged in eeprom as follows
  
  ======== INTERNAL EEPROM ===========
  Parameter            Offset
  ----------------------------
  FILE_SIGNATURE       0   (2 bytes)
  INIT_FLAG_SYS        2   (1 byte)
  Sys struct data      3   (1024 bytes max allocation)
  INIT_FLAG_MODEL      1027 (1 byte)
  Model struct data    1028 (length and offsets automatically determined)

  ======== EXTERNAL EEPROM ===========
  Parameter            Offset
  ----------------------------
  FILE_SIGNATURE       0    (2 bytes)
  INIT_FLAG_MODEL      2    (1 byte)
  Model struct data    3    (length and offsets automatically determined)
*/

#include "Arduino.h"
#include <EEPROM.h>
#include <Wire.h>

#include "../mtx.h"
#include "../ui/ui.h" 
#include "../templates.h"

#include "External_EEPROM.h" 
ExternalEEPROM myMem;

#define FILE_SIGNATURE  0xBDAC

#define ADDRESS_INT_EE_FILE_SIGNATURE   0
#define ADDRESS_INT_EE_INIT_FLAG_SYS    2
#define ADDRESS_INT_EE_SYS_DATA_START   3
#define ADDRESS_INT_EE_INIT_FLAG_MODEL  1027
#define ADDRESS_INT_EE_MODEL_DATA_START 1028

#define ADDRESS_EXT_EE_FILE_SIGNATURE   0
#define ADDRESS_EXT_EE_INIT_FLAG_MODEL  2
#define ADDRESS_EXT_EE_MODEL_DATA_START 3

uint8_t maxModelsInternal = 1; //Computed at runtime. Just an initial value here
uint8_t maxModelsExternal = 0; //Computed at runtime. Just an initial value here
bool hasExternalEE = false;

bool isInternalEE(uint8_t modelIdx);
uint16_t getModelAddressInternalEE(uint8_t modelIdx);
uint16_t getModelAddressExternalEE(uint8_t modelIdx);
void checkAndFormatEEPROM();

//--------------------------------------------------------------------------------------------------

void eeStoreInit()
{
  maxModelsInternal = (EEPROM.length() - ADDRESS_INT_EE_MODEL_DATA_START) / sizeof(Model);
  
  //External eeprom
  Wire.begin();
  Wire.setClock(400000);
  delay(10); 
  if(myMem.begin(0x50 | EXTERNAL_EEPROM_DEVICE_ADDRESS))
  {
    hasExternalEE = true;
    myMem.setMemorySize(EXTERNAL_EEPROM_TOTAL_BYTES); 
    myMem.setPageSize(EXTERNAL_EEPROM_PAGE_BYTES);
    myMem.enablePollForWriteComplete(); //Supports I2C polling of write completion
    
    maxModelsExternal = (myMem.length() - ADDRESS_EXT_EE_MODEL_DATA_START) / sizeof(Model);
  }
  
  maxNumOfModels = maxModelsInternal + maxModelsExternal;
  
  //--- Check if formatting neccessary ---
  checkAndFormatEEPROM();
  
  //--- Find active model, if none exists, create a model in slot1 ---
  
  eeReadSysConfig();
  
  //Start by searching external eeprom
  if(hasExternalEE && !isInternalEE(Sys.activeModelIdx) && eeModelIsFree(Sys.activeModelIdx))
  {
    //search from start
    uint8_t modelIdx = maxModelsInternal;
    while(1)
    {
      if(!eeModelIsFree(modelIdx)) 
      {
        //found an existing model, assign it to active and exit 
        Sys.activeModelIdx = modelIdx;
        break;
      }
      
      modelIdx++;
      if(modelIdx == maxNumOfModels) 
      {
        //no existing model found, trigger searching in internal eeprom
        Sys.activeModelIdx = 0;
        break;
      }
    }
  }
  
  //search internal eeprom
  if((isInternalEE(Sys.activeModelIdx) && eeModelIsFree(Sys.activeModelIdx)) 
    || (!hasExternalEE && !isInternalEE(Sys.activeModelIdx)))
  {
    //search from start
    uint8_t modelIdx = 0;
    while(1)
    {
      if(!eeModelIsFree(modelIdx)) 
      {
        //found an existing model, assign it to active and exit 
        Sys.activeModelIdx = modelIdx;
        break;
      }
      
      modelIdx++;
      if(modelIdx == maxModelsInternal) 
      {
        //no existing model found, create a model, set it active and load the basic mixer template
        eeCreateModel(0);
        Sys.activeModelIdx = 0;
        Model.type = MODEL_TYPE_AIRPLANE;
        if(Sys.mixerTemplatesEnabled)
          loadMixerTemplateBasic(0);
        eeSaveModelData(Sys.activeModelIdx);
        break;
      }
    }
  }
  
  //save sys
  eeSaveSysConfig();
}

//--------------------------------------------------------------------------------------------------

void checkAndFormatEEPROM()
{
  uint8_t eeInitFlagSys   = crc8((uint8_t *) &Sys, sizeof(Sys));
  uint8_t eeInitFlagModel = crc8((uint8_t *) &Model, sizeof(Model));
  uint16_t fileSignature;
  
  bool clearEESysData   = false;
  bool clearEEModelData = false;
  
  ///--------------- Internal eeprom --------------
  
  //Check signature. If its not matching, then assume it's a fresh mcu and format the eeprom
  EEPROM.get(ADDRESS_INT_EE_FILE_SIGNATURE, fileSignature);
  if(fileSignature != FILE_SIGNATURE) //force format
  {
    turnOnBacklight();
    showMsg(PSTR("Internal EEPROM\nnot formatted.\nPress any key"));
    while(buttonCode == 0)
    {
      delay(30);
      readSwitchesAndButtons();
      handlePowerOff();
    }
    buttonCode = 0; 
    clearEESysData = true;
    clearEEModelData = true;
  }
  else //Check flags. Signature may match but not the data structs
  {
    if(EEPROM.read(ADDRESS_INT_EE_INIT_FLAG_SYS) != eeInitFlagSys)
    {
      turnOnBacklight();
      showMsg(PSTR("System data\npossibly corrupt.\nReset it?\n\nYes [Up] \nNo [Down]"));
      while(buttonCode != KEY_UP && buttonCode != KEY_DOWN)
      {
        delay(30);
        readSwitchesAndButtons();
        handlePowerOff();
      }
      if(buttonCode == KEY_UP) 
        clearEESysData = true;
      else if(buttonCode == KEY_DOWN) 
        EEPROM.write(ADDRESS_INT_EE_INIT_FLAG_SYS, eeInitFlagSys);
      buttonCode = 0;
    }
    
    do {
      delay(30);
      readSwitchesAndButtons();
      handlePowerOff();
    } while(buttonCode != 0);  //wait for button release to prevent false trigger
    
    if(EEPROM.read(ADDRESS_INT_EE_INIT_FLAG_MODEL) != eeInitFlagModel)
    {
      turnOnBacklight();
      showMsg(PSTR("Model data in\nInternal EEPROM\npossibly corrupt.\nReset it?\n\nYes [Up] \nNo [Down]"));
      while(buttonCode != KEY_UP && buttonCode != KEY_DOWN)
      {
        delay(30);
        readSwitchesAndButtons();
        handlePowerOff();
      }
      if(buttonCode == KEY_UP) 
        clearEEModelData = true;
      else if(buttonCode == KEY_DOWN) 
        EEPROM.write(ADDRESS_INT_EE_INIT_FLAG_MODEL, eeInitFlagModel);
      buttonCode = 0;
    }
  }

  if(clearEESysData)
  {
    //show msg
    turnOnBacklight();
    showMsg(PSTR("Preparing storage"));
    delay(2000);
    //clear (unnecessary)
    //write system data
    eeSaveSysConfig(); 
    //write flag
    EEPROM.write(ADDRESS_INT_EE_INIT_FLAG_SYS, eeInitFlagSys);
    //write signature
    fileSignature = FILE_SIGNATURE;
    EEPROM.put(ADDRESS_INT_EE_FILE_SIGNATURE, fileSignature);
    //trigger initial setup
    startInitialSetup();
  }
  
  if(clearEEModelData)
  {
    //show msg
    turnOnBacklight();
    showMsg(PSTR("Preparing storage"));
    delay(2000);
    //clear
    /* 
    for(uint16_t i = ADDRESS_INT_EE_MODEL_DATA_START; i < EEPROM.length(); i++)
      EEPROM.update(i, 0xFF); 
    */
    //much faster way, though doesnt actually clear everything out
    for(uint8_t modelIdx = 0; modelIdx < maxModelsInternal; modelIdx++)
      eeDeleteModel(modelIdx);
    //write flag
    EEPROM.write(ADDRESS_INT_EE_INIT_FLAG_MODEL, eeInitFlagModel);
  }
  
  ///------------------ External eeprom -------------------
  
  do{
    delay(30);
    readSwitchesAndButtons();
    handlePowerOff();
  } while(buttonCode != 0);  //wait for button release to prevent false trigger
  
  clearEEModelData = false;
  
  if(hasExternalEE)
  {
    //Check signature. If its not matching, then assume it's a fresh eeprom
    myMem.get(ADDRESS_EXT_EE_FILE_SIGNATURE, fileSignature);
    if(fileSignature != FILE_SIGNATURE) //force format
    {
      turnOnBacklight();
      showMsg(PSTR("External EEPROM\nnot formatted.\nPress any key"));
      while(buttonCode == 0)
      {
        delay(30);
        readSwitchesAndButtons();
        handlePowerOff();
      }
      buttonCode = 0; 
      clearEEModelData = true;
    }
    else //Check flag. Signature may match but not the data structs
    {
      if(myMem.read(ADDRESS_EXT_EE_INIT_FLAG_MODEL) != eeInitFlagModel)
      {
        turnOnBacklight();
        showMsg(PSTR("Model data in\nExternal EEPROM\npossibly corrupt.\nReset it?\n\nYes [Up] \nNo [Down]"));
        while(buttonCode != KEY_UP && buttonCode != KEY_DOWN)
        {
          delay(30);
          readSwitchesAndButtons();
          handlePowerOff();
        }
        if(buttonCode == KEY_UP) //format
          clearEEModelData = true;
        else if(buttonCode == KEY_DOWN)
          myMem.write(ADDRESS_EXT_EE_INIT_FLAG_MODEL, eeInitFlagModel);
        buttonCode = 0;
      }
    }
    
    if(clearEEModelData)
    {
      turnOnBacklight();
      showMsg(PSTR("Preparing storage"));
      delay(2000);
      //clear
      /* 
      for(uint16_t i = ADDRESS_EXT_EE_MODEL_DATA_START; i < myMem.length(); i++)
        myMem.write(i, 0xFF); 
      */
      //much faster way, though doesnt actually clear everything out
      for(uint8_t modelIdx = maxModelsInternal; modelIdx < maxNumOfModels; modelIdx++)
        eeDeleteModel(modelIdx);
      //write flag
      myMem.write(ADDRESS_EXT_EE_INIT_FLAG_MODEL, eeInitFlagModel); 
      //write signature
      fileSignature = FILE_SIGNATURE;
      myMem.put(ADDRESS_EXT_EE_FILE_SIGNATURE, fileSignature);
    }
  }
}

//--------------------------------------------------------------------------------------------------

void eeReadSysConfig()
{
  EEPROM.get(ADDRESS_INT_EE_SYS_DATA_START, Sys);
  
  /* 
    Verify settings. Here we check a few crucial parameters.
    If the settings are corrupt, load default settings.
  */
  bool isSane = true;

  if(Sys.backlightWakeup >= BACKLIGHT_WAKEUP_COUNT) isSane = false;
  if(Sys.backlightTimeout >= BACKLIGHT_TIMEOUT_COUNT) isSane = false;
  if(Sys.rfPower >= RF_POWER_COUNT) isSane = false;
  if(Sys.defaultStickMode >= STICK_MODE_COUNT) isSane = false;
  
  if(!isSane)
  {
    turnOnBacklight();
    showMsg(PSTR("Bad system data.\nLoading defaults"));
    delay(2000);
    resetSystemParams();
    eeSaveSysConfig();
    
    //trigger initial setup
    startInitialSetup();
  }
}

//--------------------------------------------------------------------------------------------------

void eeSaveSysConfig()
{
  EEPROM.put(ADDRESS_INT_EE_SYS_DATA_START, Sys);
}

//--------------------------------------------------------------------------------------------------

void eeReadModelData(uint8_t modelIdx)
{
  if(isInternalEE(modelIdx))
    EEPROM.get(getModelAddressInternalEE(modelIdx), Model);
  else if(hasExternalEE)
    myMem.get(getModelAddressExternalEE(modelIdx), Model);
  
  if(!verifyModelData())
  {
    turnOnBacklight();
    showMsg(PSTR("Bad model data.\nLoading defaults"));
    delay(2000);
    resetModelName();
    resetModelParams();
  }
}

//--------------------------------------------------------------------------------------------------

void eeSaveModelData(uint8_t modelIdx)
{ 
  if(isInternalEE(modelIdx))
    EEPROM.put(getModelAddressInternalEE(modelIdx), Model);
  else if(hasExternalEE)
    myMem.put(getModelAddressExternalEE(modelIdx), Model);
}

//--------------------------------------------------------------------------------------------------

void eeLazyWriteModelData(uint8_t modelIdx)
{
  //This function writes model data one byte at a time so as to improve system perfomance
  static bool lazyWriteInitialized = false;
  static uint8_t lastModelIdx = 0xff;
  
  if(modelIdx != lastModelIdx)
  {
    lastModelIdx = modelIdx;
    lazyWriteInitialized = false;
  }
  
  static uint16_t address = 0;
  static uint16_t finalAddress = 0;
  
  if(!lazyWriteInitialized) //reset address counter
  {
    if(isInternalEE(modelIdx))
      address = getModelAddressInternalEE(modelIdx);
    else if(hasExternalEE)
      address = getModelAddressExternalEE(modelIdx);
    finalAddress = address + sizeof(Model) - 1;
    lazyWriteInitialized = true;
  }
  
  if(lazyWriteInitialized)
  {
    if(isInternalEE(modelIdx))
    {
      uint8_t* ptr = (uint8_t*) &Model;
      uint16_t i = address - getModelAddressInternalEE(modelIdx);
      EEPROM.update(address, *(ptr + i)); //address, value
    }
    else if(hasExternalEE)
    {
      uint8_t* ptr = (uint8_t*) &Model;
      uint16_t i = address - getModelAddressExternalEE(modelIdx);
      myMem.write(address, *(ptr + i)); //address, value
    }
    //increment address
    address++;
    if(address > finalAddress) //all writes completed, start over
      lazyWriteInitialized = false;
  }
}

//--------------------------------------------------------------------------------------------------

bool eeModelIsFree(uint8_t modelIdx)
{
  char nameBuff[sizeof(Model.name)];
  eeGetModelName(nameBuff, modelIdx, sizeof(nameBuff));
  
  //if all characters in name are 0xFF, then model slot is free
  for(uint8_t i = 0; i < (sizeof(Model.name) - 1); i++)
  {
    if((uint8_t) *(nameBuff + i) != 0xFF)
      return false;
  }
  return true;
}

//--------------------------------------------------------------------------------------------------

void eeGetModelName(char* buff, uint8_t modelIdx, uint8_t lenBuff)
{
  if(isInternalEE(modelIdx))
  {
    uint16_t address = getModelAddressInternalEE(modelIdx);
    for(uint8_t i = 0; i < sizeof(Model.name) && i < lenBuff - 1; i++) 
    {
      *(buff + i) = EEPROM.read(address + i);
    }
  }
  else if(hasExternalEE)
  {
    uint16_t address = getModelAddressExternalEE(modelIdx);
    for(uint8_t i = 0; i < sizeof(Model.name) && i < lenBuff - 1; i++) 
    {
      *(buff + i) = myMem.read(address + i);
    }
  }
}

//--------------------------------------------------------------------------------------------------

uint8_t eeGetModelType(uint8_t modelIdx)
{
  uint8_t result = 0xff;
  if(isInternalEE(modelIdx))
  {
    uint16_t address = getModelAddressInternalEE(modelIdx);
    address += sizeof(Model.name); //the type parameter immediately follows the model name
    result = EEPROM.read(address);
  }
  else if(hasExternalEE)
  {
    uint16_t address = getModelAddressExternalEE(modelIdx);
    address += sizeof(Model.name); //the type parameter immediately follows the model name
    result = myMem.read(address);
  }
  return result;
}

//--------------------------------------------------------------------------------------------------

void eeCreateModel(uint8_t modelIdx)
{
  //set defaults
  resetModelName();
  resetModelParams();
  //save to eeprom
  eeSaveModelData(modelIdx);
}

//--------------------------------------------------------------------------------------------------

void eeDeleteModel(uint8_t modelIdx)
{
  if(isInternalEE(modelIdx))
  {
    //simply remove name (all characters in name are set to 0xFF)
    uint16_t address = getModelAddressInternalEE(modelIdx);
    uint8_t len = sizeof(Model.name)/sizeof(Model.name[0]);
    for(uint8_t i = 0; i < len - 1; i++) 
      EEPROM.update(address + i, 0xFF);
  }
  else if(hasExternalEE)
  {
    //simply remove name (all characters in name are set to 0xFF)
    uint16_t address = getModelAddressExternalEE(modelIdx);
    uint8_t len = sizeof(Model.name)/sizeof(Model.name[0]);
    for(uint8_t i = 0; i < len - 1; i++) 
      myMem.write(address + i, 0xFF);
  }
}

//--------------------------------------------------------------------------------------------------

uint16_t getModelAddressInternalEE(uint8_t modelIdx)
{
  return ADDRESS_INT_EE_MODEL_DATA_START + (uint16_t)sizeof(Model) * modelIdx;
}

//--------------------------------------------------------------------------------------------------

uint16_t getModelAddressExternalEE(uint8_t modelIdx)
{
  return ADDRESS_EXT_EE_MODEL_DATA_START + (uint16_t)sizeof(Model) * (modelIdx - maxModelsInternal);
}

//--------------------------------------------------------------------------------------------------

bool isInternalEE(uint8_t modelIdx)
{
  if(modelIdx < maxModelsInternal)
    return true;
  else
    return false;
}

//--------------------------------------------------------------------------------------------------
//-------------------------------- For debug purposes ----------------------------------------------

bool eeHasExternalEE()
{
  return hasExternalEE;
}

uint16_t eeInternalEEGetSize()
{
  return EEPROM.length();
}

uint16_t eeExternalEEGetSize()
{
  uint16_t size = 0;
  if(hasExternalEE)
    size = myMem.length();
  return size;
}

uint8_t eeInternalEEReadByte(uint16_t address)
{
  return EEPROM.read(address);
}

uint8_t eeExternalEEReadByte(uint16_t address)
{
  uint8_t val = 0;
  if(hasExternalEE)
    val = myMem.read(address);
  return val;
}

void eeFactoryReset()
{
  uint16_t fileSignature = 0xFFFF;
  EEPROM.put(ADDRESS_INT_EE_FILE_SIGNATURE, fileSignature);
  if(hasExternalEE)
    myMem.put(ADDRESS_EXT_EE_FILE_SIGNATURE, fileSignature);
}
