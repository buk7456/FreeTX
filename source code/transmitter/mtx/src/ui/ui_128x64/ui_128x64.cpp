#include "ui_128x64.h"

#if defined (UI_128X64)

#if defined (DISPLAY_KS0108)
  #include "../../lcd/GFX.h"
  #include "../../lcd/LCDKS0108.h"
  LCDKS0108 display = LCDKS0108(PIN_KS_RS, PIN_KS_EN, PIN_KS_CS1, PIN_KS_CS2);
#elif defined (DISPLAY_ST7920)
  #include "../../lcd/GFX.h"
  #include "../../lcd/LCDST7920.h"
  LCDST7920 display = LCDST7920(PIN_ST_RS, PIN_ST_EN);
#endif

//------ Menu strings --------
// Max 19 characters per string

char const mainMenu[][20] PROGMEM = { 
  "Model", "Inputs", "Mixer", "Outputs", "Extras", "Telemetry", "System", "Receiver"
};

char const extrasMenu[][20] PROGMEM = { 
  "Custom curves", "Functn generators", "Logical switches", "Timers", "Counters", "Notifications", 
  "Safety checks", "Trim setup", "Flight modes"
};

char const systemMenu[][20] PROGMEM = { 
  "RF setup", "Sound", "Backlight", "Appearance", "Miscellaneous", "Advanced", "About"
};

char const advancedMenu[][20] PROGMEM = { 
  "Sticks", "Knobs", "Switches", "Battery", "Security", "Debug"
};

//------ Misc ---------------

char textBuff[32];

uint8_t theScreen = SCREEN_HOME;
uint8_t lastScreen = SCREEN_HOME;

uint8_t focusedItem = 1; 

bool isEditTextDialog = false;
bool isDisplayingBatteryWarning = false;

union SharedData mySharedUnion;

bool graphYCoordinatesInvalid = true;

uint8_t menuSelectedItemID = 0xff;

uint8_t contextMenuTopItem = 1;
uint8_t contextMenuFocusedItem = 1;
uint8_t contextMenuSelectedItemID = 0xff;

uint8_t thisModelIdx = 0;

uint8_t thisMixIdx = 0;
uint8_t destMixIdx = 0; 

uint8_t thisChIdx = 0;

uint8_t thisCrvIdx = 0;
uint8_t destCrvIdx = 0;
uint8_t thisCrvPt = 0;

uint8_t thisFgenIdx = 0;
uint8_t destFgenIdx = 0;

uint8_t thisLsIdx = 0;
uint8_t destLsIdx = 0;

uint8_t thisTimerIdx = 0;

uint8_t thisCounterIdx = 0;
uint8_t destCounterIdx = 0;

uint8_t thisNotificationIdx = 0;
uint8_t destNotificationIdx = 0;

uint8_t thisFmdIdx = 0;

uint8_t thisTelemIdx = 0;

uint8_t thisWidgetIdx = 0;
uint8_t destWidgetIdx = 0;

bool isRequestingSettingsRestore = false;
bool isRequestingStickCalibration = false;
bool isRequestingKnobCalibration = false;
bool isRequestingSwitchesSetup = false;
bool batteryGaugeCalibrated = true;
bool mainMenuLocked = true;
const char* textViewerText;

bool isOnscreenTrimMode = false;
uint8_t trimIdx = 0;

//------ Function pointers for the screens ----------------

typedef void (*ScreenHandler)(void);

// Jump table, implemented as an array of pointers to functions, in PROGMEM.
// Here we are using designated initialisers in the array.
const ScreenHandler screen_handlers[TOTAL_SCREEN_COUNT] PROGMEM = {
  [SCREEN_HOME] = ui_handler_home,
  [SCREEN_CHANNEL_MONITOR] = ui_handler_home,
  [CONTEXT_MENU_HOME_SCREEN] = ui_handler_home,
  [SCREEN_WIDGET_SETUP] = ui_handler_home,
  [CONTEXT_MENU_WIDGETS] = ui_handler_home,
  [DIALOG_COPY_WIDGET] = ui_handler_home,
  [DIALOG_MOVE_WIDGET] = ui_handler_home,
  
  [SCREEN_UNLOCK_MAIN_MENU] = ui_handler_main_menu,
  [SCREEN_MAIN_MENU] = ui_handler_main_menu,
  
  [SCREEN_UNLOCK_MODEL_MANAGER] = ui_handler_model_manager,
  [SCREEN_MODEL] = ui_handler_model_manager,
  [CONTEXT_MENU_ACTIVE_MODEL] = ui_handler_model_manager,
  [CONTEXT_MENU_INACTIVE_MODEL] = ui_handler_model_manager,
  [CONTEXT_MENU_FREE_MODEL] = ui_handler_model_manager,
  [DIALOG_MODEL_TYPE] = ui_handler_model_manager,
  [DIALOG_RENAME_MODEL] = ui_handler_model_manager,
  [DIALOG_COPYFROM_MODEL] = ui_handler_model_manager,
  [CONFIRMATION_MODEL_BACKUP] = ui_handler_model_manager,
  [DIALOG_RESTORE_MODEL] = ui_handler_model_manager,
  [CONFIRMATION_MODEL_RESTORE] = ui_handler_model_manager,
  [CONFIRMATION_MODEL_COPY] = ui_handler_model_manager,
  [CONFIRMATION_MODEL_DELETE] = ui_handler_model_manager,
  [CONFIRMATION_MODEL_RESET] = ui_handler_model_manager,
  [CONFIRMATION_CREATE_MODEL] = ui_handler_model_manager,
  
  [SCREEN_INPUTS] = ui_handler_inputs,
  
  [SCREEN_MIXER] = ui_handler_mixer,
  [DIALOG_MIX_FLIGHT_MODE] = ui_handler_mixer,
  [CONTEXT_MENU_MIXER] = ui_handler_mixer,
  [SCREEN_MIXER_OUTPUT] = ui_handler_mixer,
  [DIALOG_COPY_MIX] = ui_handler_mixer,
  [DIALOG_MOVE_MIX] = ui_handler_mixer,
  [DIALOG_RENAME_MIX] = ui_handler_mixer,
  [CONFIRMATION_MIXES_RESET] = ui_handler_mixer,
  [CONTEXT_MENU_MIXER_TEMPLATES] = ui_handler_mixer,
  [CONFIRMATION_LOAD_MIXER_TEMPLATE] = ui_handler_mixer,
  [SCREEN_MIXER_OVERVIEW] = ui_handler_mixer,
  
  [SCREEN_OUTPUTS] = ui_handler_outputs,
  [CONTEXT_MENU_OUTPUTS] = ui_handler_outputs,
  [DIALOG_RENAME_CHANNEL] = ui_handler_outputs,
  
  [SCREEN_EXTRAS_MENU] = ui_handler_extras_menu,
  
  [SCREEN_CUSTOM_CURVES] = ui_handler_custom_curves,
  [CONTEXT_MENU_CUSTOM_CURVE] = ui_handler_custom_curves,
  [DIALOG_RENAME_CUSTOM_CURVE] = ui_handler_custom_curves,
  [DIALOG_COPY_CUSTOM_CURVE] = ui_handler_custom_curves,
  [DIALOG_INSERT_CURVE_POINT] = ui_handler_custom_curves,
  [DIALOG_DELETE_CURVE_POINT] = ui_handler_custom_curves,
  
  [SCREEN_LOGICAL_SWITCHES] = ui_handler_logical_switches,
  [CONTEXT_MENU_LOGICAL_SWITCHES] = ui_handler_logical_switches,
  [DIALOG_COPY_LOGICAL_SWITCH] = ui_handler_logical_switches,
  [DIALOG_MOVE_LOGICAL_SWITCH] = ui_handler_logical_switches,
  [SCREEN_LOGICAL_SWITCH_OUTPUTS] = ui_handler_logical_switches,
  
  [SCREEN_COUNTERS] = ui_handler_counters,
  [CONTEXT_MENU_COUNTERS] = ui_handler_counters,
  [DIALOG_RENAME_COUNTER] = ui_handler_counters,
  [DIALOG_COUNTER_TYPE] = ui_handler_counters,
  [DIALOG_COPY_COUNTER] = ui_handler_counters,
  [CONFIRMATION_CLEAR_ALL_COUNTERS] = ui_handler_counters,
  [SCREEN_COUNTER_OUTPUTS] = ui_handler_counters,
  
  [SCREEN_FLIGHT_MODES] = ui_handler_flight_modes,
  
  [SCREEN_FUNCTION_GENERATORS] = ui_handler_function_generators,
  [CONTEXT_MENU_FUNCGEN] = ui_handler_function_generators,
  [DIALOG_COPY_FUNCGEN] = ui_handler_function_generators,
  [SCREEN_FUNCGEN_OUTPUTS] = ui_handler_function_generators,
  
  [SCREEN_TRIM_SETUP] = ui_handler_trim_setup,
  
  [SCREEN_SAFETY_CHECKS] = ui_handler_safety_checks,
  
  [SCREEN_TIMERS] = ui_handler_timers,
  [DIALOG_TIMER_INITIAL_TIME] = ui_handler_timers,
  [CONTEXT_MENU_TIMERS] = ui_handler_timers,
  
  [SCREEN_NOTIFICATION_SETUP] = ui_handler_custom_notifications,
  [CONTEXT_MENU_NOTIFICATIONS] = ui_handler_custom_notifications,
  [DIALOG_COPY_NOTIFICATION] = ui_handler_custom_notifications,

  [SCREEN_TELEMETRY] = ui_handler_telemetry,
  [CONTEXT_MENU_ACTIVE_SENSOR] = ui_handler_telemetry,
  [CONTEXT_MENU_FREE_SENSOR] = ui_handler_telemetry,
  [SCREEN_CREATE_SENSOR] = ui_handler_telemetry,
  [CONTEXT_MENU_SENSOR_TEMPLATES] = ui_handler_telemetry,
  [SCREEN_SENSOR_STATISTICS] = ui_handler_telemetry,
  [SCREEN_EDIT_SENSOR] = ui_handler_telemetry,
  [CONFIRMATION_DELETE_SENSOR] = ui_handler_telemetry,
  [SCREEN_TELEMETRY_GNSS] = ui_handler_telemetry,
  [SCREEN_SELECT_GNSS_UNITS] = ui_handler_telemetry,
  
  [SCREEN_SYSTEM_MENU] = ui_handler_system_settings,
  [SCREEN_RF] = ui_handler_system_settings,
  [SCREEN_SOUND] = ui_handler_system_settings,
  [SCREEN_BACKLIGHT] = ui_handler_system_settings,
  [SCREEN_APPEARANCE] = ui_handler_system_settings,
  [SCREEN_MISCELLANEOUS] = ui_handler_system_settings,
  [SCREEN_UNLOCK_ADVANCED_MENU] = ui_handler_system_settings,
  [SCREEN_ADVANCED_MENU] = ui_handler_system_settings,
  [SCREEN_STICKS] = ui_handler_system_settings,
  [SCREEN_KNOBS] = ui_handler_system_settings,
  [SCREEN_SWITCHES] = ui_handler_system_settings,
  [SCREEN_BATTERY] = ui_handler_system_settings,
  [SCREEN_SECURITY] = ui_handler_system_settings,
  [SCREEN_DEBUG] = ui_handler_system_settings,
  [SCREEN_DEBUG_STATISTICS] = ui_handler_system_settings,
  [SCREEN_INTERNAL_EEPROM_DUMP] = ui_handler_system_settings,
  [SCREEN_EXTERNAL_EEPROM_DUMP] = ui_handler_system_settings,
  [SCREEN_CHARACTER_SET] = ui_handler_system_settings,
  [SCREEN_SCREENSHOT_CONFIG] = ui_handler_system_settings,
  [DIALOG_ADJUST_LONG_PRESS_DELAY] = ui_handler_system_settings,
  [DIALOG_ADJUST_KEY_REPEAT_INTERVAL] = ui_handler_system_settings,
  [CONFIRMATION_BACKUP_SYSTEM_SETTINGS] = ui_handler_system_settings,
  [CONFIRMATION_RESTORE_SYSTEM_SETTINGS] = ui_handler_system_settings,
  [CONFIRMATION_FACTORY_RESET] = ui_handler_system_settings,
  [SCREEN_ABOUT] = ui_handler_system_settings,
  [SCREEN_EASTER_EGG] = ui_handler_system_settings,
  
  [SCREEN_RECEIVER] = ui_handler_receiver,
  [SCREEN_RECEIVER_CONFIG] = ui_handler_receiver,
  [SCREEN_RECEIVER_BINDING] = ui_handler_receiver,
  
  [SCREEN_TEXT_VIEWER] = ui_handler_text_viewer,
};

