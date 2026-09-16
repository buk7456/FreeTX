#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_main_menu()
{
  switch(theScreen)
  {
    case SCREEN_UNLOCK_MAIN_MENU:
      {
        validatePassword(SCREEN_MAIN_MENU, SCREEN_HOME);
      }
      break;
    
    case SCREEN_MAIN_MENU:
      {
        drawHeader_Menu(PSTR("Main menu"));
        
        static uint8_t topItem = 1, highlightedItem = 1;

        menuInitialise();
        menuAddItem(mainMenu[MAIN_MENU_MODEL], MAIN_MENU_MODEL, icon_m_model);
        menuAddItem(mainMenu[MAIN_MENU_INPUTS], MAIN_MENU_INPUTS, icon_m_inputs);
        menuAddItem(mainMenu[MAIN_MENU_MIXER], MAIN_MENU_MIXER, icon_m_mixer);
        menuAddItem(mainMenu[MAIN_MENU_OUTPUTS], MAIN_MENU_OUTPUTS, icon_m_outputs);
        menuAddItem(mainMenu[MAIN_MENU_EXTRAS], MAIN_MENU_EXTRAS, icon_m_extras);
        menuAddItem(mainMenu[MAIN_MENU_TELEMETRY], MAIN_MENU_TELEMETRY, icon_m_telemetry);
        menuAddItem(mainMenu[MAIN_MENU_SYSTEM], MAIN_MENU_SYSTEM, icon_m_system);
        menuAddItem(mainMenu[MAIN_MENU_RECEIVER], MAIN_MENU_RECEIVER, icon_m_receiver);
        menuDraw(&topItem, &highlightedItem);

        if(menuSelectedItemID == MAIN_MENU_MODEL) 
        {
          if(Sys.lockModels && !isEmptyStr(Sys.password, sizeof(Sys.password))) 
            changeToScreen(SCREEN_UNLOCK_MODEL_MANAGER);
          else 
            changeToScreen(SCREEN_MODEL);
        }
        else if(menuSelectedItemID == MAIN_MENU_INPUTS) changeToScreen(SCREEN_INPUTS); 
        else if(menuSelectedItemID == MAIN_MENU_MIXER)  changeToScreen(SCREEN_MIXER);
        else if(menuSelectedItemID == MAIN_MENU_OUTPUTS) changeToScreen(SCREEN_OUTPUTS);
        else if(menuSelectedItemID == MAIN_MENU_EXTRAS) changeToScreen(SCREEN_EXTRAS_MENU);
        else if(menuSelectedItemID == MAIN_MENU_TELEMETRY) changeToScreen(SCREEN_TELEMETRY);
        else if(menuSelectedItemID == MAIN_MENU_SYSTEM) changeToScreen(SCREEN_SYSTEM_MENU);
        else if(menuSelectedItemID == MAIN_MENU_RECEIVER) changeToScreen(SCREEN_RECEIVER);

        if(heldButton == KEY_SELECT)
        {
          if(!Sys.rememberMenuPosition)
          {
            topItem = 1;
            highlightedItem = 1;
          }
          changeToScreen(SCREEN_HOME);
        }
      }
      break;
  }
}

#endif
