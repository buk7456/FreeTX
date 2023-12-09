
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#include "../mtx.h"
#include "../ui/ui.h"

#include "sdStore.h"
#include "modelExport.h"
#include "modelImport.h"

//limit to 128kiB files
#define FILE_SIZE_LIMIT_BYTES 131072

bool hasSDcard = false;

const char* model_backup_directory = "MODELS/";
// const char* model_backup_directory = "/";

File modelDir;
bool isModelDirectoryOpen = false; //keeps track of open status

//--------------------------------------------------------------------------------------------------

void sdStoreInit()
{
  if(SD.begin(PIN_SD_CS))
    hasSDcard = true;
  else
    return;
  
  //create the backup directory if it does not exist
  modelDir = SD.open(model_backup_directory);
  if(!modelDir)
  {
    SD.mkdir(model_backup_directory);
  }
  //close
  modelDir.close();
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

  if(isModelDirectoryOpen) //close first as we can only open one thing at a time
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
  
  if(isModelDirectoryOpen) //close first as we can only open one thing at a time
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

  if(isModelDirectoryOpen) //close first as we can only open one thing at a time
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