//==================================================================================================

void initialiseDisplay()
{
  display.begin();
  display.setTextWrap(false);
  display.clearDisplay();
  display.display();
}

//==================================================================================================

void startInitialSetup()
{
  isRequestingSettingsRestore = true;
  isRequestingStickCalibration = true;
  isRequestingKnobCalibration = true;
  isRequestingSwitchesSetup = true;
  batteryGaugeCalibrated = false;
}

//============================ Generic messages ====================================================

void showMessage(const char* str)
{
  display.clearDisplay();
  printFullScreenMessage(str);
  display.setInterlace(false);
  display.display();
}

void showWaitMessage()
{
  display.clearDisplay();
  printFullScreenMessage(PSTR("Please wait"));
  display.setInterlace(false);
  display.display();
}

void showMuteMessage()
{
  display.clearDisplay();
  display.drawBitmap(56, 24, icon_mute_large, 16, 16, BLACK);
  display.setInterlace(false);
  display.display();
}

void showProgressMessage(const char* str, uint8_t percent)
{
  display.clearDisplay();
  printFullScreenMessage(str);
  drawHorizontalBarChart(38, display.getCursorY() + 9, 52, 7, BLACK, percent, 0, 100);
  display.setInterlace(false);
  display.display();
}

//============================ Battery warning =====================================================

void handleBatteryWarningUI()
{
  if(!batteryGaugeCalibrated)
    return;
  
  static uint32_t batteryWarningMillis = millis();
  static bool batteryWarningDismissed = false;
  
  if(batteryState == BATTERY_LOW)
  {
    if(!batteryWarningDismissed)
    {
      //show warning
      display.clearDisplay();
      drawAnimatedSprite(50, 14, animation_low_battery, 27, 11, BLACK, 7, 300, batteryWarningMillis, false);
      printFullScreenMessage(PSTR("\nBattery low"));
      display.display();
      
      isDisplayingBatteryWarning = true;
      
      //play warning tone
      audioToPlay = AUDIO_BATTERY_WARNING;
      playTones();
      
      //self-dismiss warning or on a button press
      if((pressedButton > 0 || millis() - batteryWarningMillis > 2000))
      {
        batteryWarningDismissed = true;
        batteryWarningMillis = millis();
        isDisplayingBatteryWarning = false;
        //only kill button events if a button was pressed
        if(pressedButton > 0)
          killButtonEvents();
      }
    }
    //remind low battery every 10 minutes
    if(batteryWarningDismissed && (millis() - batteryWarningMillis > 600000UL)) 
    {
      batteryWarningDismissed = false;
      batteryWarningMillis = millis();
    }
  }
  else
  {
    isDisplayingBatteryWarning = false;
    batteryWarningMillis = millis();
  }
}

//============================ Safety warning ======================================================

