#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_extras_menu()
{
  switch(theScreen)
  {
    case SCREEN_EXTRAS_MENU:
      {
        drawHeader_Menu(mainMenu[MAIN_MENU_EXTRAS]);
        
        static uint8_t topItem = 1, highlightedItem = 1;
        
        menuInitialise();
        menuAddItem(extrasMenu[EXTRAS_MENU_CUSTOM_CURVES], EXTRAS_MENU_CUSTOM_CURVES, NULL);
        menuAddItem(extrasMenu[EXTRAS_MENU_LOGICAL_SWITCHES], EXTRAS_MENU_LOGICAL_SWITCHES, NULL);
        menuAddItem(extrasMenu[EXTRAS_MENU_FUNCTION_GENERATORS], EXTRAS_MENU_FUNCTION_GENERATORS, NULL);
        menuAddItem(extrasMenu[EXTRAS_MENU_TIMERS], EXTRAS_MENU_TIMERS, NULL);
        menuAddItem(extrasMenu[EXTRAS_MENU_COUNTERS], EXTRAS_MENU_COUNTERS, NULL);
        menuAddItem(extrasMenu[EXTRAS_MENU_NOTIFICATION_SETUP], EXTRAS_MENU_NOTIFICATION_SETUP, NULL);
        menuAddItem(extrasMenu[EXTRAS_MENU_SAFETY_CHECKS], EXTRAS_MENU_SAFETY_CHECKS, NULL);
        menuAddItem(extrasMenu[EXTRAS_MENU_TRIM_SETUP], EXTRAS_MENU_TRIM_SETUP, NULL);
        if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
          menuAddItem(extrasMenu[EXTRAS_MENU_FLIGHT_MODES], EXTRAS_MENU_FLIGHT_MODES, NULL);
        menuDraw(&topItem, & highlightedItem);

        if(menuSelectedItemID == EXTRAS_MENU_CUSTOM_CURVES) changeToScreen(SCREEN_CUSTOM_CURVES);
        if(menuSelectedItemID == EXTRAS_MENU_LOGICAL_SWITCHES) changeToScreen(SCREEN_LOGICAL_SWITCHES);
        if(menuSelectedItemID == EXTRAS_MENU_FLIGHT_MODES) changeToScreen(SCREEN_FLIGHT_MODES);
        if(menuSelectedItemID == EXTRAS_MENU_TRIM_SETUP) changeToScreen(SCREEN_TRIM_SETUP);
        if(menuSelectedItemID == EXTRAS_MENU_SAFETY_CHECKS) changeToScreen(SCREEN_SAFETY_CHECKS);
        if(menuSelectedItemID == EXTRAS_MENU_COUNTERS) changeToScreen(SCREEN_COUNTERS);
        if(menuSelectedItemID == EXTRAS_MENU_FUNCTION_GENERATORS) changeToScreen(SCREEN_FUNCTION_GENERATORS);
        if(menuSelectedItemID == EXTRAS_MENU_TIMERS) changeToScreen(SCREEN_TIMERS);
        if(menuSelectedItemID == EXTRAS_MENU_NOTIFICATION_SETUP) changeToScreen(SCREEN_NOTIFICATION_SETUP);

        //Exit
        if(heldButton == KEY_SELECT)
        {
          if(!Sys.rememberMenuPosition)
          {
            topItem = 1;
            highlightedItem = 1;
          }
          changeToScreen(SCREEN_MAIN_MENU);
        }
      }
      break;
  }
}

#endif
