#ifndef _SDSTORE_H_
#define _SDSTORE_H_

void sdStoreInit();
bool sdHasCard(); 
bool sdBackupModel(const char *name);
bool sdRestoreModel(const char *name);
uint16_t sdGetModelCount();
bool sdSimilarModelExists(const char *name);
bool sdGetModelName(char *buff, uint16_t idx, uint8_t lenBuff);
void sdShowSplashScreen();
bool sdWriteScreenshot();

#endif