void handleSafetyWarningUI()  //blocking function
{
  if(isRequestingSwitchesSetup || isRequestingStickCalibration || isRequestingKnobCalibration)
    return;
  
  while(1)
  {
    delay(10);
    
    readSwitchesAndButtons();
    determineButtonEvent();
    readSticks();
    
    handlePowerOff();
    
    checkBattery();
    handleBatteryWarningUI();
    if(isDisplayingBatteryWarning)
      continue;
    
    inactivityAlarmHandler();
    
    display.clearDisplay();
    
    static bool audioTriggered = false;
    
    //Check throttle position
    bool isThrottleWarn = false;
    if(Model.checkThrottle && (Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER))
    {
      if(Model.thrSrcRaw >= SRC_STICK_AXIS_FIRST && Model.thrSrcRaw <= SRC_STICK_AXIS_LAST)
      {
        uint8_t idx = Model.thrSrcRaw - SRC_STICK_AXIS_FIRST;
        if(Sys.StickAxis[idx].type == STICK_AXIS_SELF_CENTERING && (stickAxisIn[idx] > 0 || stickAxisIn[idx] < 0))
          isThrottleWarn = true;
        else if(Sys.StickAxis[idx].type == STICK_AXIS_NON_CENTERING && stickAxisIn[idx] > -450)
          isThrottleWarn = true;
      }
      else if(Model.thrSrcRaw >= SRC_KNOB_FIRST && Model.thrSrcRaw <= SRC_KNOB_LAST)
      {
        uint8_t idx = Model.thrSrcRaw - SRC_KNOB_FIRST;
        if(knobIn[idx] > -450)
          isThrottleWarn = true;
      }
    }

    //Check switch positions
    uint8_t numSwitchWarnings = 0;
    for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
    {
      if(Model.switchWarn[i] == -1 || Sys.swType[i] == SW_ABSENT) //don't check
        continue;
      int8_t desiredState = Model.switchWarn[i];
      if(swState[i] != desiredState)
      {
        numSwitchWarnings++;
        uint8_t xpos = 6 + ((numSwitchWarnings - 1) % 5) * 24;
        uint8_t ypos = 38 + ((numSwitchWarnings - 1) / 5) * 8;
        if(!isThrottleWarn)
          ypos -= 17;
        display.setCursor(xpos, ypos);
        getSrcName(textBuff, SRC_SW_PHYSICAL_FIRST + i, sizeof(textBuff));
        display.print(textBuff);
      }
    }
    
    //Draw on screen
    if(numSwitchWarnings == 0 && !isThrottleWarn)
    {
      display.clearDisplay();
      audioTriggered = false;
      return;
    }
    else
    {
      display.fillRect(0, 0, 128, 11, BLACK);
      display.drawBitmap(27, 0, icon_warning, 11, 10, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(43, 2);
      display.print(F("WARNING"));
      display.setTextColor(BLACK);

      if(isThrottleWarn)
      {
        display.setCursor(0, 13);
        display.print(F("\x07"));
        display.print(F("Check throttle:"));
        display.setCursor(6, 21);
        getSrcName(textBuff, Model.thrSrcRaw, sizeof(textBuff));
        display.print(textBuff);
      }
      
      if(numSwitchWarnings)
      {
        if(isThrottleWarn)
          display.setCursor(0, 30);
        else
          display.setCursor(0, 13);
        display.print(F("\x07"));
        display.print(F("Check switches:"));
      }
      
      display.setCursor(28, 56);
      display.print(F("[OK] to skip"));
      
      display.display();
      
      if(!audioTriggered)
      {
        audioTriggered = true;
        audioToPlay = AUDIO_SAFETY_WARNING;
      }
    }
    
    //skip warning when the Select key is clicked/held
    if(clickedButton == KEY_SELECT || heldButton == KEY_SELECT)
    {
      killButtonEvents();
      display.clearDisplay();
      audioTriggered = false;
      return; 
    }
    
    //screenshot
    screenshotHandler();
    
    //call tone player function
    playTones();
  }
}

//============================ Data import warning =================================================

void handleDataImportWarningUI() // blocking function
{
  display.clearDisplay();
              
  //print the items center aligned
  
  strlcpy_P(textBuff, PSTR("Unrecognised data"), sizeof(textBuff));
  display.setCursor((display.width() - strlen(textBuff) * 6) / 2, 9);
  display.print(textBuff);

  strlcpy_P(textBuff, PSTR("was skipped."), sizeof(textBuff));
  display.setCursor((display.width() - strlen(textBuff) * 6) / 2, 18);
  display.print(textBuff);

  char temp[6];
  utoa(dbgTotalErrorLines, temp, 10);
  strlcpy_P(textBuff, PSTR("("), sizeof(textBuff));
  strlcat(textBuff, temp, sizeof(textBuff));
  strlcat_P(textBuff, PSTR(" lines skipped,"), sizeof(textBuff));
  display.setCursor((display.width() - strlen(textBuff) * 6) / 2, 36);
  display.print(textBuff);
  
  strlcpy_P(textBuff, PSTR("first at line "), sizeof(textBuff));
  utoa(dbgFirstErrorLineNumber, temp, 10);
  strlcat(textBuff, temp, sizeof(textBuff));
  strlcat_P(textBuff, PSTR(")"), sizeof(textBuff));
  display.setCursor((display.width() - strlen(textBuff) * 6) / 2, 45);
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

//============================ Main user interface =================================================

void handleMainUI()
{
  //-------------- Keys filtering for backlight ----------
  if(Sys.backlightSuppressFirstKey && Sys.backlightEnabled)
  {
    static bool suppressClick = false;
    if(!backlightIsOn && (pressedButton == KEY_UP || pressedButton == KEY_DOWN || pressedButton == KEY_SELECT))
    {
      if(theScreen == SCREEN_HOME)
      {
        if(isOnscreenTrimMode)
        {
          //Nothing here, all button events are let through when we are in On-screen trim mode. 
        }
        else
        {
          //Only let through the held event so we can perform long press actions without requiring an extra button press.
          pressedButton = 0;
          suppressClick = true;
        }
      }
      else //other screens
        killButtonEvents();
    }

    if(suppressClick && backlightIsOn)
    {
      if(clickedButton == KEY_UP || clickedButton == KEY_DOWN || clickedButton == KEY_SELECT)
      {
        suppressClick = false;
        clickedButton = 0;
      }
      if(buttonCode == 0)
        suppressClick = false;
    }
  }

  //-------------- Timers -------------------------------
  timerHandler();

  //-------------- Inactivity----------------------------
  inactivityAlarmHandler();
  if(Sys.lockOnInactivity && !isEmptyStr(Sys.password, sizeof(Sys.password)) && theScreen == SCREEN_HOME)
  {
    if(Sys.inactivityMinutes > 0)
    {
      if((millis() - inputsLastMovedTime) > ((uint32_t)Sys.inactivityMinutes * 60000))
        mainMenuLocked = true;
    }
  }
  
  //-------------- Telemetry alarms ---------------------
  telemetryAlarmHandler();
  
  //-------------- Battery warning ----------------------
  handleBatteryWarningUI();
  if(isDisplayingBatteryWarning)
    return;
  
  //-------------- Enable interlace mode by default -----
  //This can be overridden where necessary specially in some screens
  //to prevent interlace artifacts
  display.setInterlace(true); 

  //-------------- Main UI state machine ----------------

  if(theScreen < TOTAL_SCREEN_COUNT)
  {
    ScreenHandler handler = (ScreenHandler)pgm_read_ptr(&screen_handlers[theScreen]);
    if(handler != NULL)
      handler();
    else
      changeToScreen(SCREEN_HOME);
  }
  else
    changeToScreen(SCREEN_HOME);

  //-------------- Toasts -------------------------------
  drawToast();
  
  //-------------- Custom notifications -----------------
  notificationHandler();
  
  //-------------- Debug print --------------------------
  if(Sys.showLoopTime)
  {
    // display.fillRect(102, 55, 26, 9, WHITE);
    display.setCursor(103, 56);
    uint16_t tt = DBG_loopTime / 100;
    display.print(tt / 10);
    display.print(F(".")); 
    display.print(tt % 10); 
  }

  //-------------- Screenshot --------------------------
  screenshotHandler();
  
  //-------------- Show on physical LCD -----------------
  if(Sys.disableInterlacing) //override
    display.setInterlace(false);
  display.display(); 
  display.clearDisplay();

  //-------------- Sound --------------------------------
  playTones();
}

//============================ Helpers =============================================================

void resetPages()
{
  thisMixIdx = 0;
  thisChIdx = 0;
  thisCrvIdx = 0;
  thisCrvPt = 0;
  thisFgenIdx = 0;
  thisLsIdx = 0;
  thisTimerIdx = 0;
  thisCounterIdx = 0;
  thisNotificationIdx = 0;
  thisFmdIdx = 0;
  thisTelemIdx = 0;
  thisWidgetIdx = 0;
  trimIdx = 0;
}

//---------------------------------------------------------------------------------------------------

void toggleEditModeOnSelectClicked()
{
  if(clickedButton == KEY_SELECT)
    isEditMode = !isEditMode;
}

//--------------------------------------------------------------------------------------------------

void changeFocusOnUpDown(uint8_t numItems)
{
  if(isEditMode)
    return;
  isEditMode = true;
  focusedItem = incDec(focusedItem, numItems, 1, INCDEC_WRAP, INCDEC_SLOW);
  isEditMode = false;
}

//--------------------------------------------------------------------------------------------------

void changeToScreen(uint8_t scrn)
{
  lastScreen = theScreen;
  theScreen = scrn;
  focusedItem = 1;
  isEditMode = false;
  contextMenuTopItem = 1;
  contextMenuFocusedItem = 1;
  graphYCoordinatesInvalid = true;
  killButtonEvents();
}

//--------------------------------------------------------------------------------------------------

void printModelName(char* buff, uint8_t modelIdx)
{
  if(isEmptyStr(buff, sizeof(Model.name)))
  {
    display.print(F("Model"));
    display.print(modelIdx + 1); //add 1 as Model0 for example doesn't make sense to the user
  }
  else
    display.print(buff);
}

//--------------------------------------------------------------------------------------------------

void printFullScreenMessage(const char* str)
{
  if(pgm_read_byte(str) == '\0')
    return;

  uint8_t pos = 0; //position in string
  uint8_t numTextLines = 1;
  //get number of lines
  while(pgm_read_byte(str + pos) != '\0')
  {
    if(pgm_read_byte(str + pos) == '\n')
      numTextLines++;
    pos++;
  }
  pos = 0; //reset
  uint8_t ypos = (display.height() - numTextLines * 9) / 2 + 1; //9 is line height
  for(uint8_t line = 0; line < numTextLines; line++)
  {
    //get number of characters in the line
    uint8_t numChars = 0;
    while(pgm_read_byte(str + pos) != '\n' && pgm_read_byte(str + pos) != '\0')
    {
      numChars++;
      pos++;
    }
    pos -= numChars;
    //center text
    int16_t xpos = (display.width() - numChars * 6) / 2;
    display.setCursor(xpos, ypos);
    //write the characters 
    while(numChars--)
      display.write(pgm_read_byte(str + pos++));
    //advance 
    ypos += 9; 
    pos++;
  }
}

//--------------------------------------------------------------------------------------------------

void printHHMMSS(int32_t millisecs)
{
  if(millisecs < 0)
  {
    display.print(F("-"));
    millisecs = -millisecs;
  }
  uint32_t hh, mm, ss;
  ss = millisecs / 1000;
  hh = ss / 3600;
  ss = ss - hh * 3600;
  mm = ss / 60;
  ss = ss - mm * 60;
  if(hh > 0 || Sys.alwaysShowHours)
  {
    if(Sys.alwaysShowHours && hh < 10)
      display.print(F("0"));
    display.print(hh);
    display.print(F(":"));
  }
  if(mm < 10)
    display.print(F("0"));
  display.print(mm);
  display.print(F(":"));
  if(ss < 10)
    display.print(F("0"));
  display.print(ss);
}

//--------------------------------------------------------------------------------------------------

void printTimerValue(uint8_t idx)
{
  if(Model.Timer[idx].initialSeconds == 0) //a count up timer
    printHHMMSS(timerElapsedTime[idx]);
  else //a count down timer
  {
    uint32_t initMillis = (uint32_t) Model.Timer[idx].initialSeconds * 1000;
    if(timerElapsedTime[idx] < initMillis)
    {
      uint32_t ttqq = initMillis - timerElapsedTime[idx];
      printHHMMSS(ttqq + 999); //add 999 ms so the displayed time doesn't 
      //change immediately upon running the timer
    }
    else
    {
      uint32_t ttqq = timerElapsedTime[idx] - initMillis;
      if(ttqq >= 1000) //prevents displaying -00:00
        display.print(F("-"));
      printHHMMSS(ttqq);
    }
  }
}

//--------------------------------------------------------------------------------------------------

void printVoltage(int16_t millivolts)
{
  int16_t val = millivolts / 10;
  if(val < 0)
  {
    display.print(F("-"));
    val = -val;
  }
  display.print(val / 100);
  display.print(F("."));
  val = val % 100;
  if(val < 10) 
    display.print(F("0"));
  display.print(val);
  display.setCursor(display.getCursorX() + 3, display.getCursorY());
  display.print(F("V"));
}

//--------------------------------------------------------------------------------------------------

void printSeconds(int16_t decisecs)
{
  if(decisecs < 0)
  {
    display.print(F("-"));
    decisecs = -decisecs;
  }
  display.print(decisecs / 10);
  display.print(F("."));
  display.print(decisecs % 10);
  display.setCursor(display.getCursorX() + 3, display.getCursorY());
  display.print(F("s"));
}

//--------------------------------------------------------------------------------------------------

void printFixedPointVal(int32_t val, uint8_t decimals) 
{
  if(decimals > 9) 
    decimals = 9; //limit to avoid overflow
  
  //handle case of 0 decimals
  if(decimals == 0) 
  {
    display.print(val);
    return;
  }

  //precomputed powers of 10 for faster scaling factor calculation
  static const int32_t powersOf10[] PROGMEM = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
  };
  int32_t factor = pgm_read_dword(&powersOf10[decimals]);

  //handle negative values
  bool isNegative = (val < 0);
  if(isNegative) 
  {
    display.print(F("-"));
    val = -val;
  }

  //split the value into integer and fractional parts
  int32_t integerPart = val / factor;
  int32_t fractionalPart = val % factor;
  
  //print the integer part
  display.print(integerPart);
  
  display.print(F("."));
  
  //convert fractional part to string with leading zeros
  char buffer[12]; //large enough
  int8_t index = decimals - 1;
  for(uint8_t i = 0; i < decimals; i++)
  {
    buffer[index--] = '0' + (fractionalPart % 10);
    fractionalPart /= 10;
  }
  buffer[decimals] = '\0';
  display.print(buffer);
}

//--------------------------------------------------------------------------------------------------

void drawDottedVLine(uint8_t x, uint8_t y, uint8_t len, uint8_t fgColor, uint8_t bgColor)
{
  display.drawVLine(x, y, len, bgColor);
  for(uint8_t i = 0; i < len; i += 2)
    display.drawPixel(x, y+i, fgColor);
}

//--------------------------------------------------------------------------------------------------

void drawDottedHLine(uint8_t x, uint8_t y, uint8_t len, uint8_t fgColor, uint8_t bgColor)
{
  display.drawHLine(x, y, len, bgColor);
  for(uint8_t i = 0; i < len; i += 2)
    display.drawPixel(x+i, y, fgColor);
}

//--------------------------------------------------------------------------------------------------

void drawCursor(uint8_t xpos, uint8_t ypos)
{
  if(isEditMode) //draw blinking cursor
  {
    if((millis() - buttonReleaseTime) % 1000 < 500 || buttonCode == KEY_UP || buttonCode == KEY_DOWN)
      display.fillRect(xpos + 3, ypos - 1, 2, 9, BLACK);
  }
  else 
    display.drawBitmap(xpos, ypos, icon_pointer, 6, 7, BLACK); //draw arrow
}

//--------------------------------------------------------------------------------------------------

void drawHeader(const char* str)
{
  strlcpy_P(textBuff, str, sizeof(textBuff));
  uint8_t textWidthPix = strlen(textBuff) * 6;
  uint8_t headingXOffset = (display.width() - textWidthPix) / 2; //middle align
  display.setCursor(headingXOffset, 0);
  display.print(textBuff);
  display.drawHLine(0, 3, headingXOffset - 2, BLACK);
  display.drawHLine(headingXOffset + textWidthPix + 1, 3, 128 - (headingXOffset + textWidthPix + 1), BLACK);
}

//--------------------------------------------------------------------------------------------------

void drawSubheader(const char* str, uint8_t ypos)
{
  strlcpy_P(textBuff, str, sizeof(textBuff));
  uint8_t textWidthPix = strlen(textBuff) * 6;
  uint8_t headingXOffset = (display.width() - textWidthPix) / 2; //middle align
  display.setCursor(headingXOffset, ypos);
  display.print(textBuff);
  drawDottedHLine(0, ypos + 3, headingXOffset - 2, BLACK, WHITE);
  drawDottedHLine(headingXOffset + textWidthPix + 1, ypos + 3, 126 - (headingXOffset + textWidthPix + 1), BLACK, WHITE);
}

//--------------------------------------------------------------------------------------------------

void drawHeader_Menu(const char* str)
{
  if(Sys.useDenserMenus)
    drawHeader(str);
  else
  {
    strlcpy_P(textBuff, str, sizeof(textBuff));
    uint8_t textWidthPix = strlen(textBuff) * 6;
    uint8_t headingXOffset = (display.width() - textWidthPix) / 2; //middle align heading
    display.setCursor(headingXOffset, 0);
    display.print(textBuff);
    display.drawHLine(0, 9, 128, BLACK);
  }
}

//--------------------------------------------------------------------------------------------------

void drawCheckbox(uint8_t xpos, uint8_t ypos, bool val)
{
  display.drawBitmap(xpos, ypos, val ? icon_checkbox_checked : icon_checkbox_unchecked, 7, 7, BLACK);
}

//--------------------------------------------------------------------------------------------------

void drawLoaderSpinner(uint8_t xpos, uint8_t ypos, uint8_t size)
{
  //active cells on a 4x4 grid. Here, every 3 bytes is a frame
  static const uint8_t activeCells[] PROGMEM = {
    1, 2, 7, 2, 7, 11, 7, 11, 14, 11, 14, 13, 14, 13, 8, 13, 8, 4, 8, 4, 1, 4, 1, 2
  };
  const uint16_t frameTime = 60;
  uint8_t frameIdx = (millis() % (8 * frameTime)) / frameTime; //total of 8 frames
  for(uint8_t i = 0; i < 3; i++)
  {
    uint8_t cell = pgm_read_byte(&activeCells[(frameIdx * 3) + i]);
    uint8_t x = xpos + size * (cell % 4);
    uint8_t y = ypos + size * (cell / 4);
    display.fillRect(x, y, size, size, BLACK);
  }
}

//--------------------------------------------------------------------------------------------------

void drawAnimatedSprite(uint8_t x, uint8_t y, const uint8_t* const bitmapTable[], uint8_t w, uint8_t h, uint8_t color, 
                        uint8_t frameCount, uint16_t frameTime, uint32_t timeOffset, bool forceAnimation)
{
  uint8_t frameIdx = 0;
  if(Sys.animationsEnabled || forceAnimation)
    frameIdx = ((millis() - timeOffset) % (frameCount * frameTime)) / frameTime; 
  display.drawBitmap(x, y, (uint8_t *)pgm_read_word(&bitmapTable[frameIdx]), w, h, color);
}

//--------------------------------------------------------------------------------------------------

void drawScrollBar(uint8_t xpos, uint8_t ypos, uint16_t numItems, uint16_t topItem, uint16_t numVisible, uint16_t viewportHeight)
{ 
  if(numItems > numVisible)
  {
    uint8_t barHeight = divRoundClosest((int32_t)viewportHeight * numVisible, numItems);
    //limit barHeight to 6 px minimum
    uint16_t _viewportHeight = viewportHeight;
    if(barHeight < 6)
    {      
      _viewportHeight -= (6 - barHeight);
      barHeight = 6;
    }
    uint8_t barYpos = ypos + divRoundClosest((int32_t)_viewportHeight * (topItem - 1), numItems);
    //Restrict the scroll bar to being drawn inside the view port. 
    //Necessary due to rounding errors that may have happened.
    if((barYpos + barHeight) > (ypos + viewportHeight))
      barYpos -= 1;
    //draw
    display.drawVLine(xpos, barYpos, barHeight, BLACK);
  }
}

//--------------------------------------------------------------------------------------------------

void drawTrimSliders()
{
  int8_t x[4] = {15, 2, 68, 125};
  int8_t y[4] = {61, 13, 61, 13};

  int16_t val[4];
  val[0] = Model.X1Trim.trimState == TRIM_FLIGHT_MODE ? Model.FlightMode[activeFmdIdx].x1Trim : Model.X1Trim.commonTrim;
  val[1] = Model.Y1Trim.trimState == TRIM_FLIGHT_MODE ? Model.FlightMode[activeFmdIdx].y1Trim : Model.Y1Trim.commonTrim;
  val[2] = Model.X2Trim.trimState == TRIM_FLIGHT_MODE ? Model.FlightMode[activeFmdIdx].x2Trim : Model.X2Trim.commonTrim;
  val[3] = Model.Y2Trim.trimState == TRIM_FLIGHT_MODE ? Model.FlightMode[activeFmdIdx].y2Trim : Model.Y2Trim.commonTrim;
  
  bool isDisabled[4];
  isDisabled[0] = Model.X1Trim.trimState == TRIM_DISABLED;
  isDisabled[1] = Model.Y1Trim.trimState == TRIM_DISABLED;
  isDisabled[2] = Model.X2Trim.trimState == TRIM_DISABLED;
  isDisabled[3] = Model.Y2Trim.trimState == TRIM_DISABLED;
  
  const int16_t range = TRIM_MAX_VAL - TRIM_MIN_VAL;
  const int8_t fixedSize = 40;
  
  for(uint8_t i = 0; i < 4; i++)
  {
    bool invertColor = false;
    if(isOnscreenTrimMode)
    {
      if(i == trimIdx) 
        invertColor = true;
      if(i == 0 || i == 2) 
      { 
        y[i] -= 1;
        if(i == trimIdx)
          display.fillRect(x[i] - 1, y[i] - 3, fixedSize + 7, 7, BLACK);
      }
      if(i == 1) 
      {
        x[i] += 1;
        if(i == trimIdx)
          display.fillRect(x[i] - 3, y[i] - 1, 7, fixedSize + 7, BLACK);
      }
      if(i == 3) 
      {
        x[i] -= 1;
        if(i == trimIdx)
          display.fillRect(x[i] - 3, y[i] - 1, 7, fixedSize + 7, BLACK);
      }    
    }
    uint8_t fgColor = invertColor ? WHITE : BLACK;
    uint8_t bgColor = invertColor ? BLACK : WHITE;
    if(isDisabled[i])
      continue;
    int16_t a = (val[i] > 0) ? 9 : -9; //used for rounding up
    if(i == 0 || i == 2) //horizontal
    {
      display.drawHLine(x[i], y[i], fixedSize + 5, fgColor);
      display.drawPixel(x[i] + 2 + fixedSize/2, y[i], bgColor);
      uint8_t xqq = x[i] + 1 + fixedSize/2 + ((val[i] + a) * fixedSize / range);
      if(val[i] > 0) 
        xqq += 1;
      else if(val[i] < 0)
        xqq -= 1;
      display.drawRect(xqq, y[i] - 2, 3, 5, fgColor);
      display.drawVLine(xqq + 1, y[i] - 1, 3, bgColor);
    }
    if(i == 1 || i == 3) //vertical
    {
      display.drawVLine(x[i], y[i], fixedSize + 5, fgColor);
      display.drawPixel(x[i], y[i] + 2 + fixedSize/2, bgColor);
      uint8_t yqq = y[i] + 1 + fixedSize/2 - ((val[i] + a) * fixedSize / range);
      if(val[i] > 0) 
        yqq -= 1;
      else if(val[i] < 0)
        yqq += 1;
      display.drawRect(x[i] - 2, yqq, 5, 3, fgColor);
      display.drawHLine(x[i] - 1, yqq + 1, 3, bgColor);
    }
  }
}

//--------------------------------------------------------------------------------------------------

void drawToast()
{
  if(toastExpired)
    return;
  
  //Table for easing the slide up/down animation.
  //t ranges from 0 to 1. Here it is actually the ratio (currTime - startTime)/transitionDuration
  //Values have been multiplied by distance to move and rounded to zero decimal places.
  static const uint8_t transitionLUT[] PROGMEM = {
    // 0,2,4,5,6,7,8,9,10,10//quadratic 1-pow(1-t,2)
    // 0,1,3,4,5,6,7,7,8,8,9,9,9,10,10,10,10,10,10,10,10 //cubic 1-pow(1-t,3) 
    0,2,4,6,7,8,9,9,10,10,10,10,10,10,10,10 //quartic 1-pow(1-t,4)
  };
  
  uint8_t numElem = sizeof(transitionLUT)/sizeof(transitionLUT[0]);
  const int16_t transitionDuration = 300; //in milliseconds
  uint32_t currTime = millis();
  uint32_t startTime = toastStartTime;
  uint32_t endTime = toastEndTime + transitionDuration * 2;
  
  if(currTime >= startTime && currTime < endTime)
  {
    uint8_t ypos = 53;
    if(Sys.animationsEnabled)
    {
      uint8_t idxLUT = numElem - 1;
      if(currTime < startTime + transitionDuration)
        idxLUT = ((currTime - startTime) * (numElem - 1)) / transitionDuration;
      else if((currTime + transitionDuration) > endTime) //slide down
        idxLUT = ((endTime - currTime) * (numElem - 1)) / transitionDuration;
      ypos = 63 - pgm_read_byte(&transitionLUT[idxLUT]);
    }
    uint8_t textWidthPix = 6 * strlen_P((const char*)toastText); 
    uint8_t xpos = (display.width() - textWidthPix) / 2; //middle align text
    if(xpos < 5)
      xpos = 5;
    display.drawRoundRect(xpos - 5, ypos, textWidthPix + 9, 11, Sys.useRoundRect ? 4 : 0, WHITE);
    display.fillRoundRect(xpos - 4, ypos + 1, textWidthPix + 7, 9, Sys.useRoundRect ? 3 : 0, BLACK);
    display.setTextColor(WHITE);
    display.setCursor(xpos, ypos + 2);
    strlcpy_P(textBuff, toastText, sizeof(textBuff));
    display.print(textBuff);
    display.setTextColor(BLACK);
  }
  else if(currTime > endTime)
  {
    toastExpired = true;
  }
}

//--------------------------------------------------------------------------------------------------

void drawBoundingBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
  if(Sys.useRoundRect)
  {
    display.drawHLine(x + 4, y, w - 8, color);
    display.drawHLine(x + 4 , y + h - 1, w - 8, color);
    display.drawVLine(x, y + 4, h - 8, color);
    display.drawVLine(x + w - 1, y + 4, h - 8, color);
    //draw the four 'round' corners
    uint8_t xx, yy;
    for(uint8_t i = 0; i < 4; i++)
    {
      xx = (i % 2) ? (x + w - 4) : (x + 2);
      yy = (i < 2) ? (y + 1) : (y + h - 2); 
      display.drawHLine(xx, yy, 2, color);
      display.drawVLine((i % 2) ? (xx + 2) : (xx - 1), (i < 2) ? (yy + 1) : (yy - 2), 2, color);
    }
  }
  else
  {
    display.drawRect(x, y, w, h, color);
  }
}

