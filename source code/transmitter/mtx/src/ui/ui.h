#ifndef _UI_H_
#define _UI_H_

void initialiseDisplay();

void showMsg(const char* str);
void showProgressMsg(const char* str, uint8_t percent);
void showWaitMsg();
void showMuteMsg();

void startInitialSetup();

void handleSafetyWarnUI();
void handleMainUI();

void restoreTimerRegisters();
void restoreCounterRegisters();

#if defined (DISPLAY_KS0108)
#include "../lcd/GFX.h"
#include "../lcd/LCDKS0108.h"
extern LCDKS0108 display; //shared object
#endif

#endif
