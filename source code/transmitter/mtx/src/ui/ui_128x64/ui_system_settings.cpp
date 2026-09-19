#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_system_settings()
{
  switch(theScreen)
  {
    case SCREEN_SYSTEM_MENU:
      {
        drawHeader_Menu(mainMenu[MAIN_MENU_SYSTEM]);
        
        static uint8_t topItem = 1, highlightedItem = 1;

        menuInitialise();
        menuAddItem(systemMenu[SYSTEM_MENU_RF], SYSTEM_MENU_RF, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_SOUND], SYSTEM_MENU_SOUND, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_BACKLIGHT], SYSTEM_MENU_BACKLIGHT, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_APPEARANCE], SYSTEM_MENU_APPEARANCE, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_MISCELLANEOUS], SYSTEM_MENU_MISCELLANEOUS, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_ADVANCED], SYSTEM_MENU_ADVANCED, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_ABOUT], SYSTEM_MENU_ABOUT, NULL);
        menuDraw(&topItem, &highlightedItem);

        if(menuSelectedItemID == SYSTEM_MENU_RF) changeToScreen(SCREEN_RF);
        else if(menuSelectedItemID == SYSTEM_MENU_SOUND) changeToScreen(SCREEN_SOUND);
        else if(menuSelectedItemID == SYSTEM_MENU_BACKLIGHT) changeToScreen(SCREEN_BACKLIGHT);
        else if(menuSelectedItemID == SYSTEM_MENU_APPEARANCE) changeToScreen(SCREEN_APPEARANCE);
        else if(menuSelectedItemID == SYSTEM_MENU_MISCELLANEOUS) changeToScreen(SCREEN_MISCELLANEOUS);
        else if(menuSelectedItemID == SYSTEM_MENU_ABOUT) changeToScreen(SCREEN_ABOUT);
        else if(menuSelectedItemID == SYSTEM_MENU_ADVANCED)
        {
          if(!isEmptyStr(Sys.password, sizeof(Sys.password))) 
            changeToScreen(SCREEN_UNLOCK_ADVANCED_MENU); //prompt to enter password first
          else //no password specified, don't prompt
            changeToScreen(SCREEN_ADVANCED_MENU);
        }

        //exit
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
      
    case SCREEN_RF:
      {
        drawHeader(systemMenu[SYSTEM_MENU_RF]);
        
        display.setCursor(0, 9);
        display.print(F("RF output:"));
        drawCheckbox(72, 9, Sys.rfEnabled);
      
        display.setCursor(0, 18);
        display.print(F("RF power:"));
        display.setCursor(72, 18);
        display.print(findStringInIdStr(enum_RFpower, Sys.rfPower));

        changeFocusOnUpDown(2);
        toggleEditModeOnSelectClicked();
        drawCursor(64, focusedItem * 9);
        
        //edit
        if(focusedItem == 1)
          Sys.rfEnabled = incDec(Sys.rfEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 2)
          Sys.rfPower = incDec(Sys.rfPower, 0, RF_POWER_COUNT - 1, INCDEC_NOWRAP, INCDEC_SLOW);
        
        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_SYSTEM_MENU);
        }
      }
      break;
      
    case SCREEN_SOUND:
      {
        drawHeader(systemMenu[SYSTEM_MENU_SOUND]);
        
        enum {
          ITEM_SOUND_ENABLED,
          ITEM_SOUND_ON_INACTIVITY,
          ITEM_SOUND_SWITCHES,
          ITEM_SOUND_KNOBS,
          ITEM_SOUND_KEYS,
          ITEM_SOUND_TRIMS,
          ITEM_TRIM_TONE_FREQ_MODE,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(!Sys.soundEnabled && i > ITEM_SOUND_ENABLED)
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        changeFocusOnUpDown(listItemCount);
        static uint8_t topItem = 1;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 6)
          topItem++;
        
        toggleEditModeOnSelectClicked();
        
        //fill list and edit items
        for(uint8_t line = 0; line < 6 && line < listItemCount; line++)
        {
          uint8_t ypos = 9 + line*9;
          if(focusedItem == topItem + line)
            drawCursor(64, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (itemID == listItemIDs[focusedItem - 1] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_SOUND_ENABLED:
              {
                display.print(F("Enable:")); 
                drawCheckbox(72, ypos, Sys.soundEnabled);
                if(edit) 
                  Sys.soundEnabled = incDec(Sys.soundEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;

            case ITEM_SOUND_ON_INACTIVITY:
              {
                display.print(F("Inactvty:")); 
                drawCheckbox(72, ypos, Sys.soundOnInactivity);
                if(edit) 
                  Sys.soundOnInactivity = incDec(Sys.soundOnInactivity, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_SOUND_SWITCHES:
              {
                display.print(F("Switches:"));
                drawCheckbox(72, ypos, Sys.soundSwitches);
                if(edit)
                  Sys.soundSwitches = incDec(Sys.soundSwitches, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_SOUND_KNOBS:
              {
                display.print(F("Knobs:"));
                drawCheckbox(72, ypos, Sys.soundKnobCenter);
                if(edit)
                  Sys.soundKnobCenter = incDec(Sys.soundKnobCenter, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_SOUND_KEYS:
              {
                display.print(F("Keys:"));
                drawCheckbox(72, ypos, Sys.soundKeys);
                if(edit)
                  Sys.soundKeys = incDec(Sys.soundKeys, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_SOUND_TRIMS:
              {
                display.print(F("Trims:"));
                drawCheckbox(72, ypos, Sys.soundTrims);
                if(edit)
                  Sys.soundTrims = incDec(Sys.soundTrims, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_TRIM_TONE_FREQ_MODE:
              {
                display.print(F("Trim tone:"));
                display.setCursor(72, ypos);
                display.print(findStringInIdStr(enum_TrimToneFreqMode, Sys.trimToneFreqMode));
                if(edit)
                  Sys.trimToneFreqMode = incDec(Sys.trimToneFreqMode, 0, TRIM_TONE_FREQ_MODE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 9, listItemCount, topItem, 6, 6 * 9);
        
        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_SYSTEM_MENU);
      }
      break;
      
    case SCREEN_BACKLIGHT:
      {
        drawHeader(systemMenu[SYSTEM_MENU_BACKLIGHT]);
        
        display.setCursor(0, 9);
        display.print(F("Enable:"));
        drawCheckbox(72, 9, Sys.backlightEnabled);
        
        if(Sys.backlightEnabled)
        {
          changeFocusOnUpDown(5);
          
          display.setCursor(0, 18);
          display.print(F("Brightness:"));
          display.setCursor(72, 18);
          display.print(Sys.backlightBrightness);
          display.print(F("%"));
          
          display.setCursor(0, 27);
          display.print(F("Timeout:"));
          display.setCursor(72, 27);
          display.print(findStringInIdStr(enum_BacklightTimeout, Sys.backlightTimeout));
          
          display.setCursor(0, 36);
          display.print(F("Wake up:"));
          display.setCursor(72, 36);
          display.print(findStringInIdStr(enum_BacklightWakeup, Sys.backlightWakeup));
          
          display.setCursor(0, 45);
          display.print(F("Key filter:"));
          drawCheckbox(72, 45, Sys.backlightSuppressFirstKey);
        }
        
        toggleEditModeOnSelectClicked();
        drawCursor(64, focusedItem * 9);
        
        //edit
        if(focusedItem == 1)
          Sys.backlightEnabled = incDec(Sys.backlightEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 2)
          Sys.backlightBrightness = incDec(Sys.backlightBrightness, 10, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
        else if(focusedItem == 3)
          Sys.backlightTimeout = incDec(Sys.backlightTimeout, 0, BACKLIGHT_TIMEOUT_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 4)
          Sys.backlightWakeup = incDec(Sys.backlightWakeup, 0, BACKLIGHT_WAKEUP_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 5)
          Sys.backlightSuppressFirstKey = incDec(Sys.backlightSuppressFirstKey, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        
        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_SYSTEM_MENU);
        }
      }
      break;
      
    case SCREEN_APPEARANCE:
      {
        drawHeader(systemMenu[SYSTEM_MENU_APPEARANCE]);
        
        enum {
          ITEM_SHOW_MENU_ICONS,
          ITEM_KEEP_MENU_POSITION,
          ITEM_USE_DENSER_MENUS,
          ITEM_USE_ROUND_CORNERS,
          ITEM_ENABLE_ANIMATIONS,
          ITEM_AUTOHIDE_TRIMS,
          ITEM_ALWAYS_SHOW_HOURS_FOR_TIMERS,
          ITEM_USE_NUMERICAL_BATTERY_INDICATOR,
          ITEM_SHOW_WELCOME_MSG,
          ITEM_SHOW_SPLASH_SCREEN,
          
          ITEM_COUNT
        };
        
        //handle navigation
        changeFocusOnUpDown(ITEM_COUNT); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 6)
          topItem++;
        
        //fill list and edit items
        for(uint8_t line = 0; line < 6 && line < ITEM_COUNT; line++)
        {
          uint8_t ypos = 9 + line*9;
          if(focusedItem == topItem + line)
            drawCursor(94, ypos);
          
          if((topItem - 1 + line) >= ITEM_COUNT)
            break;
          
          uint8_t itemID = topItem - 1 + line;
          bool edit = (itemID == focusedItem - 1 && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_SHOW_MENU_ICONS:
              {
                display.print(F("Menu icons:")); 
                drawCheckbox(102, ypos, Sys.showMenuIcons);
                if(edit) 
                  Sys.showMenuIcons = incDec(Sys.showMenuIcons, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_KEEP_MENU_POSITION:
              {
                display.print(F("Keep menu pstn:")); 
                drawCheckbox(102, ypos, Sys.rememberMenuPosition);
                if(edit) 
                  Sys.rememberMenuPosition = incDec(Sys.rememberMenuPosition, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;

            case ITEM_USE_DENSER_MENUS:
              {
                display.print(F("Denser menus:")); 
                drawCheckbox(102, ypos, Sys.useDenserMenus);
                if(edit) 
                  Sys.useDenserMenus = incDec(Sys.useDenserMenus, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_USE_ROUND_CORNERS:
              {
                display.print(F("Round corners:")); 
                drawCheckbox(102, ypos, Sys.useRoundRect);
                if(edit) 
                  Sys.useRoundRect = incDec(Sys.useRoundRect, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_ENABLE_ANIMATIONS:
              {
                display.print(F("Animations:")); 
                drawCheckbox(102, ypos, Sys.animationsEnabled);
                if(edit) 
                  Sys.animationsEnabled = incDec(Sys.animationsEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_AUTOHIDE_TRIMS:
              {
                display.print(F("Autohide trims:")); 
                drawCheckbox(102, ypos, Sys.autohideTrims);
                if(edit) 
                  Sys.autohideTrims = incDec(Sys.autohideTrims, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_SHOW_SPLASH_SCREEN:
              {
                display.print(F("Splash screen:")); 
                drawCheckbox(102, ypos, Sys.showSplashScreen);
                if(edit) 
                  Sys.showSplashScreen = incDec(Sys.showSplashScreen, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_SHOW_WELCOME_MSG:
              {
                display.print(F("Welcome msg:")); 
                drawCheckbox(102, ypos, Sys.showWelcomeMessage);
                if(edit) 
                  Sys.showWelcomeMessage = incDec(Sys.showWelcomeMessage, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;

            case ITEM_USE_NUMERICAL_BATTERY_INDICATOR:
              {
                display.print(F("Numeric Batt V:")); 
                drawCheckbox(102, ypos, Sys.useNumericalBatteryIndicator);
                if(edit) 
                  Sys.useNumericalBatteryIndicator = incDec(Sys.useNumericalBatteryIndicator, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;

            case ITEM_ALWAYS_SHOW_HOURS_FOR_TIMERS:
              {
                display.print(F("Always show hr:"));
                drawCheckbox(102, ypos, Sys.alwaysShowHours);
                if(edit)
                  Sys.alwaysShowHours = incDec(Sys.alwaysShowHours, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 9, ITEM_COUNT, topItem, 6, 6 * 9);

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_SYSTEM_MENU);
      }
      break;

    case SCREEN_MISCELLANEOUS:
      {
        drawHeader(systemMenu[SYSTEM_MENU_MISCELLANEOUS]);
        
        enum {
          ITEM_AUTOSELECT_INPUT,
          ITEM_MIXER_TEMPLATES,
          ITEM_DEFAULT_CHANNEL_ORDER,
          ITEM_INACTIVITY_MINUTES,
          
          ITEM_SUBHEADING_GNSS_UNITS,
          ITEM_DEFAULT_GNSS_UNITS,
          ITEM_CUSTOM_GNSS_DISTANCE_UNITS,
          ITEM_CUSTOM_GNSS_SPEED_UNITS,
          ITEM_CUSTOM_GNSS_ALTITUDE_UNITS,
          ITEM_CUSTOM_GNSS_UNITS_FIRST = ITEM_CUSTOM_GNSS_DISTANCE_UNITS,
          ITEM_CUSTOM_GNSS_UNITS_LAST = ITEM_CUSTOM_GNSS_ALTITUDE_UNITS,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(Sys.defaultGnssUnits != GNSS_DEFAULT_UNITS_CUSTOM 
              && i >= ITEM_CUSTOM_GNSS_UNITS_FIRST 
              && i <= ITEM_CUSTOM_GNSS_UNITS_LAST)
          {
            continue;
          }
          listItemIDs[listItemCount++] = i;
        }

        //handle navigation
        do {
          changeFocusOnUpDown(listItemCount);
        } while(listItemIDs[focusedItem - 1] == ITEM_SUBHEADING_GNSS_UNITS);
        static uint8_t topItem = 1;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 6)
          topItem++;
        
        toggleEditModeOnSelectClicked();
        
        //fill list and edit items
        for(uint8_t line = 0; line < 6 && line < listItemCount; line++)
        {
          uint8_t ypos = 9 + line*9;
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (itemID == listItemIDs[focusedItem - 1]);
          bool edit = (isFocused && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_AUTOSELECT_INPUT:
              {
                display.print(F("Autoslct input:"));
                drawCheckbox(102, ypos, Sys.autoSelectMovedControl);
                if(isFocused)
                  drawCursor(94, ypos);
                if(edit)                  
                  Sys.autoSelectMovedControl = incDec(Sys.autoSelectMovedControl, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_MIXER_TEMPLATES:
              {
                display.print(F("Mixer templts:"));
                drawCheckbox(102, ypos, Sys.mixerTemplatesEnabled);
                if(isFocused)
                  drawCursor(94, ypos);
                if(edit)                  
                  Sys.mixerTemplatesEnabled = incDec(Sys.mixerTemplatesEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_DEFAULT_CHANNEL_ORDER:
              {
                display.print(F("Dflt Ch order:"));
                display.setCursor(102, ypos);
                if(Sys.mixerTemplatesEnabled)
                {
                  //make the string for the channel order
                  char tmpStr[5];
                  tmpStr[getChannelIdx('A')] = 'A';
                  tmpStr[getChannelIdx('E')] = 'E';
                  tmpStr[getChannelIdx('T')] = 'T';
                  tmpStr[getChannelIdx('R')] = 'R';
                  tmpStr[4] = '\0';
                  display.print(tmpStr);
                }
                else
                  display.print(F("N/A"));
                if(isFocused)
                  drawCursor(94, ypos);
                if(edit && Sys.mixerTemplatesEnabled) 
                {
                  //there are 4P4 = 4! = 24 possible arrangements. So our range is 0 to 23.  
                  Sys.defaultChannelOrder = incDec(Sys.defaultChannelOrder, 0, 23, INCDEC_WRAP, INCDEC_SLOW);
                }
              }
              break;
              
            case ITEM_INACTIVITY_MINUTES:
              {
                display.print(F("Inactvty mins:"));
                display.setCursor(102, ypos);
                display.print(Sys.inactivityMinutes);
                if(isFocused)
                  drawCursor(94, ypos);
                if(edit)                  
                  Sys.inactivityMinutes = incDec(Sys.inactivityMinutes, 1, 240, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_SUBHEADING_GNSS_UNITS:
              {
                drawSubheader(PSTR("GNSS units"), ypos);
              }
              break;
              
            case ITEM_DEFAULT_GNSS_UNITS:
              {
                display.print(F("Default:"));
                display.setCursor(66, ypos);
                display.print(findStringInIdStr(enum_DefaultGNSSUnits, Sys.defaultGnssUnits));
                if(isFocused)
                  drawCursor(58, ypos);
                if(edit) 
                  Sys.defaultGnssUnits = incDec(Sys.defaultGnssUnits, 0, GNSS_DEFAULT_UNITS_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;

            case ITEM_CUSTOM_GNSS_DISTANCE_UNITS:
              {
                display.print(F("Distance:"));
                display.setCursor(66, ypos);
                display.print(findStringInIdStr(enum_DisplayedUnits, Sys.customGnssDistanceUnits));
                if(isFocused)
                  drawCursor(58, ypos);
                if(edit) 
                  Sys.customGnssDistanceUnits = incDec(Sys.customGnssDistanceUnits, UNITS_DISTANCE_FIRST, UNITS_DISTANCE_LAST, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;

            case ITEM_CUSTOM_GNSS_SPEED_UNITS:
              {
                display.print(F("Speed:"));
                display.setCursor(66, ypos);
                display.print(findStringInIdStr(enum_DisplayedUnits, Sys.customGnssSpeedUnits));
                if(isFocused)
                  drawCursor(58, ypos);
                if(edit) 
                  Sys.customGnssSpeedUnits = incDec(Sys.customGnssSpeedUnits, UNITS_SPEED_FIRST, UNITS_SPEED_LAST, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_CUSTOM_GNSS_ALTITUDE_UNITS:
              {
                display.print(F("Altitude:"));
                display.setCursor(66, ypos);
                display.print(findStringInIdStr(enum_DisplayedUnits, Sys.customGnssAltitudeUnits));
                if(isFocused)
                  drawCursor(58, ypos);
                if(edit && (buttonCode == KEY_UP || buttonCode == KEY_DOWN))
                {
                  do {
                    Sys.customGnssAltitudeUnits = incDec(Sys.customGnssAltitudeUnits, UNITS_DISTANCE_FIRST, UNITS_DISTANCE_LAST, INCDEC_WRAP, INCDEC_SLOW);
                  } while (Sys.customGnssAltitudeUnits != UNITS_METRES && Sys.customGnssAltitudeUnits != UNITS_FEET);
                }
              }
              break;
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 9, listItemCount, topItem, 6, 6 * 9);
        
        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_SYSTEM_MENU);
      }
      break;
      
    case SCREEN_UNLOCK_ADVANCED_MENU:
      {
        validatePassword(SCREEN_ADVANCED_MENU, SCREEN_SYSTEM_MENU);
      }
      break;
      
    case SCREEN_ADVANCED_MENU:
      {
        drawHeader_Menu(systemMenu[SYSTEM_MENU_ADVANCED]);
        
        static uint8_t topItem = 1, highlightedItem = 1;

        menuInitialise();
        menuAddItem(advancedMenu[ADVANCED_MENU_STICKS], ADVANCED_MENU_STICKS, NULL);
        menuAddItem(advancedMenu[ADVANCED_MENU_KNOBS], ADVANCED_MENU_KNOBS, NULL);
        menuAddItem(advancedMenu[ADVANCED_MENU_SWITCHES], ADVANCED_MENU_SWITCHES, NULL);
        menuAddItem(advancedMenu[ADVANCED_MENU_BATTERY], ADVANCED_MENU_BATTERY, NULL);
        menuAddItem(advancedMenu[ADVANCED_MENU_SECURITY], ADVANCED_MENU_SECURITY, NULL);
        menuAddItem(advancedMenu[ADVANCED_MENU_DEBUG], ADVANCED_MENU_DEBUG, NULL);
        menuDraw(&topItem, &highlightedItem);

        if(menuSelectedItemID == ADVANCED_MENU_STICKS) changeToScreen(SCREEN_STICKS);
        else if(menuSelectedItemID == ADVANCED_MENU_KNOBS) changeToScreen(SCREEN_KNOBS);
        else if(menuSelectedItemID == ADVANCED_MENU_SWITCHES) changeToScreen(SCREEN_SWITCHES); 
        else if(menuSelectedItemID == ADVANCED_MENU_BATTERY) changeToScreen(SCREEN_BATTERY);
        else if(menuSelectedItemID == ADVANCED_MENU_SECURITY) changeToScreen(SCREEN_SECURITY);
        else if(menuSelectedItemID == ADVANCED_MENU_DEBUG) changeToScreen(SCREEN_DEBUG);

        //exit
        if(heldButton == KEY_SELECT)
        {
          if(!Sys.rememberMenuPosition)
          {
            topItem = 1;
            highlightedItem = 1;
          }
          changeToScreen(SCREEN_SYSTEM_MENU);
        }
      }
      break;
      
    case SCREEN_STICKS:
      {
        drawHeader(advancedMenu[ADVANCED_MENU_STICKS]);

        enum {
          PAGE_START,
          PAGE_AXIS_TYPE,
          PAGE_MOVE_STICKS, 
          PAGE_ADJUST_DEADZONE
        };
        
        static uint8_t page = PAGE_START;
        static bool initialised = false;

        if(!initialised)
        {
          initialised = true;
          focusedItem = 1;
          isEditMode = false;
          page = (lastScreen == SCREEN_HOME) ? PAGE_AXIS_TYPE : PAGE_START;
        }

        uint8_t numItems = NUM_STICK_AXES;
        
        bool allAxesAbsent = false;
        bool hasSelfCenteringAxes = false;
        uint8_t cntrQQ = 0;
        for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
        {
          if(Sys.StickAxis[i].type == STICK_AXIS_ABSENT)
            cntrQQ++;
          else if(Sys.StickAxis[i].type == STICK_AXIS_SELF_CENTERING)
            hasSelfCenteringAxes = true;
        }
        if(cntrQQ == NUM_STICK_AXES)
          allAxesAbsent = true;
        
        switch(page)
        {
          case PAGE_START:
            {
              display.setCursor(0, 9);
              display.print(F("Calibrtn:"));
              display.setCursor(78, 9);
              display.print(F("[Start]"));
              
              display.setCursor(0, 18);
              display.print(F("Deadzone:"));
              display.setCursor(78, 18);
              display.print(F("[Adjust]"));
              
              display.setCursor(0, 27);
              display.print(F("Deflt mode:"));
              display.setCursor(78, 27);
              display.print(findStringInIdStr(enum_StickMode, Sys.defaultStickMode));
              
              drawCursor(70, focusedItem * 9);
              
              changeFocusOnUpDown(3);
              toggleEditModeOnSelectClicked();
              
              if(focusedItem == 1 && isEditMode)
              {
                page = PAGE_AXIS_TYPE;
                isEditMode = false;
              }
              else if(focusedItem == 2 && isEditMode)
              {
                page = PAGE_ADJUST_DEADZONE;
                isEditMode = false;
              }
              else if(focusedItem == 3)
                Sys.defaultStickMode = incDec(Sys.defaultStickMode, 0, STICK_MODE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              
              if(heldButton == KEY_SELECT)
              {
                initialised = false;
                changeToScreen(SCREEN_ADVANCED_MENU);
              }
            }
            break;
          
          case PAGE_AXIS_TYPE:
            {
              isCalibratingControls = true;
              
              display.setCursor(0, 9);
              display.print(F("Type of axis"));
              
              //scrollable list
              
              static uint8_t topItem;
              static bool viewInitialised = false;
              if(!viewInitialised)
              {
                focusedItem = 1;
                topItem = 1;
                viewInitialised = true;
              }
              
              for(uint8_t line = 0; line < 4 && line < numItems; line++)
              {
                uint8_t ypos = 18 + line * 9;
                uint8_t item = topItem + line;
                if(focusedItem == item)
                  drawCursor(22, ypos);
                
                display.setCursor(0, ypos);
                uint8_t idx = item - 1;
                getSrcName(textBuff, SRC_STICK_AXIS_FIRST + idx, sizeof(textBuff));
                display.print(textBuff);
                display.print(F(":"));
                
                display.setCursor(30, ypos);
                display.print(findStringInIdStr(enum_StickAxisType, Sys.StickAxis[idx].type));
              }
              
              //Draw scroll bar
              drawScrollBar(127, 9, numItems, topItem, 4, 44);
              
              //'next' or 'finish' button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              if(!allAxesAbsent || lastScreen == SCREEN_HOME)
              {
                display.setCursor(90, 56);
                display.print(F("[Next]"));
                if(focusedItem == numItems + 1)
                  drawCursor(82, 56);
              }
              else
              {
                display.setCursor(78, 56);
                display.print(F("[Finish]"));
                if(focusedItem == numItems + 1)
                  drawCursor(70, 56);
              }
              
              //handle navigation
              changeFocusOnUpDown(numItems + 1); //+1 for button focus
              if(focusedItem < topItem)
                topItem = focusedItem;
              while(focusedItem >= topItem + 4 && focusedItem < numItems + 1)
                topItem++;
              toggleEditModeOnSelectClicked();
              
              //edit parameters
              uint8_t idx = focusedItem - 1;
              if(idx < numItems)
                Sys.StickAxis[idx].type = incDec(Sys.StickAxis[idx].type, 0, STICK_AXIS_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              
              if(focusedItem == numItems + 1 && isEditMode)
              {
                isEditMode = false;
                focusedItem = 1;
                viewInitialised = false;
                if(allAxesAbsent)
                {
                  initialised = false;
                  isRequestingStickCalibration = false;
                  isCalibratingControls = false;
                  if(lastScreen == SCREEN_HOME)
                    changeToScreen(SCREEN_HOME);
                }
                else
                {
                  calibrateSticks(CALIBRATE_INIT);
                  page = PAGE_MOVE_STICKS;
                }
              }
            }
            break;
            
          case PAGE_MOVE_STICKS:
            {
              if(hasSelfCenteringAxes)
                printFullScreenMessage(PSTR("Move all stick axes\nfully, then center\nthem. Proceed\nwhen done."));
              else
                printFullScreenMessage(PSTR("Move all stick axes\nfully. Proceed\nwhen done."));
                
              calibrateSticks(CALIBRATE_MOVE);

              //'next' or 'finish' button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              if(hasSelfCenteringAxes || lastScreen == SCREEN_HOME)
              {
                display.setCursor(90, 56);
                display.print(F("[Next]"));
                drawCursor(82, 56);
              }
              else
              {
                display.setCursor(78, 56);
                display.print(F("[Finish]"));
                drawCursor(70, 56);
              }
              
              if(clickedButton == KEY_SELECT)
              {
                //add dead band to stick extremes
                calibrateSticks(CALIBRATE_DEADBAND); 
                isCalibratingControls = false;
                
                if(hasSelfCenteringAxes)
                  page = PAGE_ADJUST_DEADZONE;
                else
                {
                  initialised = false;
                  isRequestingStickCalibration = false;
                  if(lastScreen == SCREEN_HOME)
                    changeToScreen(SCREEN_HOME);
                }
              }
            }
            break;
            
          case PAGE_ADJUST_DEADZONE:
            {
              display.setCursor(0, 9);
              display.print(F("Adjust deadzone"));
              
              //scrollable list
              
              static uint8_t topItem;
              static bool viewInitialised = false;
              if(!viewInitialised)
              {
                focusedItem = 1;
                topItem = 1;
                viewInitialised = true;
              }
              
              for(uint8_t line = 0; line < 4 && line < numItems; line++)
              {
                uint8_t ypos = 18 + line * 9;
                uint8_t item = topItem + line;
                if(focusedItem == item)
                  drawCursor(22, ypos);
                
                uint8_t idx = item - 1;

                display.setCursor(0, ypos);
                getSrcName(textBuff, SRC_STICK_AXIS_FIRST + idx, sizeof(textBuff));
                display.print(textBuff);
                display.print(F(":"));
                
                display.setCursor(30, ypos);
                if(Sys.StickAxis[idx].type == STICK_AXIS_SELF_CENTERING)
                {
                  display.print(Sys.StickAxis[idx].deadzone);
                  display.print(F("%"));
                }
                else
                  display.print(F("N/A"));
                
                display.setCursor(66, ypos);
                display.print(F("("));
                display.print(stickAxisIn[idx]/5);
                display.print(F(")"));
              }
              
              //Draw scroll bar
              drawScrollBar(127, 9, numItems, topItem, 4, 44);
              
              //'next' or 'finish' button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              if(lastScreen == SCREEN_HOME)
              {
                display.setCursor(90, 56);
                display.print(F("[Next]"));
                if(focusedItem == numItems + 1)
                  drawCursor(82, 56);
              }
              else 
              {
                display.setCursor(78, 56);
                display.print(F("[Finish]"));
                if(focusedItem == numItems + 1)
                  drawCursor(70, 56);
              }
              
              //handle navigation
              changeFocusOnUpDown(numItems + 1); //+1 for button focus
              if(focusedItem < topItem)
                topItem = focusedItem;
              while(focusedItem >= topItem + 4 && focusedItem < numItems + 1)
                topItem++;
              toggleEditModeOnSelectClicked();
              
              //edit parameters
              uint8_t idx = focusedItem - 1;
              if(idx < numItems && Sys.StickAxis[idx].type == STICK_AXIS_SELF_CENTERING)
                Sys.StickAxis[idx].deadzone = incDec(Sys.StickAxis[idx].deadzone, 0, 15, INCDEC_NOWRAP, INCDEC_SLOW);
              
              if(focusedItem == numItems + 1 && isEditMode)
              {
                //exit calibration
                viewInitialised = false;
                isRequestingStickCalibration = false;
                initialised = false;
                if(lastScreen == SCREEN_HOME)
                  changeToScreen(SCREEN_HOME);
              }
            }
            break;
        }
      }
      break;
      
    case SCREEN_KNOBS:
      {
        drawHeader(advancedMenu[ADVANCED_MENU_KNOBS]);

        enum {
          PAGE_START,
          PAGE_KNOB_TYPE,
          PAGE_MOVE_KNOBS, 
          PAGE_ADJUST_DEADZONE
        };
        
        static uint8_t page = PAGE_START;
        static bool initialised = false;

        if(!initialised)
        {
          initialised = true;
          focusedItem = 1;
          isEditMode = false;
          page = (lastScreen == SCREEN_HOME) ? PAGE_KNOB_TYPE : PAGE_START;
        }

        uint8_t numItems = NUM_KNOBS;
        
        bool allKnobsAbsent = false;
        bool hasCenterDetentKnobs = false;
        uint8_t cntrQQ = 0;
        for(uint8_t i = 0; i < NUM_KNOBS; i++)
        {
          if(Sys.Knob[i].type == KNOB_ABSENT)
            cntrQQ++;
          else if(Sys.Knob[i].type == KNOB_CENTER_DETENT)
            hasCenterDetentKnobs = true;
        }
        if(cntrQQ == NUM_KNOBS)
          allKnobsAbsent = true;

        switch(page)
        {
          case PAGE_START:
            {
              display.setCursor(0, 9);
              display.print(F("Calibrtn:"));
              display.setCursor(78, 9);
              display.print(F("[Start]"));
              
              display.setCursor(0, 18);
              display.print(F("Deadzone:"));
              display.setCursor(78, 18);
              display.print(F("[Adjust]"));
              
              drawCursor(70, focusedItem * 9);
              
              changeFocusOnUpDown(2);
              toggleEditModeOnSelectClicked();
              
              if(focusedItem == 1 && isEditMode)
              {
                page = PAGE_KNOB_TYPE;
                isEditMode = false;
              }
              else if(focusedItem == 2 && isEditMode)
              {
                page = PAGE_ADJUST_DEADZONE;
                isEditMode = false;
              }
              
              if(heldButton == KEY_SELECT)
              {
                initialised = false;
                changeToScreen(SCREEN_ADVANCED_MENU);
              }
            }
            break;
          
          case PAGE_KNOB_TYPE:
            {
              isCalibratingControls = true;
              
              display.setCursor(0, 9);
              display.print(F("Type of knob"));
              
              //scrollable list
              
              static uint8_t topItem;
              static bool viewInitialised = false;
              if(!viewInitialised)
              {
                focusedItem = 1;
                topItem = 1;
                viewInitialised = true;
              }
              
              for(uint8_t line = 0; line < 4 && line < numItems; line++)
              {
                uint8_t ypos = 18 + line * 9;
                uint8_t item = topItem + line;
                if(focusedItem == item)
                  drawCursor(40, ypos);
                
                display.setCursor(0, ypos);
                uint8_t idx = item - 1;
                getSrcName(textBuff, SRC_KNOB_FIRST + idx, sizeof(textBuff));
                display.print(textBuff);
                display.print(F(":"));
                
                display.setCursor(48, ypos);
                display.print(findStringInIdStr(enum_KnobType, Sys.Knob[idx].type));
              }
              
              //Draw scroll bar
              drawScrollBar(127, 9, numItems, topItem, 4, 44);
              
              //'next' or 'finish' button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              if(!allKnobsAbsent || lastScreen == SCREEN_HOME)
              {
                display.setCursor(90, 56);
                display.print(F("[Next]"));
                if(focusedItem == numItems + 1)
                  drawCursor(82, 56);
              }
              else
              {
                display.setCursor(78, 56);
                display.print(F("[Finish]"));
                if(focusedItem == numItems + 1)
                  drawCursor(70, 56);
              }
              
              //handle navigation
              changeFocusOnUpDown(numItems + 1); //+1 for button focus
              if(focusedItem < topItem)
                topItem = focusedItem;
              while(focusedItem >= topItem + 4 && focusedItem < numItems + 1)
                topItem++;
              toggleEditModeOnSelectClicked();
              
              //edit parameters
              uint8_t idx = focusedItem - 1;
              if(idx < numItems)
                Sys.Knob[idx].type = incDec(Sys.Knob[idx].type, 0, KNOB_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              
              if(focusedItem == numItems + 1 && isEditMode)
              {
                isEditMode = false;
                focusedItem = 1;
                viewInitialised = false;
                if(allKnobsAbsent)
                {
                  initialised = false;
                  isRequestingKnobCalibration = false;
                  isCalibratingControls = false;
                  if(lastScreen == SCREEN_HOME)
                    changeToScreen(SCREEN_HOME);
                }
                else
                {
                  calibrateKnobs(CALIBRATE_INIT);
                  page = PAGE_MOVE_KNOBS;
                }
              }
            }
            break;
            
          case PAGE_MOVE_KNOBS:
            {
              if(hasCenterDetentKnobs)
                printFullScreenMessage(PSTR("Move all knobs\nfully, then center\nthem. Proceed\nwhen done."));
              else
                printFullScreenMessage(PSTR("Move all knobs\nfully. Proceed\nwhen done."));
              
              calibrateKnobs(CALIBRATE_MOVE);
              
              //'next' or 'finish' button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              if(hasCenterDetentKnobs || lastScreen == SCREEN_HOME)
              {
                display.setCursor(90, 56);
                display.print(F("[Next]"));
                drawCursor(82, 56);
              }
              else
              {
                display.setCursor(78, 56);
                display.print(F("[Finish]"));
                drawCursor(70, 56);
              }
              
              if(clickedButton == KEY_SELECT)
              {
                //add dead band to extremes
                calibrateKnobs(CALIBRATE_DEADBAND); 
                isCalibratingControls = false;
                
                if(hasCenterDetentKnobs)
                  page = PAGE_ADJUST_DEADZONE;
                else
                {
                  initialised = false;
                  isRequestingKnobCalibration = false;
                  if(lastScreen == SCREEN_HOME)
                    changeToScreen(SCREEN_HOME);
                }
              }
            }
            break;
            
          case PAGE_ADJUST_DEADZONE:
            {
              display.setCursor(0, 9);
              display.print(F("Adjust deadzone"));
              
              //scrollable list
              
              static uint8_t topItem;
              static bool viewInitialised = false;
              if(!viewInitialised)
              {
                focusedItem = 1;
                topItem = 1;
                viewInitialised = true;
              }
              
              for(uint8_t line = 0; line < 4 && line < numItems; line++)
              {
                uint8_t ypos = 18 + line * 9;
                uint8_t item = topItem + line;
                if(focusedItem == item)
                  drawCursor(40, ypos);
                
                uint8_t idx = item - 1;

                display.setCursor(0, ypos);
                getSrcName(textBuff, SRC_KNOB_FIRST + idx, sizeof(textBuff));
                display.print(textBuff);
                display.print(F(":"));
                
                display.setCursor(48, ypos);
                if(Sys.Knob[idx].type == KNOB_CENTER_DETENT)
                {
                  display.print(Sys.Knob[idx].deadzone);
                  display.print(F("%"));
                }
                else
                  display.print(F("N/A"));
                
                display.setCursor(84, ypos);
                display.print(F("("));
                display.print(knobIn[idx]/5);
                display.print(F(")"));
              }
              
              //Draw scroll bar
              drawScrollBar(127, 9, numItems, topItem, 4, 44);
              
              //'next' or 'finish' button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              if(lastScreen == SCREEN_HOME)
              {
                display.setCursor(90, 56);
                display.print(F("[Next]"));
                if(focusedItem == numItems + 1)
                  drawCursor(82, 56);
              }
              else 
              {
                display.setCursor(78, 56);
                display.print(F("[Finish]"));
                if(focusedItem == numItems + 1)
                  drawCursor(70, 56);
              }
              
              //handle navigation
              changeFocusOnUpDown(numItems + 1); //+1 for button focus
              if(focusedItem < topItem)
                topItem = focusedItem;
              while(focusedItem >= topItem + 4 && focusedItem < numItems + 1)
                topItem++;
              toggleEditModeOnSelectClicked();
              
              //edit parameters
              uint8_t idx = focusedItem - 1;
              if(idx < numItems && Sys.Knob[idx].type == KNOB_CENTER_DETENT)
                Sys.Knob[idx].deadzone = incDec(Sys.Knob[idx].deadzone, 0, 15, INCDEC_NOWRAP, INCDEC_SLOW);
              
              if(focusedItem == numItems + 1 && isEditMode)
              {
                //exit calibration
                if(lastScreen == SCREEN_HOME)
                  changeToScreen(SCREEN_HOME);
                viewInitialised = false;
                isRequestingKnobCalibration = false;
                initialised = false;
              }
            }
            break;
        }
      }
      break;
      
    case SCREEN_SWITCHES:
      {
        drawHeader(advancedMenu[ADVANCED_MENU_SWITCHES]);

        //-- Scrollable list
        static uint8_t topItem;
        static bool showWarning = true;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          viewInitialised = true;
          showWarning = true;
          focusedItem = 1;
          topItem = 1;
        }
        
        bool hasNextButton = false;
        if(lastScreen == SCREEN_HOME)
        {
          showWarning = false;
          hasNextButton = true;
        }

        if(showWarning)
        {
          printFullScreenMessage(PSTR("\nThese settings might\naffect your existing\nmodels.\n\n[OK] to continue"));
          if(clickedButton == KEY_SELECT)
            showWarning = false;
          else if(heldButton == KEY_SELECT)
          {
            viewInitialised = false;
            changeToScreen(SCREEN_ADVANCED_MENU);
          }
          
          break;
        }

        //fill list
        uint8_t end = hasNextButton ? 5 : 6;
        for(uint8_t line = 0; line < end && line < NUM_PHYSICAL_SWITCHES; line++)
        {
          uint8_t ypos = 9 + line * 9;
          if(focusedItem == topItem + line)
            drawCursor(28, ypos);
          display.setCursor(0, ypos);
          uint8_t idx = line + topItem - 1;
          getSrcName(textBuff, SRC_SW_PHYSICAL_FIRST + idx, sizeof(textBuff));
          display.print(textBuff);
          display.print(F(":"));
          display.setCursor(36, ypos);
          display.print(findStringInIdStr(enum_SwitchType, Sys.swType[idx]));
        }
        
        //Draw scroll bar
        drawScrollBar(127, 9, NUM_PHYSICAL_SWITCHES, topItem, end, hasNextButton ? 44 : 54);
        
        //show the 'next' button
        if(hasNextButton)
        {
          drawDottedHLine(0, 54, 128, BLACK, WHITE);
          display.setCursor(90, 56);
          display.print(F("[Next]"));
          if(focusedItem == NUM_PHYSICAL_SWITCHES + 1)
            drawCursor(82, 56);
        }
        
        //Navigate
        changeFocusOnUpDown(hasNextButton ? NUM_PHYSICAL_SWITCHES + 1 : NUM_PHYSICAL_SWITCHES);
        toggleEditModeOnSelectClicked();
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + end && focusedItem < NUM_PHYSICAL_SWITCHES + 1)
          topItem++;
        
        //Edit items
        uint8_t idx = focusedItem - 1;
        if(idx < NUM_PHYSICAL_SWITCHES)
          Sys.swType[idx] = incDec(Sys.swType[idx], 0, SW_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);

        //exit
        if((heldButton == KEY_SELECT && !hasNextButton)
           || (focusedItem == NUM_PHYSICAL_SWITCHES + 1 && clickedButton == KEY_SELECT))
        {
          viewInitialised = false;
          changeToScreen(lastScreen);
          isRequestingSwitchesSetup = false;
        }
      }
      break;
    
    case SCREEN_BATTERY:
      {
        drawHeader(advancedMenu[ADVANCED_MENU_BATTERY]);
        
        bool hasNextButton = false;
        if(lastScreen == SCREEN_HOME)
          hasNextButton = true;

        display.setCursor(0, 9);
        display.print(F("Gauge min:"));
        display.setCursor(72, 9);
        printVoltage(Sys.batteryVoltsMin);

        display.setCursor(0, 18);
        display.print(F("Gauge max:"));
        display.setCursor(72, 18);
        printVoltage(Sys.batteryVoltsMax);
        
        display.setCursor(0, 27);
        display.print(F("Multplr:"));
        display.setCursor(72, 27);
        display.print(Sys.batteryCalibrationFactor);
        
        display.setCursor(0, hasNextButton ? 45 : 56);
        display.print(F("("));
        printVoltage(batteryVoltsNow);
        display.print(F(")"));
        
        if(hasNextButton)
        {
          drawDottedHLine(0, 54, 128, BLACK, WHITE);
          display.setCursor(90, 56);
          display.print(F("[Next]"));
          if(focusedItem == 4)
            drawCursor(82, 56);
        }
        
        if(focusedItem < 4)
          drawCursor(64, focusedItem * 9);
        
        changeFocusOnUpDown(hasNextButton ? 4 : 3);
        toggleEditModeOnSelectClicked();
        
        //edit items
        if(focusedItem == 1)
        {
          //scale down by 1/10 since we display only 2 decimals
          int16_t val = Sys.batteryVoltsMin / 10;
          int16_t min = 300;
          int16_t max = (Sys.batteryVoltsMax - 100) / 10;
          //inc dec
          val = incDec(val, min, max, INCDEC_NOWRAP, INCDEC_NORMAL);
          //scale back
          Sys.batteryVoltsMin = val * 10;
        }
        else if(focusedItem == 2)
        {
          //scale down by 1/10 since we display only 2 decimals
          int16_t val = Sys.batteryVoltsMax / 10;
          int16_t min = (Sys.batteryVoltsMin + 100) / 10;
          int16_t max = 1500;
          //inc dec
          val = incDec(val, min, max, INCDEC_NOWRAP, INCDEC_NORMAL);
          //scale back
          Sys.batteryVoltsMax = val * 10;
        }
        else if(focusedItem == 3)
          Sys.batteryCalibrationFactor = incDec(Sys.batteryCalibrationFactor, 0, 2000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);

        //exit
        if((heldButton == KEY_SELECT && !hasNextButton) || (focusedItem == 4 && clickedButton == KEY_SELECT))
        {
          batteryGaugeCalibrated = true;
          changeToScreen(lastScreen);
        }
      }
      break;
      
    case SCREEN_SECURITY:
      {
        if(isEditTextDialog)
        {
          editTextDialog(PSTR("Password"), Sys.password, sizeof(Sys.password), true, false, false);
          break;
        }

        drawHeader(advancedMenu[ADVANCED_MENU_SECURITY]);
        
        drawCheckbox(8, 9, Sys.lockModels);
        display.setCursor(18, 9);
        display.print(F("Lock models"));

        drawCheckbox(8, 18, Sys.lockStartup);
        display.setCursor(18, 18);
        display.print(F("Lock startup"));

        drawCheckbox(8, 27, Sys.lockOnInactivity);
        display.setCursor(18, 27);
        display.print(F("Lock on inactvty"));
        
        display.setCursor(8, 36);
        if(isEmptyStr(Sys.password, sizeof(Sys.password)))
          display.print(F("[Set password]"));
        else
          display.print(F("[Change password]"));

        changeFocusOnUpDown(4);
        toggleEditModeOnSelectClicked();
        drawCursor(0, focusedItem * 9);
        
        if(focusedItem == 1)
          Sys.lockModels = incDec(Sys.lockModels, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 2 && isEditMode)
        {
          Sys.lockStartup = incDec(Sys.lockStartup, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
          if(!Sys.lockStartup)
            Sys.lockOnInactivity = false;
        }
        else if(focusedItem == 3 && isEditMode)
        {
          Sys.lockOnInactivity = incDec(Sys.lockOnInactivity, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
          if(Sys.lockOnInactivity)
            Sys.lockStartup = true;
        }
        else if(focusedItem == 4 && isEditMode)
          isEditTextDialog = true;

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_ADVANCED_MENU);
      }
      break;
    
    case SCREEN_DEBUG:
      {
        drawHeader(advancedMenu[ADVANCED_MENU_DEBUG]);
 
        enum {
          ITEM_VIEW_STATISTICS,
          ITEM_SHOW_CHARACTER_SET,
          ITEM_CONFIGURE_SCREENSHOTS,
          ITEM_SHOW_LOOP_TIME,
          ITEM_DISABLE_INTERLACING,
          ITEM_SIMULATE_TELEMETRY,
          ITEM_ADJUST_LONG_PRESS_DELAY,
          ITEM_ADJUST_KEY_REPEAT_INTERVAL,
          ITEM_DUMP_INTERNAL_EEPROM,
          ITEM_DUMP_EXTERNAL_EEPROM,
          ITEM_BACKUP_SYSTEM_SETTINGS,
          ITEM_RESTORE_SYSTEM_SETTINGS,
          ITEM_FACTORY_RESET,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(i == ITEM_DUMP_EXTERNAL_EEPROM && !eeHasExternalEE())
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        //initialise
        static uint8_t topItem;
        static uint8_t lastFocusedItem;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = 1;
          lastFocusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }
        
        //handle navigation
        focusedItem = lastFocusedItem;
        changeFocusOnUpDown(listItemCount);
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 6)
          topItem++;
        lastFocusedItem = focusedItem;
        
        //fill list and edit items
        for(uint8_t line = 0; line < 6 && line < listItemCount; line++)
        {
          uint8_t ypos = 9 + line*9;
          if(focusedItem == topItem + line)
            drawCursor(0, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (itemID == listItemIDs[focusedItem - 1]);
          
          display.setCursor(8, ypos);
          switch(itemID)
          {
            case ITEM_VIEW_STATISTICS:
              {
                display.print(F("[View statistics]"));
                if(isFocused && clickedButton == KEY_SELECT)
                  changeToScreen(SCREEN_DEBUG_STATISTICS);
              }
              break;

            case ITEM_SHOW_LOOP_TIME:
              {
                drawCheckbox(display.getCursorX(), ypos, Sys.showLoopTime);
                display.setCursor(display.getCursorX() + 10, ypos);
                display.print(F("Show loop time"));
                if(isFocused)
                {
                  toggleEditModeOnSelectClicked();
                  Sys.showLoopTime = incDec(Sys.showLoopTime, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                }
              }
              break;

            case ITEM_SIMULATE_TELEMETRY:
              {
                drawCheckbox(display.getCursorX(), ypos, Sys.simulateTelemetry);
                display.setCursor(display.getCursorX() + 10, ypos);
                display.print(F("Simulate telemetry"));
                if(isFocused)
                {
                  toggleEditModeOnSelectClicked();
                  bool lastState = Sys.simulateTelemetry;
                  Sys.simulateTelemetry = incDec(Sys.simulateTelemetry, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                  if(!lastState && Sys.simulateTelemetry)
                    makeToast(PSTR("ID:0x30, Src:Virt1"), 2000, 300);
                }
              }
              break;
              
            case ITEM_DISABLE_INTERLACING:
              {
                drawCheckbox(display.getCursorX(), ypos, Sys.disableInterlacing);
                display.setCursor(display.getCursorX() + 10, ypos);
                display.print(F("No LCD interlacing"));
                if(isFocused)
                {
                  toggleEditModeOnSelectClicked();
                  bool lastState = Sys.disableInterlacing;
                  Sys.disableInterlacing = incDec(Sys.disableInterlacing, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                  if(!lastState && Sys.disableInterlacing)
                    makeToast(PSTR("May reduce performnc"), 2000, 300);
                }
              }
              break;

            case ITEM_ADJUST_LONG_PRESS_DELAY: 
              {
                display.print(F("[Long press delay]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                  changeToScreen(DIALOG_ADJUST_LONG_PRESS_DELAY);
              }
              break;

            case ITEM_ADJUST_KEY_REPEAT_INTERVAL: 
              {
                display.print(F("[Key repeat intrvl]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                  changeToScreen(DIALOG_ADJUST_KEY_REPEAT_INTERVAL);
              }
              break;
              
            case ITEM_DUMP_INTERNAL_EEPROM:
              {
                display.print(F("[Dump intnl EEPROM]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                  changeToScreen(SCREEN_INTERNAL_EEPROM_DUMP);
              }
              break;
              
            case ITEM_DUMP_EXTERNAL_EEPROM: 
              {
                display.print(F("[Dump extnl EEPROM]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                  changeToScreen(SCREEN_EXTERNAL_EEPROM_DUMP);
              }
              break;
              
            case ITEM_SHOW_CHARACTER_SET:
              {
                display.print(F("[View char set]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                  changeToScreen(SCREEN_CHARACTER_SET);
              }
              break;
              
            case ITEM_CONFIGURE_SCREENSHOTS:
              {
                display.print(F("[Screenshot config]"));
                if(isFocused && clickedButton == KEY_SELECT)
                {
                  if(sdHasCard())
                    changeToScreen(SCREEN_SCREENSHOT_CONFIG);
                  else
                    makeToast(PSTR("SD card not found"), 2000, 0);
                }
              }
              break;

            case ITEM_BACKUP_SYSTEM_SETTINGS:
              {
                display.print(F("[Back up settings]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                {
                  if(sdHasCard())
                    changeToScreen(CONFIRMATION_BACKUP_SYSTEM_SETTINGS);
                  else
                    makeToast(PSTR("SD card not found"), 2000, 0);
                }
              }
              break;

            case ITEM_RESTORE_SYSTEM_SETTINGS:
              {
                display.print(F("[Restore settings]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                {
                  if(sdHasCard())
                    changeToScreen(CONFIRMATION_RESTORE_SYSTEM_SETTINGS);
                  else
                    makeToast(PSTR("SD card not found"), 2000, 0);
                }
              }
              break;
              
            case ITEM_FACTORY_RESET:
              {
                display.print(F("[Factory reset]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                  changeToScreen(CONFIRMATION_FACTORY_RESET);
              }
              break;
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 9, listItemCount, topItem, 6, 6 * 9);
        
        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_ADVANCED_MENU);
          viewInitialised = false;
        }
      }
      break;

    case SCREEN_DEBUG_STATISTICS:
      {
        drawHeader(PSTR("Statistics"));

        telemetryForceRequest = true;

        enum {
          ITEM_PACKET_RATE,
          ITEM_UP_TIME,
          ITEM_LOOP_NUM,
          ITEM_FIXED_LOOP_TIME,
          ITEM_INACTIVITY_TIME,
          ITEM_FREE_RAM,
          
          ITEM_COUNT
        };

        //initialise
        static uint8_t thisPage = 1;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          thisPage = 1;
          viewInitialised = true;
        }

        //handle navigation
        uint8_t numPages = (ITEM_COUNT + 5) / 6;
        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);

        //fill list
        for(uint8_t line = 0; line < 6 && line < ITEM_COUNT; line++)
        {
          uint8_t ypos = 9 + line * 9;
          uint8_t itemID = (thisPage - 1) * 6 + line;
          if(itemID >= ITEM_COUNT)
            break;

          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_PACKET_RATE:
              {
                display.print(F("Pkt rate:"));
                display.setCursor(66, ypos);
                display.print(transmitterPacketRate);
                display.print(F(", "));
                display.print(receiverPacketRate);
              }
              break;
            
            case ITEM_UP_TIME:
              {
                display.print(F("Up time:"));
                display.setCursor(66, ypos);
                printHHMMSS(millis());
              }
              break;
            
            case ITEM_INACTIVITY_TIME:
              {
                display.print(F("Inactvty:"));
                display.setCursor(66, ypos);
                printHHMMSS(millis() - inputsLastMovedTime);
              }
              break;
            
            case ITEM_FREE_RAM:
              {
                display.print(F("Free RAM:"));
                display.setCursor(66, ypos);
                display.print(getFreeRam());
                display.setCursor(display.getCursorX() + 3, ypos);
                display.print(F("bytes"));
              }
              break;
            
            case ITEM_LOOP_NUM:
              {
                display.print(F("Loop #:"));
                display.setCursor(66, ypos);
                display.print(thisLoopNum);
              }
              break;
            
            case ITEM_FIXED_LOOP_TIME:
              {
                display.print(F("Fxd loop:"));
                display.setCursor(66, ypos);
                display.print(fixedLoopTime);
                display.setCursor(display.getCursorX() + 3, ypos);
                display.print(F("ms"));
              }
              break;

          }
        }

        //Draw scroll bar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        //exit
        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_DEBUG);
          telemetryForceRequest = false;
        }
      }
      break;
      
    case SCREEN_INTERNAL_EEPROM_DUMP:
    case SCREEN_EXTERNAL_EEPROM_DUMP:
      {
        //We read 32 bytes at a time and cache them, only updating when the view gets scrolled.
        int16_t numPages = eeInternalEEGetSize() / 32;
        if(theScreen == SCREEN_EXTERNAL_EEPROM_DUMP)
          numPages = eeExternalEEGetSize() / 32;
        
        static int16_t page = 0; 
        
        static bool viewInitialised = false;
        static bool needsUpdate = true;
        if(!viewInitialised)
        {
          viewInitialised = true;
          needsUpdate = true;
          page = 0;
          //save data first at once, as there could be pending writes
          showWaitMessage();
          stopTones();
          eeSaveModelData(Sys.activeModelIdx);
          eeSaveSysConfig();
        }
        
        //handle navigation
        int16_t lastPage = page;
        isEditMode = true;
        page = incDec(page, numPages - 1, 0, INCDEC_WRAP, INCDEC_SLOW, INCDEC_NORMAL);
        if(page != lastPage)
          needsUpdate = true;
        
        //fill buffer
        if(needsUpdate)
        {
          needsUpdate = false;
          for(uint8_t idx = 0; idx < 32 && idx < sizeof(textBuff); idx++)
          {
            uint32_t addr = ((int32_t)page * 32) + idx;
            if(theScreen == SCREEN_EXTERNAL_EEPROM_DUMP)
              textBuff[idx] = eeExternalEEReadByte(addr);
            else
              textBuff[idx] = eeInternalEEReadByte(addr);
          }
        }

        //--- draw
        
        display.drawVLine(32, 0, 64, BLACK);
        display.drawVLine(95, 0, 64, BLACK);
        
        for(uint8_t line = 0; line < 8; line++)
        {
          //--- show offsets in decimal
          display.setCursor(0, line*8);
          //count the digits of the address
          uint8_t digits = 1;
          uint32_t addr = ((int32_t)page * 32) + line * 4; //4 bytes per line
          uint32_t num = addr;          
          while(num >= 10)
          {
            num /= 10;
            digits++;
          }
          //print leading zeros
          while(digits < 5)
          {
            display.print(F("0"));
            digits++;
          }
          //print
          display.print(addr); 
          
          //--- show bytes
          for(uint8_t col = 0; col < 4; col++)
          {
            display.setCursor(36 + col * 15, line * 8);
            uint8_t idx = (line * 4) + col;
            uint8_t val = textBuff[idx];
            if(val < 0x10)
              display.print(F("0"));
            display.print(val, 16); //print as hex
            
            //show the characters
            display.setCursor(99 + col * 6, line * 8);
            if(val == 0x00 || val == 0x20 || val == 0xFF || (val >= 0x0A && val <= 0x0D))
              display.write(0x2E);
            else
              display.write(val);
          }
        }
        
        //scrollbar
        drawScrollBar(127, 0, numPages, page + 1, 1, 1 * 64);
        
        //Exit
        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_DEBUG);
        }
      }
      break;
      
    case SCREEN_CHARACTER_SET:
      {
        display.drawVLine(9, 0, 64, BLACK);
        display.drawHLine(0, 9, 128, BLACK);
        //show horizontal markers
        for(uint8_t i = 0; i < 16; i++)
        {
          display.setCursor(12 + i*7, 1);
          display.print(i, 16); //as hex
        }
        
        const uint8_t itemsPerPage = 16 * 6;
        const uint8_t numPages = 3;
        static uint8_t thisPage = 1;
        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        uint16_t startIdx = (thisPage - 1) * itemsPerPage;
        for(uint16_t i = startIdx; i < startIdx + itemsPerPage && i < 256; i++)
        {
          uint8_t xOffset = 7 * ((i - startIdx) % 16);
          uint8_t yOffset = 9 * ((i - startIdx) / 16);
          display.setCursor(12 + xOffset, 11 + yOffset);
          display.write(i);
          //show vertical markers
          if(xOffset == 0)
          {
            display.setCursor(2, 11 + yOffset);
            display.print(i/16, 16); //as hex
          }
        }
        
        //show scrollbar
        drawScrollBar(127, 11, numPages, thisPage, 1, 1 * 53);
        
        //Exit
        if(heldButton == KEY_SELECT)
        {
          thisPage = 1;
          changeToScreen(SCREEN_DEBUG);
        }
      }
      break;
      
    case SCREEN_SCREENSHOT_CONFIG:
      {
        drawHeader(PSTR("Screenshot config"));
        
        //use a temporary variable for the swtch and assign later 
        //to avoid unintended action while scrolling through the options
        static uint8_t tempSwtch;
        static bool tempInitialised = false;
        if(!tempInitialised)
        {
          tempSwtch = screenshotSwtch;
          tempInitialised = true;
        }
        
        display.setCursor(0, 9);
        display.print(F("Switch:"));
        display.setCursor(66, 9);
        getControlSwitchName(textBuff, tempSwtch, sizeof(textBuff));
        display.print(textBuff);
        
        display.setCursor(0, 46);
        display.print(F("(These settings are\ntemporary.)"));
        
        drawCursor(58, focusedItem * 9);
        
        changeFocusOnUpDown(1);
        toggleEditModeOnSelectClicked();
        
        if(focusedItem == 1)
          tempSwtch = incDecControlSwitch(tempSwtch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_TRIM_AS_SW);
        
        //assign from temp
        if(!isEditMode || (buttonCode == 0 && millis() - buttonReleaseTime >= 1000))
        {
          screenshotSwtch = tempSwtch;
          tempInitialised = false;
        }
          
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_DEBUG);
          //assign from temp
          screenshotSwtch = tempSwtch;
          tempInitialised = false;
        }
      }
      break;

    case DIALOG_ADJUST_LONG_PRESS_DELAY:
      {
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Long press delay"));
        display.setCursor(47, 28);
        display.print(Sys.longPressDelay);
        display.setCursor(display.getCursorX() + 3, 28);
        display.print(F("ms"));
        drawCursor(39, 28);

        isEditMode = true;
        Sys.longPressDelay = incDec(Sys.longPressDelay, LONG_PRESS_DELAY_MIN, LONG_PRESS_DELAY_MAX, INCDEC_NOWRAP, INCDEC_NORMAL);

        if(clickedButton == KEY_SELECT || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_DEBUG);
      }
      break;

    case DIALOG_ADJUST_KEY_REPEAT_INTERVAL:
      {
        drawBoundingBox(2, 14, 124, 35,BLACK);
        display.setCursor(7, 18);
        display.print(F("Key repeat interval"));
        display.setCursor(47, 28);
        display.print(Sys.keyRepeatInterval);
        display.setCursor(display.getCursorX() + 3, 28);
        display.print(F("ms"));
        drawCursor(39, 28);

        isEditMode = true;
        Sys.keyRepeatInterval = incDec(Sys.keyRepeatInterval, KEY_REPEAT_INTERVAL_MIN, KEY_REPEAT_INTERVAL_MAX, INCDEC_NOWRAP, INCDEC_NORMAL);

        if(clickedButton == KEY_SELECT || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_DEBUG);
      }
      break;

    case CONFIRMATION_BACKUP_SYSTEM_SETTINGS:
      {
        static bool initialised = false;
        static bool fileExists = false;
        if(!initialised)
        {
          fileExists = sdSystemSettingsExists();
          initialised = true;
        }
        if(fileExists)
          printFullScreenMessage(PSTR("The backup file\nalready exists.\nOverwrite it?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP || !fileExists)
        {
          showWaitMessage();
          stopTones();
          if(!sdBackupSystemSettings())
            makeToast(PSTR("Back up failed"), 2000, 0);
          initialised = false;
          changeToScreen(SCREEN_DEBUG);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
        {
          initialised = false;
          changeToScreen(SCREEN_DEBUG);
        }
      }
      break;

    case CONFIRMATION_RESTORE_SYSTEM_SETTINGS:
      {
        if(lastScreen == SCREEN_HOME)
          printFullScreenMessage(PSTR("Restore system\nsettings from\nthe backup file?\n\nYes [Up] \nNo [Down]"));
        else
          printFullScreenMessage(PSTR("System settings will\nbe overwritten.\nContinue?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          showWaitMessage();
          stopTones();
          eeSaveSysConfig(); //save first to EEPROM to be able to revert changes
          if(sdRestoreSystemSettings())
          {
            //reset flags coming from initial setup
            isRequestingSettingsRestore = false;
            isRequestingStickCalibration = false;
            isRequestingKnobCalibration = false;
            isRequestingSwitchesSetup = false;
            batteryGaugeCalibrated = true;

            //save to EEPROM
            eeSaveSysConfig();

            //show message if errors were encountered in the imported data
            if(dbgTotalErrorLines > 0)
            {
              display.clearDisplay();
              
              //print the items center aligned
              
              strlcpy_P(textBuff, PSTR("Some data skipped"), sizeof(textBuff));
              display.setCursor((display.width() - strlen(textBuff) * 6) / 2, 11);
              display.print(textBuff);
              
              char temp[6];
              utoa(dbgTotalErrorLines, temp, 10);
              strlcpy(textBuff, temp, sizeof(textBuff));
              strlcat_P(textBuff, PSTR(" errors,"), sizeof(textBuff));
              display.setCursor((display.width() - strlen(textBuff) * 6) / 2, 29);
              display.print(textBuff);
              
              strlcpy_P(textBuff, PSTR("first error at"), sizeof(textBuff));
              display.setCursor((display.width() - strlen(textBuff) * 6) / 2, 37);
              display.print(textBuff);
              
              strlcpy_P(textBuff, PSTR("line "), sizeof(textBuff));
              utoa(dbgFirstErrorLineNumber, temp, 10);
              strlcat(textBuff, temp, sizeof(textBuff));
              display.setCursor((display.width() - strlen(textBuff) * 6) / 2, 46);
              display.print(textBuff);
              
              display.setInterlace(false);
              display.display();
              
              //briefly block, then self-dismiss the warning
              uint32_t startTime = millis();
              while(1)
              {
                readSwitchesAndButtons();
                determineButtonEvent();
                playTones();
                handlePowerOff();
                if(millis() - startTime > 5000 || clickedButton == KEY_SELECT)
                {
                  killButtonEvents();
                  break;
                }
                delay(10);
              }
            }
          }
          else
          {
            eeReadSysConfig(); //revert changes
            if(!sdSystemSettingsExists())
              makeToast(PSTR("File not found"), 2000, 0);
            else
              makeToast(PSTR("Restore failed"), 2000, 0);
          }
          isRequestingSettingsRestore = false;
          changeToScreen(lastScreen);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
        {
          isRequestingSettingsRestore = false;
          changeToScreen(lastScreen);
        }
      }
      break;
      
    case CONFIRMATION_FACTORY_RESET:
      {
        printFullScreenMessage(PSTR("Factory reset will\nerase all data,\nincluding models."
                                "\nPress [UP]\nrepeatedly\nto confirm."));
        //trigger action
        static uint8_t cntr = 0;
        if(pressedButton == KEY_UP)
          cntr++;
        if(cntr >= 5)
        {
          stopTones();
          eeFactoryReset(); //implicitly calls the function that shows the progress
          while(1) //blocking
          {
            delay(10);
            readSwitchesAndButtons();
            determineButtonEvent();
            handlePowerOff();
            checkBattery();
            handleBatteryWarningUI();
            if(isDisplayingBatteryWarning)
              continue;
            inactivityAlarmHandler();
            playTones();
            display.clearDisplay();
            printFullScreenMessage(PSTR("All data has\nbeen erased.\nReboot to continue"));
            display.display();
          }
        }

        if(millis() - buttonStartTime > 500)
          cntr = 0;
        
        //show a graphical progress bar
        uint8_t w = ((uint16_t) 128 * cntr) / 5;
        display.fillRect(0, 61, w, 3, BLACK);

        if(heldButton == KEY_SELECT)
        {
          cntr = 0;
          changeToScreen(SCREEN_DEBUG);
        }
      } 
      break;
      
    case SCREEN_ABOUT:
      {
        drawHeader_Menu(systemMenu[SYSTEM_MENU_ABOUT]);
        
        static uint8_t topItem = 1, highlightedItem = 1;
        
        enum {
          ITEM_VERSION_INFO,
          ITEM_DISCLAIMER,
          ITEM_THIRD_PARTY_NOTICES,
          ITEM_EASTER_EGG,
        };

        menuInitialise();
        menuAddItem(PSTR("Version info"), ITEM_VERSION_INFO, NULL);
        menuAddItem(PSTR("Disclaimer"), ITEM_DISCLAIMER, NULL);
        menuAddItem(PSTR("Third party notices"), ITEM_THIRD_PARTY_NOTICES, NULL);
        menuAddItem(PSTR(""), ITEM_EASTER_EGG, NULL);
        menuDraw(&topItem, &highlightedItem);

        if(menuSelectedItemID == ITEM_VERSION_INFO)
        {
          textViewerText = versionText;
          changeToScreen(SCREEN_TEXT_VIEWER);
        }
        else if(menuSelectedItemID == ITEM_DISCLAIMER)
        {
          textViewerText = disclaimerText;
          changeToScreen(SCREEN_TEXT_VIEWER);
        }
        else if (menuSelectedItemID == ITEM_THIRD_PARTY_NOTICES)
        {
          textViewerText = thirdPartyNoticesText;
          changeToScreen(SCREEN_TEXT_VIEWER);
        }
        else if(menuSelectedItemID == ITEM_EASTER_EGG)
        {
          changeToScreen(SCREEN_EASTER_EGG);
        }

        //exit
        if(heldButton == KEY_SELECT)
        {
          if(!Sys.rememberMenuPosition)
          {
            topItem = 1;
            highlightedItem = 1;
          }
          changeToScreen(SCREEN_SYSTEM_MENU);
        }
      }
      break;
      
    case SCREEN_EASTER_EGG:
      {
        //------ Simple flappy bird game -------
        
        display.setInterlace(false);
        
        //note that some values here have been empirically determined
        
        const int16_t BIRD_HEIGHT = 12;
        const int16_t BIRD_LENGTH = 17;
        const int16_t BIRD_YMAX = 50;
        const int16_t PIPE_WIDTH = 17;
        const int16_t PIPE_GAP = 30;
        const float   GRAVITY = 0.068; 
        const float   SCROLL_ACC = 0.0004; 
        const float   SCROLL_VELOCITY_MAX = 2.0;
        const float   BG_SCROLL_ACC = 0.0002;
        
        enum {
          PIPE_VARIANT_A,
          PIPE_VARIANT_B,
          PIPE_VARIANT_C,
          PIPE_VARIANT_COUNT
        };
        
        static const uint8_t bgTerrain[128] PROGMEM = {
          0x04, 0x0c, 0x06, 0x02, 0x03, 0x06, 0x04, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x04, 0x0c, 0x08, 
          0x08, 0x0c, 0x06, 0x04, 0x0c, 0x0c, 0x04, 0x04, 0x0c, 0x08, 0x08, 0x0e, 0x03, 0x03, 0x02, 0x02, 
          0x06, 0x04, 0x04, 0x0c, 0x08, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x04, 0x07, 0x01, 0x03, 
          0x02, 0x02, 0x02, 0x02, 0x02, 0x06, 0x04, 0x04, 0x06, 0x02, 0x06, 0x03, 0x07, 0x04, 0x04, 0x04, 
          0x0c, 0x08, 0x0c, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x06, 0x03, 0x07, 0x04, 0x04, 0x04, 
          0x0c, 0x06, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x08, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 
          0x06, 0x03, 0x01, 0x01, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x03, 0x03, 0x06, 0x06, 0x02, 
          0x02, 0x06, 0x04, 0x04, 0x0e, 0x08, 0x08, 0x08, 0x0c, 0x06, 0x06, 0x04, 0x06, 0x02, 0x06, 0x04
        };
        
        enum {
          GAME_STATE_INIT,
          GAME_STATE_PLAYING,
          GAME_STATE_OVER,
          GAME_STATE_PAUSED
        };
        
        static int16_t hiScore;
        
        //Dynamically allocated memory for the game
        
        static struct FlappyGame {
          float    birdX;
          float    birdY; 
          float    pipeX;
          float    pipeY;
          uint8_t  pipeVariant;
          float    velocity; 
          float    scrollVelocity;
          bool     maxScrollVelocityReached;
          int16_t  score;
          bool     hasScored;
          float    bgScrollVelocity;
          float    bgX;
          uint8_t  state;
        } *game = NULL;
        
        //Allocate memory if it hasn't been allocated yet
        
        if(game == NULL)
        {
          game = (struct FlappyGame *)malloc(sizeof(struct FlappyGame));
          if(game == NULL) //allocation failed
          {
            showMessage(PSTR("Failed to allocate\nmemory for game"));
            delay(2000);
            //force exit
            changeToScreen(SCREEN_ABOUT);
            break;
          }
          else
          {
            game->state = GAME_STATE_INIT;
          }
        }
        
        //Run the game
        
        switch(game->state)
        {
          case GAME_STATE_INIT:
            {
              printFullScreenMessage(PSTR("Get ready"));
              if(clickedButton == KEY_SELECT || millis() - buttonReleaseTime > 1000)
              {
                //initialise variables
                game->birdX = 24.0;
                game->birdY = 11.0;
                game->velocity = 0.0;
                game->bgX = 0.0;
                game->score = 0;
                game->hasScored = false;
                game->pipeX = 127.0;
                game->pipeY = random(5 + PIPE_GAP, 51);
                game->pipeVariant = random() % PIPE_VARIANT_COUNT;
                game->scrollVelocity = 0.65;
                game->bgScrollVelocity = 0.1625;
                game->maxScrollVelocityReached = false;
                
                //change state
                game->state = GAME_STATE_PLAYING;
                killButtonEvents();
              }
            }
            break;
          
          case GAME_STATE_PLAYING:
            {
              //Calculate the vertical position of the bird
              //Here we are using the equations of linear motion, but with discrete time
              // i.e. s = ut + 1/2at^2 and v = u + at
              //As our time is discrete, we treat it as 1 time unit, and can then simply
              //add the current y position to the previous y position. We also do same for velocity.
              //To move the bird up on key event, we simply assign a fixed negative value 
              //to the velocity (not realistic physics).
              
              if(pressedButton == KEY_SELECT || pressedButton == KEY_UP)
                game->velocity = -1.043; //about some constant * sqrt(GRAVITY). Constant determined to be about 4
              
              //s = ut + 1/2at^2, v = u + at, time is unity and discrete
              game->birdY += game->velocity + (GRAVITY / 2.0);
              game->velocity += GRAVITY;
              
              if(game->birdY > BIRD_YMAX)
                game->birdY = BIRD_YMAX;

              //Calculate the pipe x position
              if(game->maxScrollVelocityReached)
                game->pipeX -= game->scrollVelocity;
              else
              {
                game->pipeX -= (game->scrollVelocity + (SCROLL_ACC / 2.0));
                game->scrollVelocity += SCROLL_ACC;
                if(game->scrollVelocity > SCROLL_VELOCITY_MAX)
                  game->maxScrollVelocityReached = true;
              }
              
              // game->maxScrollVelocityReached = false; //##debug
              
              //Generate new pipe when out of view
              if(game->pipeX < 0.0 - PIPE_WIDTH)
              {
                game->pipeX = 127;
                game->pipeVariant = random() % PIPE_VARIANT_COUNT;
                if(game->pipeVariant == PIPE_VARIANT_C)
                  game->pipeY = random(5 + PIPE_GAP, 51);
                if(game->pipeVariant == PIPE_VARIANT_B)
                  game->pipeY = random(5 + PIPE_GAP, 62);
                if(game->pipeVariant == PIPE_VARIANT_A)
                  game->pipeY = random(PIPE_GAP, 51);
              }
              
              //Calculate background position
              //the background is parallax scrolled
              if(game->maxScrollVelocityReached)
                game->bgX += game->bgScrollVelocity;
              else
              {
                game->bgX += (game->bgScrollVelocity + (BG_SCROLL_ACC / 2.0));
                game->bgScrollVelocity += BG_SCROLL_ACC;
              }
              
              if(game->bgX > 127)
                game->bgX = 0.0;
              
              ///--- Detect collisions ---
              
              bool collided = false; 
              
              //collision of bird with floor
              if(game->birdY >= BIRD_YMAX)
                collided = true;
              
              //collision of bird with pipe
              if(game->pipeX < (game->birdX + BIRD_LENGTH) && game->birdX < (game->pipeX + PIPE_WIDTH))
              {
                if(game->pipeVariant == PIPE_VARIANT_A || game->pipeVariant == PIPE_VARIANT_C)
                {
                  if(game->pipeY < (game->birdY + BIRD_HEIGHT))
                    collided = true;
                }
                if(game->pipeVariant == PIPE_VARIANT_B || game->pipeVariant == PIPE_VARIANT_C)
                {
                  if(game->birdY < (game->pipeY - PIPE_GAP + 1))
                    collided = true;
                }
              }
              
              // collided = false; //###Debug
              
              ///--- Calculate scores ---
              
              if(game->birdX > (game->pipeX + PIPE_WIDTH) && !game->hasScored && !collided)
              {
                game->hasScored = true;
                game->score++;
                if(game->score > hiScore)
                  hiScore = game->score;
              }
              if(game->birdX < game->pipeX)
                game->hasScored = false;
              
              ///--- Draw on the screen ---
              
              //draw the background
              uint8_t idx = game->bgX;
              for(uint8_t x = 0; x < 128; x++, idx++)
              {
                if(idx > 127)
                  idx = 0;
                uint8_t val = pgm_read_byte(bgTerrain + idx);
                for(uint8_t b = 0; b < 4; b++)
                {
                  if(val & (1 << b))
                    display.drawPixel(x, 54 + b, BLACK);
                }
              }
              
              //Draw the foreground
              display.drawHLine(0, 62, 128, BLACK);
              int16_t gx = game->pipeX;
              while(gx >= 0)
              {
                display.drawPixel(gx, 63, BLACK);
                display.drawPixel(gx + 1, 63, BLACK);
                gx -= 6;
              }
              gx = game->pipeX;
              while(gx < 128)
              {
                display.drawPixel(gx, 63, BLACK);
                display.drawPixel(gx + 1, 63, BLACK);
                gx += 6;
              }
              
              //Draw the pipe
              if(game->pipeVariant == PIPE_VARIANT_A || game->pipeVariant == PIPE_VARIANT_C)
              {
                //lower pipe
                display.drawRect(game->pipeX, game->pipeY, PIPE_WIDTH, 4, BLACK);
                display.fillRect(game->pipeX + 1, game->pipeY + 3, PIPE_WIDTH - 2, 60 - game->pipeY, WHITE);
                display.drawRect(game->pipeX + 1, game->pipeY + 3, PIPE_WIDTH - 2, 60 - game->pipeY, BLACK);
              }
              if(game->pipeVariant == PIPE_VARIANT_B || game->pipeVariant == PIPE_VARIANT_C)
              {
                //upper pipe
                display.drawRect(game->pipeX, game->pipeY - PIPE_GAP - 3, PIPE_WIDTH, 4, BLACK);
                display.drawVLine(game->pipeX + 1, 0, game->pipeY - PIPE_GAP - 3, BLACK);
                display.drawVLine(game->pipeX + PIPE_WIDTH - 2, 0, game->pipeY - PIPE_GAP - 3, BLACK);
              }
              
              //Draw the bird
              display.fillRect(game->birdX, game->birdY, BIRD_LENGTH, BIRD_HEIGHT, WHITE);
              drawAnimatedSprite(game->birdX, game->birdY, animation_bird, BIRD_LENGTH, BIRD_HEIGHT, BLACK, 3, 80, 0, true);

              //Draw the score
              display.setCursor(0, 0);
              display.print(game->score);
              
              //###Debug draw
              // display.setCursor(0, 8);
              // display.print(game->scrollVelocity);
              // display.setCursor(0, 16);
              // display.print(game->bgScrollVelocity);
              
              ///--- Play audio ---
              /* 
              if(audioToPlay = AUDIO_KEY_PRESSED) //always suppress key press tones
                audioToPlay = AUDIO_NONE;
              if(game->hasScored)
                audioToPlay = AUDIO_NONE; //replace with actual
              if(collided)
                audioToPlay = AUDIO_NONE; //replace with actual 
              */

              ///--- Change game state ---
              
              //change state if collided
              if(collided)
              {
                game->state = GAME_STATE_OVER;
                killButtonEvents();
              }
            }
            break;
          
          case GAME_STATE_OVER:
            {
              drawBoundingBox(11, 14, 105, 35,BLACK);
              display.setCursor(17, 18);
              display.print(F("Game over"));
              display.setCursor(17, 28);
              display.print(F("Score:   "));
              display.print(game->score);
              display.setCursor(17, 38);
              display.print(F("HiScore: "));
              display.print(hiScore);
              
              if(clickedButton == KEY_SELECT)
              {
                game->state = GAME_STATE_INIT;
                killButtonEvents();
              }
            }
            break;
        }
        
        //Exit the game, freeing the allocated memory
        if(heldButton == KEY_SELECT)
        {
          free(game); //free the memory
          game = NULL; //set the pointer to NULL to avoid dangling pointer issues
          changeToScreen(SCREEN_ABOUT);
        }
      }
      break;
  }
}

#endif
