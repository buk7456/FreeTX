
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#include "../../config.h"
#include "../common.h"
#include "../crc.h"
#include "../ui/ui.h"
#include "modelExport.h"
#include "modelImport.h"
#include "sdStore.h"

#if defined (DISPLAY_KS0108)
#include "../lcd/GFX.h"
#include "../lcd/LCDKS0108.h"
#endif

//limit to 128kiB files
#define FILE_SIZE_LIMIT_BYTES 131072

bool hasSDcard = false;

//NOTE THAT ONLY 8.3 NAMES ARE SUPPORTED

const char* model_backup_directory = "MODELS/";
// const char* model_backup_directory = "/";

const char* screenshot_directory = "SCRNSHOT/";

File modelDir;
bool isModelDirectoryOpen = false; //keeps track of open status

//--------------------------------------------------------------------------------------------------

void sdStoreInit()
{
  if(SD.begin(PIN_SD_CS))
    hasSDcard = true;
  else
    return;
  
  //create the backup directory
  if(!SD.exists(model_backup_directory))
    SD.mkdir(model_backup_directory);

  //create the screenshots directory 
  if(!SD.exists(screenshot_directory))
    SD.mkdir(screenshot_directory);

}

//--------------------------------------------------------------------------------------------------

bool sdHasCard()
{
  return hasSDcard;
}

//--------------------------------------------------------------------------------------------------

bool sdBackupModel(const char *name)
{
  if(!hasSDcard)
    return false;

  if(isModelDirectoryOpen) //close first if open
  {
    modelDir.close();
    isModelDirectoryOpen = false;
  }
  
  char fullNameStr[30]; //includes the path
  memset(fullNameStr, 0, sizeof(fullNameStr));
  strlcpy(fullNameStr, model_backup_directory, sizeof(fullNameStr));
  strlcat(fullNameStr, name, sizeof(fullNameStr));
  
  //remove the file if it exists, so we start afresh
  if(SD.exists(fullNameStr))
    SD.remove(fullNameStr);
  
  File myFile = SD.open(fullNameStr, FILE_WRITE);
  if(myFile)
  {
    /* 
    //binary format
    myFile.write((uint8_t *) &Model, sizeof(Model));
    */
    
    //export in human readable format
    exportModelData(myFile);
    
    //close the file
    myFile.close(); 
    
    return true;
  }
  
  return false;
}

//--------------------------------------------------------------------------------------------------

bool sdRestoreModel(const char *name)
{
  if(!hasSDcard)
    return false;
  
  char fullNameStr[30]; //includes the path. e.g MODELS/qqq.mdl
  memset(fullNameStr, 0, sizeof(fullNameStr));
  strlcpy(fullNameStr, model_backup_directory, sizeof(fullNameStr));
  strlcat(fullNameStr, name, sizeof(fullNameStr));
  
  if(isModelDirectoryOpen) //close first if open
  {
    modelDir.close();
    isModelDirectoryOpen = false;
  }
  
  File myFile = SD.open(fullNameStr);
  if(myFile)
  {
    //prevent reading large files as they could be spam or cause the system to hang
    if(myFile.size() > FILE_SIZE_LIMIT_BYTES)
    {
      myFile.close();
      return false;
    }
    
    /* 
    //read directly from the binary file into the model struct
    uint8_t *ptr = (uint8_t *)&Model;
    uint16_t i = 0;
    while(myFile.available())
    {
      *(ptr + i) = myFile.read();
      i++;
      if(i == sizeof(Model))
        break;
    } 
    */
    
    //reset model struct as we directly read into it
    resetModelName();
    resetModelParams();
    //import the model
    importModelData(myFile);
    
    //close the file
    myFile.close();

    //sanity check the model data we just read in
    if(!verifyModelData())
    {
      showMsg(PSTR("Bad model data.\nLoading defaults"));
      delay(2000);
      resetModelName();
      resetModelParams();
    }
    
    return true;
  }
  
  return false;
}

//--------------------------------------------------------------------------------------------------

uint16_t sdGetModelCount()
{
  if(!hasSDcard)
    return 0;

  if(isModelDirectoryOpen) //close first so we dont get a wrong count
  {
    modelDir.close();
    isModelDirectoryOpen = false;
  }
  
  uint16_t count = 0;
  
  modelDir = SD.open(model_backup_directory);
  if(modelDir)
  {
    while(true)
    {
      File entry = modelDir.openNextFile();
      if(!entry) //no more files
        break;
      if(!entry.isDirectory())//a file
        count++;
      //close
      entry.close();
    }
    //close
    modelDir.close();
  }

  return count;
}

//--------------------------------------------------------------------------------------------------