//--------------------------------------------------------------------------------------------------

void drawDialogCopyMove(const char* str, uint8_t srcIdx, uint8_t destIdx, bool isCopy)
{
  drawBoundingBox(11, 14, 105, 35,BLACK);
  display.setCursor(17, 18);
  if(isCopy) display.print(F("Copy "));
  else display.print(F("Move "));
  strlcpy_P(textBuff, str, sizeof(textBuff));
  display.print(textBuff);
  display.print(srcIdx + 1); 
  display.setCursor(17, 28);
  display.print(F("to:"));
  display.setCursor(47, 28);
  display.print(textBuff);
  display.print(destIdx + 1);
  drawCursor(39, 28);
}

//--------------------------------------------------------------------------------------------------

void drawHorizontalBarChart(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, int32_t val, int32_t valMin, int32_t valMax)
{
  display.drawRect(x, y, w, h, color);
  if(val < valMin) 
    val = valMin;
  if(val > valMax) 
    val = valMax;
  if(valMin > valMax) 
    return;
  if(valMax - valMin == 0) 
    return;
  w = divRoundClosest(((int32_t)w - 2) * (val - valMin), valMax - valMin);
  display.fillRect(x+1, y, w, h, color);
}

//--------------------------------------------------------------------------------------------------

void drawHorizontalBarChartZeroCentered(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, int32_t val, int32_t range)
{
  int8_t xCenter = x + w/2;
  if(range != 0)
  {
    if(range < 0) 
      range = -range;
    if(val > 0)
    {
      if(val > range/2) 
        val = range/2;
      int8_t xLen = (val*w)/range;
      display.fillRect(xCenter, y, xLen + 1, h, color);
    }
    else if(val < 0)
    {
      val = -val;
      if(val > range/2) 
        val = range/2;
      int8_t xLen = (val*w)/range;
      display.fillRect(xCenter - xLen, y, xLen, h, color);
    }
  }
  drawDottedHLine(x, y + h/2, w, color, !color);
  display.drawVLine(xCenter, y, h, color);
}

