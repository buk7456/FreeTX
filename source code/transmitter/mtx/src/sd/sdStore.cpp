
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#include "../../config.h"
#include "../common.h"
#include "../crc.h"
#include "../ui/ui.h"
#include "dataExport.h"
#include "dataImport.h"
#include "sdStore.h"

#if defined (DISPLAY_KS0108)
  #include "../lcd/GFX.h"
  #include "../lcd/LCDKS0108.h"
#elif defined (DISPLAY_ST7920)
  #include "../lcd/GFX.h"
  #include "../lcd/LCDST7920.h"
#endif

//limit to 128 kiB files
#define FILE_SIZE_LIMIT_BYTES 131072

bool hasSDcard = false;

//NOTE THAT ONLY 8.3 NAMES ARE SUPPORTED

const char* model_backup_directory = "MODELS/";
// const char* model_backup_directory = "/";

File modelDir;
bool isModelDirectoryOpen = false; //keeps track of open status

void showMonochromeBMP();

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
}

//--------------------------------------------------------------------------------------------------

bool sdHasCard()
{
  return hasSDcard;
}


//============================ Model backup and restore ============================================

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
  
  char fullNameStr[30]; //includes the path. E.g. MODELS/qqq.mdl
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
      showMessage(PSTR("Bad model data.\nLoading defaults"));
      delay(2000);
      resetModelName();
      resetModelParams();
      return false;
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

  if(isModelDirectoryOpen) //close first so we don't get a wrong count
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
  
  //check if exists
  if(SD.exists(fullNameStr))
    return true;

  return false;
}

//============================ Splash screen =======================================================

#define MAX_PATH_SIZE 26 
static const char splash_full_name_str[MAX_PATH_SIZE] PROGMEM = "IMAGES/SPLASH";         // Raw bytes, headerless
static const char splash_BMP_full_name_str[MAX_PATH_SIZE] PROGMEM = "IMAGES/SPLASH.BMP"; // BMP

void sdShowSplashScreen()
{
  if(!hasSDcard)
    return;

  char filename[MAX_PATH_SIZE];
 
  // Check for the BMP first, then fallback to the Raw
  strlcpy_P(filename, splash_BMP_full_name_str, sizeof(filename));
  if(SD.exists(filename))
  {
    showMonochromeBMP();
    return;
  }
  
  strlcpy_P(filename, splash_full_name_str, sizeof(filename));
  
  //abort if it doesn't exist
  if(!SD.exists(filename))
    return;
  
  File myFile = SD.open(filename);
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
    
    //read from file and write to LCD
  #if defined (DISPLAY_KS0108)
    uint8_t page = 0, column = 0;
    while(myFile.available())
    {
      uint8_t c = myFile.read();
      display.writePageColumn(page, column, c);
      column++;
      if(column > 127)
      {
        column = 0;
        page++;
        if(page > 7)
          break;
      }
    }
  #elif defined (DISPLAY_ST7920)
    uint8_t x = 0, y = 0;
    while(myFile.available())
    {
      uint8_t c = myFile.read();
      for(uint8_t j = 0; j < 8; j++)
      {
        display.drawPixel(x, y + j, ~(c >> j) & 1);
      }
      x++;
      if(x > 127)
      {
        x = 0;
        y += 8;
      }
      if(y > 63)
        break;
    }
    display.display();
  #endif

    //close the file
    myFile.close();

    //delay to make it noticeable
    delay(3000);
  }
}

//---------------------------- BMP splash helper ---------------------------------------------------