bool sdGetModelName(char *buff, uint16_t idx, uint8_t lenBuff)
{
  if(!hasSDcard)
    return false;
  
  static uint16_t counter = 0;
  static uint16_t prevIdx = 0;
  bool rewind = false;
  if(idx < prevIdx)
    rewind = true;
  prevIdx = idx;

  if(!isModelDirectoryOpen)
  {
    modelDir = SD.open(model_backup_directory);
    counter = 0;
  }
  if(modelDir)
  {
    isModelDirectoryOpen = true;
    if(rewind)
    {
      modelDir.rewindDirectory(); //return to first file
      counter = 0;
    }
    while(true)
    {
      File entry = modelDir.openNextFile();
      if(!entry) //no more files
        break;
      if(!entry.isDirectory())//a file
      {
        if(idx == counter) //found it
        {
          //get the name into buffer
          strlcpy(buff, entry.name(), lenBuff);
          //close
          entry.close();
          counter++;
          break;
        }
        counter++;
      }
      //close
      entry.close();
    }
    
    //here we do not close the modelDir to allow persistency 
    //and fast retrieval if we have a lot of files inside it.
    
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------

bool sdSimilarModelExists(const char *name)
{
  if(!hasSDcard)
    return false;

  if(isModelDirectoryOpen) //close first if open
  {
    modelDir.close();
    isModelDirectoryOpen = false;
  }
  
  char fullNameStr[30]; //includes the path
  memset(fullNameStr, 0, sizeof(fullNameStr));
  strlcpy(fullNameStr, model_backup_directory, sizeof(fullNameStr));
  strlcat(fullNameStr, name, sizeof(fullNameStr));
  
  //chech if exists
  if(SD.exists(fullNameStr))
    return true;

  return false;
}

//--------------------------- Splash screen -------------------------------------------------------

void sdShowSplashScreen()
{
  if(!hasSDcard)
    return;

  static const char* splash_full_name_str = "IMAGES/SPLASH";
  
  //abort if it doesnt exist
  if(!SD.exists(splash_full_name_str))
    return;
  
  File myFile = SD.open(splash_full_name_str);
  if(myFile)
  {
    if(myFile.isDirectory())//not a file, abort
    {
      myFile.close();
      return;
    }

    //prevent reading large files as they could be spam or cause the system to hang
    if(myFile.size() > FILE_SIZE_LIMIT_BYTES)
    {
      myFile.close();
      return;
    }
    
    //--- read from file and directly write to the lcd ---

    uint8_t page = 0;
    uint8_t column = 0;

    while(myFile.available())
    {
      uint8_t c = myFile.read();

    #if defined (DISPLAY_KS0108)
      display.writePageColumn(page, column, c);
      column++;
      if(column > 127)
      {
        column = 0;
        page++;
        if(page > 7)
          break;
      }
    #endif

    } 
    
    //close the file
    myFile.close();

    //delay to make it noticeable
    delay(3000);
  }
}

//----------------------------Screenshot ----------------------------------------------------------

bool sdWriteScreenshot()
{
  if(!hasSDcard)
    return false;

  #if defined (UI_128X64)
  static const uint8_t bmpHeader[] PROGMEM = {
    0x42, 0x4d, 0x3e, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x28, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00
  };
  #endif

  if(isModelDirectoryOpen) //close first if open
  {
    modelDir.close();
    isModelDirectoryOpen = false;
  }

  //make a name for the file. Sequential numbering is used.
  char fullNameStr[30]; //includes the path. Example Folder/img001.bmp
  memset(fullNameStr, 0, sizeof(fullNameStr));
  strlcpy(fullNameStr, screenshot_directory, sizeof(fullNameStr));
  strlcat_P(fullNameStr, PSTR("img"), sizeof(fullNameStr));
  uint8_t digits = 1;
  uint16_t num = Sys.screenshotSeqNo;
  while(num >= 10)
  {
    num /= 10;
    digits++;
  }
  while(digits < 3)
  {
    strlcat_P(fullNameStr, PSTR("0"), sizeof(fullNameStr));
    digits++;
  }
  char temp[6];
  memset(temp, 0, sizeof(temp));
  itoa(Sys.screenshotSeqNo, temp, 10);
  strlcat(fullNameStr, temp, sizeof(fullNameStr));
  strlcat_P(fullNameStr, PSTR(".bmp"), sizeof(fullNameStr));
  
  //remove the file if it exists, so we start afresh
  if(SD.exists(fullNameStr))
    SD.remove(fullNameStr);
  
  File myFile = SD.open(fullNameStr, FILE_WRITE);
  if(myFile)
  {

  #if defined(UI_128X64)
    //write the header
    for(uint8_t i = 0; i < sizeof(bmpHeader); i++)
    {
      myFile.write(pgm_read_byte(bmpHeader + i));
    }

    //write the pixel data
    for(int8_t y = 63; y >= 0; y--)
    {
      for(uint8_t _byte = 0; _byte < 16; _byte++)
      {
        uint8_t val = 0;
        for(int8_t _bit = 7; _bit >= 0; _bit--)
        {
          uint8_t x = (_byte * 8) + (7 -_bit);
          val |= (display.getPixel(x, y) << _bit);
        }
        myFile.write(~val);
      }
    }

  #endif

    //close the file
    myFile.close(); 
    
    //update the sequence number
    Sys.screenshotSeqNo++;
    if(Sys.screenshotSeqNo > 999)
      Sys.screenshotSeqNo = 0;
    
    return true;
  }

  return false;
}