//--------------------------------------------------------------------------------------------------

void drawCustomCurve(custom_curve_t *crv, uint8_t selectPt, uint8_t src)
{
  //--- draw axes
  display.drawVLine(100, 11, 51, BLACK);
  display.drawHLine(75, 36, 51, BLACK);

  //--- draw cross hairs
  if(src != SRC_NONE)
  {
    int16_t xQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
    int16_t yQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
    for(uint8_t pt = 0; pt < crv->numPoints; pt++)
    {
      xQQ[pt] = 5 * crv->xVal[pt];
      yQQ[pt] = 5 * crv->yVal[pt];
    }
    drawDottedVLine(100 + mixSources[src]/20, 11, 51, BLACK, WHITE);
    int8_t y;
    if(crv->smooth) 
      y = cubicHermiteInterpolate(xQQ, yQQ, crv->numPoints, mixSources[src]) / 20;
    else 
      y = linearInterpolate(xQQ, yQQ, crv->numPoints, mixSources[src]) / 20;
    drawDottedHLine(75, 36 - y, 51, BLACK, WHITE);
  }

  //--- plot graph. Plot area is 50x50 px
  if(crv->smooth)
  {
    bool changed = false;
    //Instead of wasting RAM storing all previous xVal and yVal, we simply store a 
    //checksum of the curve struct data that has been passed to this function. 
    //We then compare the current checksum to the previous checksum.
    //We also compare pointers to the curve struct, just in case there were collisions
    static uint8_t lastChecksum;
    uint8_t checksum = crc8((uint8_t*)crv, sizeof(Model.CustomCurve[0]));
    if(checksum != lastChecksum)
    {
      lastChecksum = checksum;
      changed = true;
    }
    static custom_curve_t *lastCrv = NULL;
    if(lastCrv != crv)
    {
      lastCrv = crv;
      changed = true;
    }
    
    //recalculate if changed or invalid
    if(changed || graphYCoordinatesInvalid)
    {
      graphYCoordinatesInvalid = false;

      int16_t xQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
      int16_t yQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
      for(uint8_t pt = 0; pt < crv->numPoints; pt++)
      {
        xQQ[pt] = 5 * crv->xVal[pt];
        yQQ[pt] = 5 * crv->yVal[pt];
      }
      //compute y coordinate
      for(int8_t xCoord = -25; xCoord <= 25; xCoord++)
      {
        uint8_t i = 25 + xCoord;
        mySharedUnion.graphYCoord[i] = cubicHermiteInterpolate(xQQ, yQQ, crv->numPoints, xCoord * 20) / 20; 
      }
    }
    
    //plot
    for(uint8_t pt = 0; pt < crv->numPoints - 1; pt++)
    {
      int8_t xStart = crv->xVal[pt]/4;
      int8_t xEnd   = crv->xVal[pt+1]/4;
      int8_t yStart = crv->yVal[pt]/4;
      int8_t yEnd   = crv->yVal[pt+1]/4;
      
      //exit if values are invalid
      if(xStart < -25 || xStart > 25 || xEnd < -25 || xEnd > 25) 
        break;
      
      if(xEnd - xStart <= 1)
        display.drawLine(100 + xStart, 36 - yStart, 100 + xEnd, 36 - yEnd, BLACK);
      else
      {
        for(int8_t xCoord = xStart; xCoord <= xEnd; xCoord++)
        {
          uint8_t i = 25 + xCoord;
          //If the difference between successive y coordinates is more than 1 pixel then draw a line 
          //between the two points to make the graph visually continuous (not broken)
          if(xCoord == xStart)
            mySharedUnion.graphYCoord[i] = yStart;
          if(xCoord == xEnd)
            mySharedUnion.graphYCoord[i] = yEnd;
          if(xCoord > xStart && (abs(mySharedUnion.graphYCoord[i] - mySharedUnion.graphYCoord[i-1]) > 1))
            display.drawLine(100 + xCoord - 1, 36 - mySharedUnion.graphYCoord[i-1], 100 + xCoord, 36 - mySharedUnion.graphYCoord[i], BLACK); 
          else
            display.drawPixel(100 + xCoord, 36 - mySharedUnion.graphYCoord[i], BLACK);
        }
      }
    }
  }
  else
  {
    //linear interpolate between points.
    //here we simply draw straight lines between the points.
    for(uint8_t pt = 0; pt < crv->numPoints - 1; pt++)
    {
      display.drawLine(100 + crv->xVal[pt]/4, 36 - crv->yVal[pt]/4, 
                       100 + crv->xVal[pt+1]/4, 36 - crv->yVal[pt+1]/4, BLACK);
    }
  }
  
  //--- mark nodes, show node we've selected
  for(uint8_t pt = 0; pt < crv->numPoints; pt++)
  {
    //mark nodes
    display.fillRect(99 + crv->xVal[pt]/4, 35 - crv->yVal[pt]/4 ,3, 3, BLACK);
    display.drawPixel(100 + crv->xVal[pt]/4, 36 - crv->yVal[pt]/4, WHITE);
    //show selected node
    if(pt == selectPt)
    {
      display.fillRect(98 + crv->xVal[pt]/4, 34 - crv->yVal[pt]/4, 5, 5, WHITE);
      display.drawRect(98 + crv->xVal[pt]/4, 34 - crv->yVal[pt]/4, 5, 5, BLACK);
    }
  }
}

