#ifndef _UI_H_
#define _UI_H_

void initialiseDisplay();

void showMessage(const char* str);
void showProgressMessage(const char* str, uint8_t percent);
void showWaitMessage();
void showMuteMessage();

void startInitialSetup();

void handleMainUI();
void handleSafetyWarningUI();

#if defined (DISPLAY_KS0108)
  #include "../lcd/GFX.h"
  #include "../lcd/LCDKS0108.h"
  extern LCDKS0108 display; //shared object
#elif defined (DISPLAY_ST7920)
  #include "../lcd/GFX.h"
  #include "../lcd/LCDST7920.h"
  extern LCDST7920 display; //shared object
#endif


#endif
