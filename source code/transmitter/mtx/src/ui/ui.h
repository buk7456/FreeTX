#ifndef _UI_H_
#define _UI_H_

void initialiseDisplay();

void showMsg(const char* str);
void showWaitMsg();

void startInitialSetup();

void handleSafetyWarnUI();
void handleMainUI();

void restoreTimerRegisters();

void restoreCounterRegisters();

#endif