//--------------------------------------------------------------------------------------------------

void drawMixerCurvePreview(mixer_params_t* mxr, bool showCrossHairs)
{
  display.drawVLine(114, 40, 21, BLACK);
  display.drawHLine(104, 50, 21, BLACK);
  
  //only recalculate points if invalid to avoid unnecessary computations
  static int16_t lastChecksum = 0;
  int16_t checksum = 0;
  checksum += mxr->weight;
  checksum += mxr->offset;
  checksum += mxr->curveType;
  checksum += mxr->curveVal;
  if(checksum != lastChecksum)
  {
    lastChecksum = checksum;
    graphYCoordinatesInvalid = true;
  }

  const int8_t xStart = -10;
  const int8_t xEnd = 10;

  if(graphYCoordinatesInvalid)
  {
    graphYCoordinatesInvalid = false;

    int16_t _xx[MAX_NUM_POINTS_CUSTOM_CURVE];
    int16_t _yy[MAX_NUM_POINTS_CUSTOM_CURVE];
    if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
    { 
      uint8_t _crvIdx = mxr->curveVal;
      uint8_t _numPts = Model.CustomCurve[_crvIdx].numPoints;
      for(uint8_t pt = 0; pt < _numPts; pt++)
      {
        _xx[pt] = 5 * Model.CustomCurve[_crvIdx].xVal[pt];
        _yy[pt] = 5 * Model.CustomCurve[_crvIdx].yVal[pt];
      }
    }
    
    for(int8_t xCoord = xStart; xCoord <= xEnd; xCoord++)
    {
      int16_t operand = (int16_t) xCoord * 50;
      //--- curves
      if(mxr->curveType == MIX_CURVE_TYPE_EXPO)
        operand = calcExpo(operand, mxr->curveVal);
      else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
      {
        uint8_t _crvIdx = mxr->curveVal;
        uint8_t _numPts = Model.CustomCurve[_crvIdx].numPoints;
        if(Model.CustomCurve[_crvIdx].smooth)
          operand = cubicHermiteInterpolate(_xx, _yy, _numPts, operand);
        else
          operand = linearInterpolate(_xx, _yy, _numPts, operand);
      }
      else if(mxr->curveType == MIX_CURVE_TYPE_FUNCTION)
      {
        uint8_t _func = mxr->curveVal;
        switch(_func)
        {
          case MIX_CURVE_FUNC_X_GREATER_THAN_ZERO: 
            if(operand < 0) operand = 0; 
            break;
          case MIX_CURVE_FUNC_X_LESS_THAN_ZERO: 
            if(operand > 0) operand = 0; 
            break;
          case MIX_CURVE_FUNC_ABS_X: 
            if(operand < 0) operand = -operand; 
            break;
        }
      }
      //--- weight, offset
      operand = weightAndOffset(operand, mxr->weight, mxr->offset);
      
      //--- differential
      if(mxr->curveType == MIX_CURVE_TYPE_DIFF && mxr->curveVal != 0)
        operand = calcDifferential(operand, mxr->curveVal);
     
      //--- write to array
      int8_t yCoord = operand / 50;
      yCoord = constrain(yCoord, -10, 10);
      uint8_t i = xCoord - xStart;
      mySharedUnion.graphYCoord[i] = yCoord;
    }
  }

  //--- Plot
  if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
  {
    for(int8_t xCoord = xStart; xCoord <= xEnd; xCoord++)
    {
      uint8_t i = xCoord - xStart;
      display.drawPixel(104 + i, 50 - mySharedUnion.graphYCoord[i], BLACK);
    }
  }
  else
  {
    for(int8_t xCoord = xStart; xCoord <= xEnd; xCoord++)
    {
      uint8_t i = xCoord - xStart;
      //If the difference between successive y coordinates is more than 1 pixel then draw a line 
      //between the two points to make the graph visually continuous (not broken)
      if(xCoord > xStart && (abs(mySharedUnion.graphYCoord[i] - mySharedUnion.graphYCoord[i-1]) > 1))
        display.drawLine(103 + i, 50 - mySharedUnion.graphYCoord[i-1], 104 + i, 50 - mySharedUnion.graphYCoord[i], BLACK); 
      else
        display.drawPixel(104 + i, 50 - mySharedUnion.graphYCoord[i], BLACK);
    }
  }

  //add input and output markers
  if(showCrossHairs && mxr->input != SRC_NONE && mxr->output != SRC_NONE)
  {
    uint8_t x = 114 + divRoundClosest(focusedMixInputVal, 50);
    drawDottedVLine(x, 40, 21, BLACK, WHITE);
    drawDottedHLine(104, 50 - divRoundClosest(focusedMixOutputVal, 50), 21, BLACK, WHITE);
    
#if defined (DISPLAY_KS0108)
    //selectively turn off interlacing to prevent artifacts
    static uint8_t lastX;
    if(lastX != x)
    {
      lastX = x;
      display.setInterlace(false);
    }
#endif

  }
}

//--------------------------------------------------------------------------------------------------

void drawTooltip(uint8_t x, uint8_t y, char* str)
{
  uint8_t lenStr = strlen(str);
  if(lenStr < 3)
    lenStr = 3;
  uint8_t widthPix = (lenStr * 6) + 3;

  int8_t xqq = x - 7; //start position of main rectangle
  if(xqq < 0)
    xqq = 0;
  if(xqq + widthPix > display.width() - 2)
    xqq = display.width() - 2 - widthPix;
  
  //clear background
  display.fillRect(xqq - 1, y - 15, widthPix + 3, 16, WHITE);

  //draw main rectangle
  if(Sys.useRoundRect)
    display.drawRoundRect(xqq, y - 14, widthPix, 11, 1, BLACK);
  else
    display.drawRect(xqq, y - 14, widthPix, 11, BLACK);
  
  //draw shadow
  if(Sys.useRoundRect)
  {
    display.drawHLine(xqq + 2, y - 3, widthPix - 2, BLACK);
    display.drawVLine(xqq + widthPix, y - 12, 9, BLACK);
    display.drawPixel(xqq + widthPix - 1, y - 4, BLACK);
  }
  else
  {
    display.drawHLine(xqq + 1, y - 3, widthPix, BLACK);
    display.drawVLine(xqq + widthPix, y - 13, 11, BLACK);
  }

  //draw connector
  if(x < 5)
  {
    if(x < 1)
      x = 1;
    display.drawBitmap(x - 1, y - 4, tooltip_connector_down_left, 5, 5, BLACK, WHITE);
  }
  else if(x > display.width() - 7)
  {
    if(x > display.width() - 3)
      x = display.width() - 3;
    display.drawBitmap(x - 3, y - 4, tooltip_connector_down_right, 5, 5, BLACK, WHITE);
  }
  else
    display.drawBitmap(x - 2, y - 4, tooltip_connector_down, 6, 5, BLACK, WHITE);
  
  //print the text middle aligned
  uint8_t xOffset = ((widthPix - strlen(str) * 6) / 2) + 1;
  display.setCursor(xqq + xOffset, y - 12);
  display.print(str);
}