void showMonochromeBMP()
{
  char filename[MAX_PATH_SIZE];
  strlcpy_P(filename, splash_BMP_full_name_str, sizeof(filename));
  
  // abort if it doesn't exist
  if(!SD.exists(filename))
    return;

  File bmp = SD.open(filename);

  if(!bmp)
    return;

  uint8_t header[54];

  if(bmp.read(header, 54) != 54)
  {
    bmp.close();
    return;
  }

  // check BMP signature
  if(header[0] != 'B' || header[1] != 'M')
  {
    bmp.close();
    return;
  }

  uint32_t dataOffset = *(uint32_t *)&header[10];
  int32_t width = *(int32_t *)&header[18];
  int32_t height = *(int32_t *)&header[22];
  uint16_t bpp = *(uint16_t *)&header[28];

  if(width != 128 || (height != 64 && height != -64) || bpp != 1)
  {
    bmp.close();
    return;
  }

  bool topDown = (height < 0);
  int absHeight = abs(height);
  uint16_t rowSize = ((width + 31) / 32) * 4;
  
  bmp.seek(dataOffset);
  
  uint8_t row[16];  // 128 pixels = 16 bytes
  
  display.clearDisplay();
  display.fillRect(0, 0, 128, 64, BLACK);

  for(int rowNum = 0; rowNum < absHeight; rowNum++)
  {
    bmp.read(row, rowSize);
    int y = topDown ? rowNum : (absHeight - 1 - rowNum);
    for(int x = 0; x < width; x++)
    {
      uint8_t b = row[x >> 3];
      if(b & (0x80 >> (x & 7)))
      {
        display.drawPixel(x, y, WHITE);
      }
    }
  }

  bmp.close();
  
  display.display();
  
  //delay to make it noticeable
  delay(3000);
}

//============================ Screenshot writer ===================================================

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

  static const char screenshot_directory[] PROGMEM = "SCRNSHOT/";

  //create the screenshots directory
  static bool initialised = false;
  if(!initialised)
  {
    initialised = true;
    char directoryName[sizeof(screenshot_directory)];
    strlcpy_P(directoryName, screenshot_directory, sizeof(directoryName));
    if(!SD.exists(directoryName))
      SD.mkdir(directoryName);
  }

  //make a name for the file. Sequential numbering is used.
  char fullNameStr[30]; //includes the path. Example Folder/img001.bmp
  memset(fullNameStr, 0, sizeof(fullNameStr));
  strlcpy_P(fullNameStr, screenshot_directory, sizeof(fullNameStr));
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

  #if defined (UI_128X64)
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

//============================ System settings backup and restore ==================================

static const char system_directory[] PROGMEM = "SYSTEM/";

bool sdBackupSystemSettings()
{
  if(!hasSDcard)
    return false;

  if(isModelDirectoryOpen) //close first if open
  {
    modelDir.close();
    isModelDirectoryOpen = false;
  }

  //create the  directory
  static bool initialised = false;
  if(!initialised)
  {
    initialised = true;
    char directoryName[sizeof(system_directory)];
    strlcpy_P(directoryName, system_directory, sizeof(directoryName));
    if(!SD.exists(directoryName))
      SD.mkdir(directoryName);
  }

  char fullNameStr[30]; //includes the path
  memset(fullNameStr, 0, sizeof(fullNameStr));
  strlcpy_P(fullNameStr, system_directory, sizeof(fullNameStr));
  strlcat_P(fullNameStr, PSTR("SETTINGS"), sizeof(fullNameStr));
  
  //remove the file if it exists, so we start afresh
  if(SD.exists(fullNameStr))
    SD.remove(fullNameStr);
  
  File myFile = SD.open(fullNameStr, FILE_WRITE);
  if(myFile)
  {
    //export in human readable format
    exportSystemData(myFile);
    
    //close the file
    myFile.close(); 
    
    return true;
  }
  
  return false;
}

//--------------------------------------------------------------------------------------------------

bool sdRestoreSystemSettings()
{
  if(!hasSDcard)
    return false;
  
  char fullNameStr[30]; //includes the path
  memset(fullNameStr, 0, sizeof(fullNameStr));
  strlcpy_P(fullNameStr, system_directory, sizeof(fullNameStr));
  strlcat_P(fullNameStr, PSTR("SETTINGS"), sizeof(fullNameStr));
  
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
    
    //reset the sys struct as we directly read into it
    resetSystemParams();
    //import the settings
    importSystemData(myFile);
    
    //close the file
    myFile.close();

    //sanity check the data we just read in
    if(!verifySystemData())
    {
      showMessage(PSTR("Bad system data.\nReverting changes"));
      delay(2000);
      resetSystemParams();
      return false;
    }
    
    return true;
  }
  
  return false;
}

//--------------------------------------------------------------------------------------------------

bool sdSystemSettingsExists()
{
  char fullNameStr[30]; //includes the path
  memset(fullNameStr, 0, sizeof(fullNameStr));
  strlcpy_P(fullNameStr, system_directory, sizeof(fullNameStr));
  strlcat_P(fullNameStr, PSTR("SETTINGS"), sizeof(fullNameStr));
  
  if(SD.exists(fullNameStr))
    return true;
  else
    return false;
}
