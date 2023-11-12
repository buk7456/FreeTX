#ifndef _SDSTORE_H_
#define _SDSTORE_H_

void sdStoreInit();

bool sdHasCard(); 

//Backup a model. returns true on success
bool sdBackupModel(const char *name);

//Restore a model
bool sdRestoreModel(const char *name);

//Get count of backed up models
uint16_t sdGetModelCount();

//Check if a model with similar name exists in the backup
bool sdSimilarModelExists(const char *name);

//get name
bool sdGetModelName(char *buff, uint16_t idx, uint8_t lenBuff);

//Open directory

//Delete directory

//Delete file

//Open a file

//Close a file

//Get length of a file (in bytes)

//Read a byte from a file, at a certain offset into file

#endif