//--------------------------------------------------------------------------------------------------

void drawTelemetryValue(uint8_t xpos, uint8_t ypos, uint8_t idx, int16_t rawVal, bool blink)
{  
  if(rawVal == TELEMETRY_NO_DATA)
  {
    display.setCursor(xpos, ypos);
    display.print(F("No data"));
    return;
  }

  if(blink && telemetryAlarmState[idx] && (millis() % 1000 > 700)) //flashing effect
    return;
  int32_t tVal = ((int32_t) rawVal * Model.Telemetry[idx].multiplier) / 100;
  tVal += Model.Telemetry[idx].offset;

  display.setCursor(xpos, ypos);
  printTelemParam(idx, tVal, true);
}

//--------------------------------------------------------------------------------------------------

void printTelemParam(uint8_t idx, int32_t val, bool showUnits)
{
  int8_t qq = Model.Telemetry[idx].factor10;
  if(qq < 0)
    printFixedPointVal(val, -qq);
  else if(qq == 0)
    display.print(val);
  else if(qq > 0)
  {
    display.print(val);
    if(val != 0)
    {
      while(qq)
      {
        display.print(F("0"));
        qq--;
      }
    }
  }

  if(showUnits)
  {
    bool addSpaceBeforeUnit = true;
    uint8_t a = Model.Telemetry[idx].unitsName[0];
    uint8_t b = Model.Telemetry[idx].unitsName[1];
    if(b == '\0' && (a == '%' || a == 0xF8))
      addSpaceBeforeUnit = false;
    if(addSpaceBeforeUnit)
      display.setCursor(display.getCursorX() + 3, display.getCursorY());
    display.print(Model.Telemetry[idx].unitsName);
  }
}

//--------------------------------------------------------------------------------------------------

void editTextDialog(const char* title, char* buff, uint8_t lenBuff, bool allowEmpty, bool trimStr, bool isSecureMode)
{
  static uint8_t charPos = 0;
  uint8_t thisChar = *(buff + charPos);
  if(thisChar == 0) //handle partial string
  {
    thisChar = 32;
    //write a null at next position so we don't see leftover garbage strings
    *(buff + charPos + 1) = '\0';
  }
  
  //--- draw ---
  drawBoundingBox(11, 14, 105, 35,BLACK);
  display.setCursor(17, 18);
  strlcpy_P(textBuff, title, sizeof(textBuff));
  display.print(textBuff);
  
  uint8_t textWidthPix = (lenBuff - 1) * 6;
  uint8_t xpos = (display.width() - textWidthPix) / 2; //middle align
  display.setCursor(xpos, 28);
  for(uint8_t i = 0; i < lenBuff; i++)
  {
    uint8_t c = *(buff + i);
    if(c == '\0')
      break;
    if(isSecureMode && charPos != i) // mask/hide entered characters
      display.write(0x20); 
    else
      display.write(c);
  }
  
  display.drawHLine(xpos, 37, (6 * (lenBuff - 1)) - 1, BLACK);
  
  //draw blinking cursor
  if((millis() - buttonReleaseTime) % 1000 < 500 || buttonCode > 0)
    display.fillRect(xpos + 6 * charPos, 36, 5, 1, BLACK);
  
  //---- map characters ---
  // 9 to 0  (57 ... 48)   -->  0 ... 9
  // Z to A  (90 ... 65)   -->  10 ... 35
  // space   (32)          -->  36
  // a to z  (97 ... 122)  -->  37 ... 62
  // - . /   (45 ... 47)   -->  63 ... 65
  // % symbol  (37)        -->  66
  // Degree symbol (248)   -->  67
  // Caret ^ (94)          -->  68
  // Square symbol (253)   -->  69
  // Greek letter mu (230) -->  70

  if(thisChar == 32) thisChar = 36;
  else if(thisChar == 37)  thisChar = 66;
  else if(thisChar == 94)  thisChar = 68;
  else if(thisChar == 230) thisChar = 70;
  else if(thisChar == 248) thisChar = 67;
  else if(thisChar == 253) thisChar = 69;
  else if(thisChar <= 47)  thisChar += 18;
  else if(thisChar <= 57)  thisChar = 57 - thisChar;
  else if(thisChar <= 90)  thisChar = 100 - thisChar;
  else if(thisChar <= 122) thisChar -= 60;
  
  //adjust
  isEditMode = true;
  thisChar = incDec(thisChar, 70, 0, INCDEC_NOWRAP, INCDEC_SLOW);

  //map back
  if(thisChar == 36) thisChar = 32; 
  else if(thisChar == 66) thisChar = 37;
  else if(thisChar == 67) thisChar = 248;
  else if(thisChar == 68) thisChar = 94;
  else if(thisChar == 69) thisChar = 253;
  else if(thisChar == 70) thisChar = 230;
  else if(thisChar <= 9)  thisChar = 57 - thisChar;
  else if(thisChar <= 35) thisChar = 100 - thisChar;
  else if(thisChar <= 62) thisChar = 60 + thisChar;
  else if(thisChar <= 65) thisChar = thisChar - 18;

  //write
  *(buff + charPos) = thisChar;
  
  //change to next or prev character
  if(clickedButton == KEY_SELECT)
    charPos++;
  else if(heldButton == KEY_SELECT && charPos > 0)
  {
    //move to prev character if not secure mode
    if(!isSecureMode)
      charPos--;
    killButtonEvents();
  }
  
  //clear text
  if(charPos == 0 && heldButton == KEY_SELECT && millis() - buttonStartTime >= 1000)
  {
    killButtonEvents();
    *buff = '\0';
  }
  
  //done, exit 
  if(charPos == (lenBuff - 1))
  {
    charPos = 0;
    if(isEmptyStr(buff, lenBuff) && !allowEmpty)
      makeToast(PSTR("Cannot be empty"), 2000, 0);
    else
    {
      isEditTextDialog = false;
      isEditMode = false;
      if(trimStr) 
        trimWhiteSpace(buff, lenBuff);
    }
  }
}

//--------------------------------------------------------------------------------------------------

void validatePassword(uint8_t nextScreen, uint8_t prevScreen)
{
  isEditTextDialog = true;
  static char enteredPassword[sizeof(Sys.password)];
  editTextDialog(PSTR("Enter password"), enteredPassword, sizeof(Sys.password), true, false, true);
  if(!isEditTextDialog) //exited
  {
    if(strcmp(enteredPassword, Sys.password) == 0)
    {
      changeToScreen(nextScreen);
      enteredPassword[0] = '\0';
      mainMenuLocked = false;
    }
    else
    {
      if(!isEmptyStr(enteredPassword, sizeof(Sys.password)))
        makeToast(PSTR("Incorrect password"), 2000, 0);
      changeToScreen(prevScreen);
      enteredPassword[0] = '\0';
    }
  }
}

//--------------------------------------------------------------------------------------------------

void drawNotificationOverlay(uint8_t idx, uint32_t startTime, uint32_t endTime)
{
  //Table for easing the slide up/down animation.
  //t ranges from 0 to 1. Here it is actually the ratio (currTime - startTime)/transitionDuration
  //Values have been multiplied by distance to move and rounded to zero decimal places.
  static const uint8_t transitionLUT[] PROGMEM = {
    0,2,4,6,7,8,9,9,10,10,10,10,10,10,10,10 //quartic 1-pow(1-t,4)
  };
  
  const uint8_t numElem = sizeof(transitionLUT)/sizeof(transitionLUT[0]);
  const int16_t transitionDuration = 300; //in milliseconds
  
  uint32_t currTime = millis();
  if(currTime >= startTime && currTime < endTime)
  {
    uint8_t ypos = 53;
    if(Sys.animationsEnabled)
    {
      uint8_t idxLUT = numElem - 1;
      if(currTime < startTime + transitionDuration) //slide up
        idxLUT = ((currTime - startTime) * (numElem - 1)) / transitionDuration;
      else if((currTime + transitionDuration) > endTime) //slide down
        idxLUT = ((endTime - currTime) * (numElem - 1)) / transitionDuration;
      ypos = 63 - pgm_read_byte(&transitionLUT[idxLUT]);
    }

    uint8_t textWidthPix = 6 * strlen(Model.CustomNotification[idx].text); 
    uint8_t xpos = (128 - textWidthPix) / 2; //middle align text
    display.drawRoundRect(xpos - 5, ypos, textWidthPix + 9, 11, Sys.useRoundRect ? 4 : 0, WHITE);
    display.fillRoundRect(xpos - 4, ypos + 1, textWidthPix + 7, 9, Sys.useRoundRect ? 3 : 0, BLACK);
    display.setCursor(xpos, ypos + 2);
    display.setTextColor(WHITE);
    display.print(Model.CustomNotification[idx].text);
    display.setTextColor(BLACK);
  }
}

//---------------------------- Menus and context menus ---------------------------------------------

#define _MENU_MAX_ITEMS 16

const char* _menuItems[_MENU_MAX_ITEMS];
uint8_t _menuItemIDs[_MENU_MAX_ITEMS];
const uint8_t* _menuItemIcons[_MENU_MAX_ITEMS];
uint8_t _menuItemCount = 0;

// variables to help in horizontal scrolling for lengthy text
bool _scrollInitialised = false;
uint8_t _counter = 0;
uint32_t _loopNumOffset;
bool _scrollStarted = false;

void menuInitialise()
{
  _menuItemCount = 0;
}

