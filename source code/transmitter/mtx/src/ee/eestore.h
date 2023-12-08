#ifndef _EESTORE_H_
#define _EESTORE_H_

void eeStoreInit();

void eeReadSysConfig();
void eeSaveSysConfig();

void eeReadModelData(uint8_t modelIdx); 
void eeSaveModelData(uint8_t modelIdx);

void eeLazyWriteModelData(uint8_t modelIdx);

void eeGetModelName(char* buff, uint8_t modelIdx, uint8_t lenBuff);
uint8_t eeGetModelType(uint8_t modelIdx);

bool eeModelIsFree(uint8_t modelIdx);
void eeCreateModel(uint8_t modelIdx);
void eeDeleteModel(uint8_t modelIdx);

//mainly for debug purposes
bool     eeHasExternalEE();
uint16_t eeInternalEEGetSize();
uint16_t eeExternalEEGetSize();
uint8_t  eeInternalEEReadByte(uint16_t address);
uint8_t  eeExternalEEReadByte(uint16_t address);
void     eeFactoryReset();

#endif