void menuAddItem(const char* str, uint8_t itemID, const uint8_t* icon)
{
  if(_menuItemCount < _MENU_MAX_ITEMS)
  {
    _menuItems[_menuItemCount] = str;
    _menuItemIDs[_menuItemCount] = itemID;
    _menuItemIcons[_menuItemCount] = icon;
    _menuItemCount++;
  }
}

uint8_t menuGetItemCount()
{
  return _menuItemCount;
}

void menuDraw(uint8_t *topItem, uint8_t *highlightedItem)
{
  if(_menuItemCount == 0)
    return;
  
  uint8_t maxVisible = 4;
  uint8_t lineHeight = 13;
  uint8_t y0 = 14;
  if(Sys.useDenserMenus)
  {
    maxVisible = 5;
    lineHeight = 11;
    y0 = 11;
  }
  
  uint8_t numVisible = (_menuItemCount <= maxVisible) ? _menuItemCount : maxVisible;
  
  //handle scenario of being called with invalid highlightedItem
  if(*highlightedItem > _menuItemCount) 
    *highlightedItem = 1;
  
  //handle navigation
  isEditMode = true;
  *highlightedItem = incDec(*highlightedItem, _menuItemCount, 1, INCDEC_WRAP, INCDEC_SLOW);
  isEditMode = false;
  if(*highlightedItem < *topItem)
    *topItem = *highlightedItem;
  while(*highlightedItem >= *topItem + maxVisible)
    (*topItem)++;

  //check if there are any icons in the menu
  bool hasMenuIcons = false;
  for(uint8_t i = 0; i < _menuItemCount; i++)
  {
    if(_menuItemIcons[i] != NULL)
    {
      hasMenuIcons = true;
      break;
    }
  }

  //horizontal scrolling for lengthy text
  if(!_scrollInitialised)
  {
    _scrollInitialised = true;
    _counter = 0;
    _loopNumOffset = thisLoopNum;
    _scrollStarted = false;
  }
  if(buttonCode == KEY_UP || buttonCode == KEY_DOWN || heldButton == KEY_SELECT)
  {
    _scrollInitialised = false;
  }
  
  //fill menu slots
  for(uint8_t line = 0; line < numVisible; line++)
  {
    //prevent showing garbage entries when we've changed to using denser menus
    if(*topItem + line > _menuItemCount)
      break;
    
    uint8_t item = *topItem + line;
    uint8_t ypos = y0 + line * lineHeight;

    //highlight selection
    bool isFocused = false;
    if(*highlightedItem == item)
    {
      isFocused = true;
      display.fillRoundRect(2, Sys.useDenserMenus ? ypos - 2 : ypos - 3, _menuItemCount <= maxVisible ? 124 : 123, 
                            lineHeight, Sys.useRoundRect ? 4 : 0, BLACK);
      display.setTextColor(WHITE);
    }

    //show icon
    if(_menuItemIcons[item - 1] != NULL && Sys.showMenuIcons)
      display.drawBitmap(7, ypos - 2 , _menuItemIcons[item - 1], 15, 11, *highlightedItem == item ? WHITE : BLACK);
    
    //show text
    display.setCursor((hasMenuIcons && Sys.showMenuIcons) ? 26 : 10, ypos);
    strlcpy_P(textBuff, _menuItems[item - 1], sizeof(textBuff));
    uint8_t maxVisibleCharacters = hasMenuIcons ? 16 : 19;
    if(strlen(textBuff) <= maxVisibleCharacters)
    {
      display.print(textBuff);
    }
    else //long text, horizontal scroll it
    {
      if(isFocused)
      {
        uint8_t lenStr = strlen(textBuff);
        uint8_t lenTotal = lenStr + 6; //6 extra spaces inserted in-between tail and head
        for(uint8_t i = 0; i < maxVisibleCharacters; i++)
        {
          uint8_t idx = (_counter + i) % lenTotal;
          char c = (idx < lenStr) ? textBuff[idx] : 0x20;
          display.print(c);
        }
        uint16_t scrollDelay = _scrollStarted ? 240 : 600;
        if((thisLoopNum - _loopNumOffset + 1) % divRoundClosest(scrollDelay, fixedLoopTime) == 0)
        {
          _counter++;
          if(_counter >= lenTotal)
            _counter = 0;
          _loopNumOffset = thisLoopNum; //reset the offset
          _scrollStarted = true;
        }
      }
      else
      {
        for(uint8_t i = 0; i < maxVisibleCharacters - 1; i++)
        {
          char c = textBuff[i];
          display.print(c);
        }
        //show an ellipsis character
        uint8_t x = display.getCursorX();
        uint8_t y = ypos + 6;
        display.drawPixel(x, y, BLACK);
        display.drawPixel(x + 2, y, BLACK);
        display.drawPixel(x + 4, y, BLACK);
      }
    }
    display.setTextColor(BLACK);
  }
  
  //scroll bar
  drawScrollBar(127, Sys.useDenserMenus ? 9 : 11, _menuItemCount, *topItem, numVisible, numVisible * lineHeight);

  //get the id of the item selected
  menuSelectedItemID = 0xff;
  if(clickedButton == KEY_SELECT)
  {
    menuSelectedItemID = _menuItemIDs[*highlightedItem - 1];
    //reset some variables
    _scrollInitialised = false;
  }
}

void contextMenuInitialise()
{
  _menuItemCount = 0;
}

void contextMenuAddItem(const char* str, uint8_t itemID)
{
  if(_menuItemCount < _MENU_MAX_ITEMS)
  {
    _menuItems[_menuItemCount] = str;
    _menuItemIDs[_menuItemCount] = itemID;
    _menuItemCount++;
  }
}

uint8_t contextMenuGetItemCount()
{
  return _menuItemCount;
}

void contextMenuDraw()
{
  if(_menuItemCount == 0)
    return;
  
  //--- scrollable list
  
  const uint8_t maxVisible = 5;
  
  //handle navigation
  isEditMode = true;
  contextMenuFocusedItem = incDec(contextMenuFocusedItem, _menuItemCount, 1, INCDEC_WRAP, INCDEC_SLOW);
  isEditMode = false;
  if(contextMenuFocusedItem < contextMenuTopItem)
    contextMenuTopItem = contextMenuFocusedItem;
  while(contextMenuFocusedItem >= contextMenuTopItem + maxVisible)
    contextMenuTopItem++;

  //Calculate y coordinate for text item 0. Items are center aligned
  uint8_t numVisible = _menuItemCount <= maxVisible ? _menuItemCount : maxVisible;
  uint8_t y0 = ((display.height() - (numVisible * 10)) / 2) + 1;  //10 is line height
  
  //draw bounding box
  drawBoundingBox(3, y0 - 4, 122, numVisible * 10 + 5, BLACK);  
  
  //horizontal scrolling for lengthy text
  if(!_scrollInitialised)
  {
    _scrollInitialised = true;
    _counter = 0;
    _loopNumOffset = thisLoopNum;
    _scrollStarted = false;
  }
  if(buttonCode == KEY_UP || buttonCode == KEY_DOWN || heldButton == KEY_SELECT)
  {
    _scrollInitialised = false;
  }
  
  //fill list
  for(uint8_t line = 0; line < numVisible; line++)
  {
    uint8_t ypos = y0 + line * 10;
    uint8_t item = contextMenuTopItem + line;
    strlcpy_P(textBuff, _menuItems[item-1], sizeof(textBuff));
    
    bool isFocused = false;
    if(item == contextMenuFocusedItem)
    {
      isFocused = true;
      display.fillRoundRect(5, ypos - 2, _menuItemCount <= maxVisible ? 118 : 114, 11, Sys.useRoundRect ? 4 : 0, BLACK);
      display.setTextColor(WHITE);
    }
    
    display.setCursor(9, ypos);
    if(strlen(textBuff) <= 18)
    {
      display.print(textBuff);
    }
    else //long text, horizontal scroll it
    {
      if(isFocused)
      {
        uint8_t lenStr = strlen(textBuff);
        uint8_t lenTotal = lenStr + 6; //6 extra spaces inserted in-between tail and head
        for(uint8_t i = 0; i < 18; i++)
        {
          uint8_t idx = (_counter + i) % lenTotal;
          char c = (idx < lenStr) ? textBuff[idx] : 0x20;
          display.print(c);
        }
        uint16_t scrollDelay = _scrollStarted ? 240 : 600;
        if((thisLoopNum - _loopNumOffset + 1) % divRoundClosest(scrollDelay, fixedLoopTime) == 0)
        {
          _counter++;
          if(_counter >= lenTotal)
            _counter = 0;
          _loopNumOffset = thisLoopNum; //reset the offset
          _scrollStarted = true;
        }
      }
      else
      {
        for(uint8_t i = 0; i < 17; i++)
        {
          char c = textBuff[i];
          display.print(c);
        }
        //show an ellipsis character
        uint8_t x = display.getCursorX();
        uint8_t y = ypos + 6;
        display.drawPixel(x, y, BLACK);
        display.drawPixel(x + 2, y, BLACK);
        display.drawPixel(x + 4, y, BLACK);
      }
    }
    
    display.setTextColor(BLACK);
  }
  
  //scroll bar
  uint8_t  y = Sys.useRoundRect ? y0 - 1 : y0 - 2;
  uint16_t h = Sys.useRoundRect ? numVisible * 10 - 1 : numVisible * 10 + 1;
  drawScrollBar(121, y, _menuItemCount, contextMenuTopItem, numVisible, h);
  
  //get the id of the selected item
  contextMenuSelectedItemID = 0xff;
  if(clickedButton == KEY_SELECT)
  {
    contextMenuSelectedItemID = _menuItemIDs[contextMenuFocusedItem - 1];
    //reset some variables
    _scrollInitialised = false;
  }
}

#endif //UI_128X64
