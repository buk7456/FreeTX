
#include "Arduino.h"

#include "../../config.h"
#include "../common.h"
#include "../stringDefs.h"
#include "../crc.h"
#include "../inputs.h"
#include "../mathHelpers.h"
#include "../mixer.h"
#include "../mtx.h"
#include "../templates.h"
#include "../tonePlayer.h"
#include "../ee/eestore.h"
#include "../sd/sdStore.h"
#include "bitmaps.h"
#include "about.h"
#include "ui.h"
#include "uiCommon.h"

#if defined (UI_128X64)

#if defined (DISPLAY_KS0108)
#include "../lcd/GFX.h"
#include "../lcd/LCDKS0108.h"
LCDKS0108 display = LCDKS0108(PIN_KS_RS, PIN_KS_EN, PIN_KS_CS1, PIN_KS_CS2);
#endif

//------ Menu strings --------
// Max 19 characters per string

char const mainMenu[][20] PROGMEM = { 
  "Model", "Inputs", "Mixer", "Outputs", "Extras", "Telemetry", "System", "Receiver"
};
enum {
  MAIN_MENU_MODEL, MAIN_MENU_INPUTS, MAIN_MENU_MIXER, MAIN_MENU_OUTPUTS, MAIN_MENU_EXTRAS,
  MAIN_MENU_TELEMETRY, MAIN_MENU_SYSTEM, MAIN_MENU_RECEIVER
};

char const extrasMenu[][20] PROGMEM = { 
  "Custom curves", "Functn generators", "Logical switches", "Timers", "Counters", "Notifications", 
  "Safety checks", "Trim setup", "Flight modes"
};
enum {
  EXTRAS_MENU_CUSTOM_CURVES, EXTRAS_MENU_FUNCTION_GENERATORS, EXTRAS_MENU_LOGICAL_SWITCHES,
  EXTRAS_MENU_TIMERS, EXTRAS_MENU_COUNTERS, EXTRAS_MENU_NOTIFICATION_SETUP, EXTRAS_MENU_SAFETY_CHECKS,
  EXTRAS_MENU_TRIM_SETUP, EXTRAS_MENU_FLIGHT_MODES
};

char const systemMenu[][20] PROGMEM = { 
  "RF setup", "Sound", "Backlight", "Appearance", "Advanced"
};
enum {
  SYSTEM_MENU_RF, SYSTEM_MENU_SOUND, SYSTEM_MENU_BACKLIGHT, SYSTEM_MENU_APPEARANCE, SYSTEM_MENU_ADVANCED
};

char const advancedMenu[][20] PROGMEM = { 
  "Sticks", "Knobs", "Switches", "Battery", "Security", "Miscellaneous", "Debug", "About"
};
enum {
  ADVANCED_MENU_STICKS, ADVANCED_MENU_KNOBS, ADVANCED_MENU_SWITCHES, 
  ADVANCED_MENU_BATTERY, ADVANCED_MENU_SECURITY,
  ADVANCED_MENU_MISCELLANEOUS, ADVANCED_MENU_DEBUG,
  ADVANCED_MENU_ABOUT
};

//---------------------------- Main UI states ------------------------------------------------------

enum {
  //--- Home screen and related
  SCREEN_HOME,
  SCREEN_CHANNEL_MONITOR,
  CONTEXT_MENU_HOME_SCREEN,
  SCREEN_WIDGET_SETUP,
  CONTEXT_MENU_WIDGETS,
  DIALOG_COPY_WIDGET,
  DIALOG_MOVE_WIDGET,
  
  //---- Main menu ----
  SCREEN_UNLOCK_MAIN_MENU,
  SCREEN_MAIN_MENU,
  
  //---- Model ----
  SCREEN_UNLOCK_MODEL_MANAGER,
  SCREEN_MODEL,
  CONTEXT_MENU_ACTIVE_MODEL,
  CONTEXT_MENU_INACTIVE_MODEL,
  CONTEXT_MENU_FREE_MODEL,
  DIALOG_MODEL_TYPE,
  DIALOG_RENAME_MODEL,
  DIALOG_COPYFROM_MODEL,
  CONFIRMATION_MODEL_BACKUP,
  DIALOG_RESTORE_MODEL,
  CONFIRMATION_MODEL_RESTORE,
  CONFIRMATION_MODEL_COPY,
  CONFIRMATION_MODEL_DELETE,
  CONFIRMATION_MODEL_RESET,
  CONFIRMATION_CREATE_MODEL,
  
  //---- Inputs ----
  SCREEN_INPUTS,
  
  //---- Mixer ----
  SCREEN_MIXER,
  DIALOG_MIX_FLIGHT_MODE,
  CONTEXT_MENU_MIXER,
  SCREEN_MIXER_OUTPUT,
  DIALOG_COPY_MIX,
  DIALOG_MOVE_MIX,
  DIALOG_RENAME_MIX,
  CONFIRMATION_MIXES_RESET,
  CONTEXT_MENU_MIXER_TEMPLATES,
  CONFIRMATION_LOAD_MIXER_TEMPLATE,
  SCREEN_MIXER_OVERVIEW,
  
  //---- Outputs ----
  SCREEN_OUTPUTS,
  CONTEXT_MENU_OUTPUTS,
  DIALOG_RENAME_CHANNEL,
  
  //---- Extras ---
  SCREEN_EXTRAS_MENU,
  //custom curves
  SCREEN_CUSTOM_CURVES,
  CONTEXT_MENU_CUSTOM_CURVE,
  DIALOG_RENAME_CUSTOM_CURVE,
  DIALOG_COPY_CUSTOM_CURVE,
  DIALOG_INSERT_CURVE_POINT,
  DIALOG_DELETE_CURVE_POINT,
  //logical switches
  SCREEN_LOGICAL_SWITCHES,
  CONTEXT_MENU_LOGICAL_SWITCHES,
  DIALOG_COPY_LOGICAL_SWITCH,
  DIALOG_MOVE_LOGICAL_SWITCH,
  SCREEN_LOGICAL_SWITCH_OUTPUTS,
  //counters
  SCREEN_COUNTERS,
  CONTEXT_MENU_COUNTERS,
  DIALOG_RENAME_COUNTER,
  DIALOG_COPY_COUNTER,
  CONFIRMATION_CLEAR_ALL_COUNTERS,
  //flight modes
  SCREEN_FLIGHT_MODES,
  //function generators
  SCREEN_FUNCTION_GENERATORS,
  CONTEXT_MENU_FUNCGEN,
  DIALOG_COPY_FUNCGEN,
  SCREEN_FUNCGEN_OUTPUTS,
  //trim setup
  SCREEN_TRIM_SETUP,
  //safety checks
  SCREEN_SAFETY_CHECKS,
  //timers
  SCREEN_TIMERS,
  DIALOG_TIMER_INITIAL_TIME,
  CONTEXT_MENU_TIMERS,
  //notifications
  SCREEN_NOTIFICATION_SETUP,
  CONTEXT_MENU_NOTIFICATIONS,
  DIALOG_COPY_NOTIFICATION,

  //---- Telemetry ----
  //general telemetry
  SCREEN_TELEMETRY,
  CONTEXT_MENU_ACTIVE_SENSOR,
  CONTEXT_MENU_FREE_SENSOR,
  SCREEN_CREATE_SENSOR,
  CONTEXT_MENU_SENSOR_TEMPLATES,
  SCREEN_SENSOR_STATISTICS,
  SCREEN_EDIT_SENSOR,
  CONFIRMATION_DELETE_SENSOR,
  //gnss telemetry
  SCREEN_TELEMETRY_GNSS,
  
  //---- System settings ----
  SCREEN_SYSTEM_MENU,
  SCREEN_RF,
  SCREEN_SOUND,
  SCREEN_BACKLIGHT,
  SCREEN_APPEARANCE,
  SCREEN_UNLOCK_ADVANCED_MENU,
  SCREEN_ADVANCED_MENU,
  SCREEN_STICKS,
  SCREEN_KNOBS,
  SCREEN_SWITCHES,
  SCREEN_BATTERY,
  SCREEN_SECURITY,
  SCREEN_MISCELLANEOUS,
  SCREEN_ABOUT,
  SCREEN_EASTER_EGG,
  SCREEN_DEBUG,
  SCREEN_INTERNAL_EEPROM_DUMP,
  SCREEN_EXTERNAL_EEPROM_DUMP,
  SCREEN_CHARACTER_SET,
  SCREEN_SCREENSHOT_CONFIG,
  CONFIRMATION_FACTORY_RESET,
  
  //---- Receiver ----
  SCREEN_RECEIVER,
  SCREEN_RECEIVER_CONFIG,
  SCREEN_RECEIVER_BINDING,
  
  //---- Generic text viewer ----
  SCREEN_TEXT_VIEWER,
};

//---------------------------- Misc ----------------------------------------------------------------

char textBuff[32]; //generic buffer for working with strings. Leave at 32 as it's also reused to dump eeprom.

uint8_t theScreen = SCREEN_HOME;
uint8_t lastScreen = SCREEN_HOME;

uint8_t focusedItem = 1; 

bool isEditTextDialog = false;
bool isDisplayingBattWarn = false;

//Cache the graph y coordinates to avoid unnecessary recomputation.
int8_t graphYCoord[51]; //51 points total to plot. 
bool   graphYCoordinatesInvalid = true;

//menu
uint8_t menuSelectedItemID = 0xff;

//context menu
uint8_t contextMenuTopItem = 1;
uint8_t contextMenuFocusedItem = 1;
uint8_t contextMenuSelectedItemID = 0xff;

//Model
uint8_t thisModelIdx = 0;

//mixer
uint8_t thisMixIdx = 0;
uint8_t destMixIdx = 0; 

//channel outputs
uint8_t thisChIdx = 0;

//custom curves
uint8_t thisCrvIdx = 0;
uint8_t destCrvIdx = 0;
uint8_t thisCrvPt = 0;

//function generators
uint8_t thisFgenIdx = 0;
uint8_t destFgenIdx = 0;

//logical switch
uint8_t thisLsIdx = 0;
uint8_t destLsIdx = 0;

//timers
uint8_t thisTimerIdx = 0;

//counters
uint8_t thisCounterIdx = 0;
uint8_t destCounterIdx = 0;

//notifications
uint8_t thisNotificationIdx = 0;
uint8_t destNotificationIdx = 0;

//flight modes
uint8_t thisFmdIdx = 0;

//telemetry 
uint8_t thisTelemIdx = 0;

//widgets
uint8_t thisWidgetIdx = 0;
uint8_t destWidgetIdx = 0;

//others 
bool isRequestingStickCalibration = false;
bool isRequestingKnobCalibration = false;
bool isRequestingSwitchesSetup = false;
bool batteryGaugeCalibrated = true;
bool mainMenuLocked = true;
const char* textViewerText;

//onscreen trim
bool isOnscreenTrimMode = false;
uint8_t trimIdx = 0;

//---------------------------- Function declarations -----------------------------------------------

void handleBatteryWarnUI();

void changeToScreen(uint8_t scrn);
void changeFocusOnUpDown(uint8_t numItems);
void toggleEditModeOnSelectClicked();
void drawCursor(uint8_t xpos, uint8_t ypos);
void drawHeader(const char* str);
void drawHeader_Menu(const char* str);
void drawDottedVLine(uint8_t x, uint8_t y, uint8_t len, uint8_t fgColor, uint8_t bgColor);
void drawDottedHLine(uint8_t x, uint8_t y, uint8_t len, uint8_t fgColor, uint8_t bgColor);
void drawBoundingBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void drawScrollBar(uint8_t xpos, uint8_t ypos, uint16_t numItems, uint16_t topItem, uint16_t numVisible, uint16_t viewportHeight);
void drawCheckbox(uint8_t xpos, uint8_t ypos, bool val);
void drawLoaderSpinner(uint8_t xpos, uint8_t ypos, uint8_t size);
void drawAnimatedSprite(uint8_t x, uint8_t y, const uint8_t* const bitmapTable[], uint8_t w, uint8_t h, uint8_t color, 
                        uint8_t frameCount, uint16_t frameTime, uint32_t timeOffset);
void drawHorizontalBarChart(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, int16_t val, int16_t valMin, int16_t valMax);
void drawHorizontalBarChartZeroCentered(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, int16_t val, int16_t range);
void drawTrimSliders();
void drawDialogCopyMove(const char* str, uint8_t srcIdx, uint8_t destIdx, bool isCopy);
void drawCustomCurve(custom_curve_t *crv, uint8_t selectPt, uint8_t src);
void drawToast();
void drawTelemetryValue(uint8_t xpos, uint8_t ypos, uint8_t idx, int16_t rawVal, bool blink);
void printTelemParam(uint8_t idx, int32_t val, bool showUnits);
void printFullScreenMessage(const char* str);
void printHHMMSS(int32_t millisecs);
void printVoltage(int16_t millivolts);
void printSeconds(int16_t decisecs);
void printFixedPointVal(int32_t val, uint8_t decimals);
void printModelName(char* buff, uint8_t modelIdx);
void printTimerValue(uint8_t idx);
void editTextDialog(const char* title, char* buff, uint8_t lenBuff, bool allowEmpty, bool trimStr, bool isSecureMode);
void validatePassword(uint8_t nextScreen, uint8_t prevScreen);
void resetPages();

void menuInitialise();
void menuAddItem(const char* str, uint8_t itemID, const uint8_t* icon);
void menuDraw(uint8_t *topItem, uint8_t *highlightedItem);
uint8_t menuGetItemCount();

void contextMenuInitialise();
void contextMenuAddItem(const char* str, uint8_t itemID);
void contextMenuDraw();
uint8_t contextMenuGetItemCount();

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

void showDataImportErrorMessage(const char* str, uint16_t lineNumber)
{
  display.clearDisplay();
  printFullScreenMessage(str);
  memset(textBuff, 0, sizeof(textBuff));
  char tmp[12];
  strlcpy_P(textBuff, PSTR("(Line "), sizeof(textBuff));
  utoa(lineNumber, tmp, 10);
  strlcat(textBuff, tmp, sizeof(textBuff));
  strlcat_P(textBuff, PSTR(")"), sizeof(textBuff));
  uint8_t textWidthPix = strlen(textBuff) * 6;
  display.setCursor((display.width() - textWidthPix) / 2, display.getCursorY() + 9);
  display.print(textBuff);
  display.setInterlace(false);
  display.display();
}

//============================ Battery warning =====================================================

void handleBatteryWarnUI()
{
  if(!batteryGaugeCalibrated)
    return;
  
  static uint32_t battWarnMillis = millis();
  static bool battWarnDismissed = false;
  
  if(battState == BATTLOW)
  {
    if(!battWarnDismissed)
    {
      //show warning
      display.clearDisplay();
      drawAnimatedSprite(50, 14, animation_low_batt, 27, 11, BLACK, 7, 300, battWarnMillis);
      printFullScreenMessage(PSTR("\nBattery low"));
      display.display();
      
      isDisplayingBattWarn = true;
      
      //play warning tone
      audioToPlay = AUDIO_BATTERY_WARN;
      playTones();
      
      //self dismiss warning or on a button press
      if((pressedButton > 0 || millis() - battWarnMillis > 2000))
      {
        battWarnDismissed = true;
        battWarnMillis = millis();
        isDisplayingBattWarn = false;
        //only kill button events if a button was pressed
        if(pressedButton > 0)
          killButtonEvents();
      }
    }
    //remind low battery every 10 minutes
    if(battWarnDismissed && (millis() - battWarnMillis > 600000UL)) 
    {
      battWarnDismissed = false;
      battWarnMillis = millis();
    }
  }
  else
  {
    isDisplayingBattWarn = false;
    battWarnMillis = millis();
  }
}

//============================ Safety warning ======================================================

void handleSafetyWarnUI()  //blocking function
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
    handleBatteryWarnUI();
    if(isDisplayingBattWarn)
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
      if(Model.switchWarn[i] == -1 || Sys.swType[i] == SW_ABSENT) //dont check
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
        audioToPlay = AUDIO_SAFETY_WARN;
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

//============================ Main user interface =================================================

void handleMainUI()
{
  //-------------- Keys filtering for backlight ----------
  if(Sys.backlightSuppressFirstKey && Sys.backlightEnabled)
  {
    static bool suppressDownClicked = false; //flag to kill the clickedButton event on home screen
    if(!backlightIsOn)
    {
      if(buttonCode == KEY_SELECT || buttonCode == KEY_UP)
        killButtonEvents();
      if(buttonCode == KEY_DOWN)
      {
        if(theScreen == SCREEN_HOME) 
        {
          //Only let through the held event so we can mute/unmute telemetry alarms without
          //requiring extra key action.
          pressedButton = 0;
          suppressDownClicked = true; 
        }
        else
          killButtonEvents();
      }
    }
    if(suppressDownClicked && clickedButton == KEY_DOWN)
    {
      suppressDownClicked = false;
      clickedButton = 0;
    }
  }

  //-------------- Timers -------------------------------
  timerHandler();

  //-------------- Inactivity alarm ---------------------
  inactivityAlarmHandler();
  
  //-------------- Telemetry alarms ---------------------
  telemetryAlarmHandler();
  
  //-------------- Battery warning ----------------------
  handleBatteryWarnUI();
  if(isDisplayingBattWarn)
    return;
  
  //-------------- Enable interlace mode by default -----
  //This can be overriden where necessary specially in some screens
  //to prevent interlace artifacts
  display.setInterlace(true); 

  //-------------- Main ui state machine ----------------
  switch (theScreen)
  {
    ////////////////////////////////// HOME ////////////////////////////////////////////////////////
    
    case SCREEN_HOME:
      {
        if(isRequestingStickCalibration)
        {
          changeToScreen(SCREEN_STICKS);
          break; 
        }
        
        if(isRequestingKnobCalibration)
        {
          changeToScreen(SCREEN_KNOBS);
          break; 
        }

        if(isRequestingSwitchesSetup)
        {
          changeToScreen(SCREEN_SWITCHES);
          break;
        }
        
        if(!batteryGaugeCalibrated)
        {
          changeToScreen(SCREEN_BATTERY);
          break;
        }
        
        if(!Sys.lockStartup || isEmptyStr(Sys.password, sizeof(Sys.password)))
          mainMenuLocked = false;

        //--------------- Icons -----------------
        
        uint8_t icon_xpos = 127;
        
        //Battery level indicator
        //Gauges don't indicate state of charge; only battery voltage
        if(Sys.useNumericalBatteryIndicator)
        {
          //print right aligned. 
          //Using a hack to measure the width of text first before actually printing.
          display.setCursor(0, 64);
          printVoltage(battVoltsNow);
          icon_xpos = 127 - display.getCursorX();
          display.setCursor(icon_xpos, 0);
          printVoltage(battVoltsNow);
        }
        else
        {
          icon_xpos = 113;
          display.drawRect(icon_xpos, 0, 14, 7, BLACK);
          display.drawVLine(icon_xpos + 14, 2, 3, BLACK);
          if(battState == BATTHEALTY)
          {
            static int8_t lastNumOfBars = 20;
            int8_t numOfBars = 2 + ((int32_t)(battVoltsNow - Sys.battVoltsMin) * 20) / (Sys.battVoltsMax - Sys.battVoltsMin);
            if(numOfBars > 20) 
              numOfBars = 20;
            if(numOfBars > lastNumOfBars && numOfBars - lastNumOfBars < 2) //prevent jitter at boundaries
              numOfBars = lastNumOfBars;
            for(uint8_t i = 0; i < numOfBars/2; i++)
              display.drawVLine(icon_xpos + 2 + i, 2, 3, BLACK);
            lastNumOfBars = numOfBars;
          }
        }
        
        //Draw other icons. Offset the x position by (4 + iconWidth)
        //Rf icon and tx power level as signal strength bars
        if(Sys.rfEnabled)
        {
          icon_xpos -= 17;
          display.drawBitmap(icon_xpos, 0, icon_rf, 7, 7, BLACK);
          uint8_t bars = 2 + (4 * Sys.rfPower) / (RF_POWER_COUNT - 1);
          for(uint8_t i = 0; i < bars; i++)
            display.drawVLine(icon_xpos + 6 + i, 6 - i, i + 1, BLACK);
        }

        //Mute icon
        if(!Sys.soundEnabled)
        {
          icon_xpos -= 13;
          display.drawBitmap(icon_xpos, 0, icon_mute, 8, 7, BLACK);
        }

        //Telemetry mute icon
        if(telemetryMuteAlarms && Sys.soundEnabled)
        {
          icon_xpos -= 19;
          display.drawBitmap(icon_xpos, 0, icon_telemetry_mute, 14, 7, BLACK);
        }

        //Lock icon
        if(mainMenuLocked)
        {
          icon_xpos -= 10;
          display.drawBitmap(icon_xpos, 0, icon_padlock, 5, 7, BLACK);
        }

        //------------ Trims --------------------
        static uint32_t endTime; 
        if(buttonCode >= KEY_TRIM_FIRST && buttonCode <= KEY_TRIM_LAST)
          endTime = millis() + 3000UL;
        if(!Sys.autohideTrims ||(Sys.autohideTrims && millis() < endTime) || isOnscreenTrimMode)
          drawTrimSliders();

        //------------ Model name, flight modes -------------
        //Model name
        display.setCursor(11, 9);
        printModelName(Model.name, Sys.activeModelIdx);

        //Flightmode
        //Determine if flight modes are defined other than the default. 
        //If any exist, show the active flight mode. Otherwise hide it, as user does not care.
        bool hasDefinedFlightModes = false;
        for(uint8_t i = 1; i < NUM_FLIGHT_MODES; i++)
        {
          if(Model.FlightMode[i].swtch != CTRL_SW_NONE)
          {
            hasDefinedFlightModes = true;
            break;
          }
        }
        if(hasDefinedFlightModes)
        {
          if(isEmptyStr(Model.FlightMode[activeFmdIdx].name, sizeof(Model.FlightMode[0].name)))
          {
            strlcpy_P(textBuff, PSTR("Fmd"), sizeof(textBuff));
            char suffix[5];
            memset(suffix, 0, sizeof(suffix));
            itoa(activeFmdIdx + 1, suffix, 10);
            strlcat(textBuff, suffix, sizeof(textBuff));
          }
          else
            strlcpy(textBuff, Model.FlightMode[activeFmdIdx].name, sizeof(textBuff));
          //print right aligned
          uint8_t len = strlen(textBuff) + 2; //add 2 for brackets 
          display.setCursor(118 - len * 6, 9);
          display.print(F("("));
          display.print(textBuff);
          display.print(F(")"));
        }
        
        //separator line
        display.drawHLine(11, 18, 106, BLACK);
        
        //------------ Widgets ------------------
        
        //get available telemetry sensors into a list
        uint8_t tlmQQ[NUM_WIDGETS];
        uint8_t tlmCount = 0;
        for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
        {
          if(!isEmptyStr(Model.Telemetry[i].name, sizeof(Model.Telemetry[0].name))) 
          {
            //skip sensors that are not set to show on home
            if(!Model.Telemetry[i].showOnHome)
              continue;
            //skip "no data" sensors
            if(telemetryReceivedValue[i] == TELEMETRY_NO_DATA) 
              continue;
            //skip special telemetry
            if(Model.Telemetry[i].type == TELEMETRY_TYPE_GNSS)
              continue;
            //check if already assigned to a widget
            bool skip = false;
            for(uint8_t j = 0; j < NUM_WIDGETS; j++)
            {
              if(Model.Widget[j].type == WIDGET_TYPE_TELEMETRY && Model.Widget[j].src == i)
                skip = true;
            }
            if(skip)
              continue;
            
            tlmQQ[tlmCount] = i;
            tlmCount++;
            if(tlmCount == sizeof(tlmQQ))
              break;
          }
        }
        
        //draw the widgets
        uint8_t ypos = 21;
        uint8_t printCount = 0;
        uint8_t tlmCntr = 0;
        for(uint8_t i = 0; i < NUM_WIDGETS; i++)
        {
          widget_params_t *widget = &Model.Widget[i];
          bool hasPrinted = false;
          if(widget->type == WIDGET_TYPE_TELEMETRY)
          {
            uint8_t idx = widget->src;
            if(widget->src == WIDGET_SRC_AUTO)
            { 
              if(tlmCount == 0 || tlmCntr >= tlmCount)
                continue;
              idx = tlmQQ[tlmCntr];
              tlmCntr++;
            }
            display.setCursor(11, ypos);
            display.print(Model.Telemetry[idx].name);
            display.print(F(":"));
            if(widget->disp == WIDGET_DISP_NUMERICAL || telemetryReceivedValue[idx] == TELEMETRY_NO_DATA)
              drawTelemetryValue(67, ypos, idx, telemetryReceivedValue[idx], true);
            else if(widget->disp == WIDGET_DISP_GAUGE)
            {
              bool show = true;
              if(telemetryAlarmState[idx] && (millis() % 1000 > 700))
                show = false;
              if(show)
              {
                int32_t tVal = ((int32_t) telemetryReceivedValue[idx] * Model.Telemetry[idx].multiplier) / 100;
                tVal += Model.Telemetry[idx].offset;
                drawHorizontalBarChart(70, ypos + 1, 41, 4, BLACK, tVal, widget->gaugeMin, widget->gaugeMax);
              }
            }
            hasPrinted = true;
          }
          else if(widget->type == WIDGET_TYPE_MIXSOURCES)
          {
            if(widget->src == SRC_NONE)
              continue;
            display.setCursor(11, ypos);
            getSrcName(textBuff, widget->src, sizeof(textBuff));
            display.print(textBuff);
            display.print(F(":"));
            display.setCursor(67, ypos);
            if(widget->disp == WIDGET_DISP_NUMERICAL)
              display.print(mixSources[widget->src]/5);
            else if(widget->disp == WIDGET_DISP_GAUGE)
              drawHorizontalBarChart(70, ypos + 1, 41, 4, BLACK, mixSources[widget->src]/5, widget->gaugeMin, widget->gaugeMax);
            else if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
              drawHorizontalBarChartZeroCentered(71, ypos + 2, 39, 3, BLACK, mixSources[widget->src]/5, widget->gaugeMax - widget->gaugeMin);
            hasPrinted = true;
          }
          else if(widget->type == WIDGET_TYPE_OUTPUTS)
          {
            display.setCursor(11, ypos);
            if(!isEmptyStr(Model.Channel[widget->src].name, sizeof(Model.Channel[0].name)))
              display.print(Model.Channel[widget->src].name);
            else
            {
              display.print(F("Ch"));
              display.print(widget->src + 1);
              display.print(F(" out"));
            }
            display.print(F(":"));
            display.setCursor(67, ypos);
            if(widget->disp == WIDGET_DISP_NUMERICAL)
              display.print(channelOut[widget->src]/5);
            else if(widget->disp == WIDGET_DISP_GAUGE)
              drawHorizontalBarChart(70, ypos + 1, 41, 4, BLACK, channelOut[widget->src]/5, widget->gaugeMin, widget->gaugeMax);
            else if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
              drawHorizontalBarChartZeroCentered(71, ypos + 2, 39, 3, BLACK, channelOut[widget->src]/5, widget->gaugeMax - widget->gaugeMin);
            hasPrinted = true;
          }
          else if(widget->type == WIDGET_TYPE_TIMERS)
          {
            display.setCursor(11, ypos);
            printTimerValue(widget->src);
            //print timer name
            uint8_t name_xpos = display.getCursorX() + 6;
            if(name_xpos < 67)
              name_xpos = 67;
            display.setCursor(name_xpos, ypos);
            display.print(Model.Timer[widget->src].name);
            hasPrinted = true;
          }
          else if(widget->type == WIDGET_TYPE_COUNTERS)
          {
            display.setCursor(11, ypos);
            if(!isEmptyStr(Model.Counter[widget->src].name, sizeof(Model.Counter[0].name)))
              display.print(Model.Counter[widget->src].name);
            else
            {
              display.print(F("Counter"));
              display.print(widget->src + 1);
            }
            display.print(F(":"));
            display.setCursor(67, ypos);
            display.print(counterOut[widget->src]);
            hasPrinted = true;
          }

          if(hasPrinted)
          {
            printCount++;
            ypos += 9;
          }
          //abort if no more space on the screen
          if(printCount == 4)
            break;
        }
        
        //-------------- handle keys ------------

        if(isOnscreenTrimMode)
        {
          if(heldButton == KEY_SELECT)
          {
            isOnscreenTrimMode = false;
            killButtonEvents();
            audioToPlay = AUDIO_TRIM_MODE_EXITED;
          }

          if(pressedButton == KEY_SELECT)
          {
            //suppress AUDIO_KEY_PRESSED
            if(audioToPlay == AUDIO_KEY_PRESSED)
              audioToPlay = AUDIO_NONE;
          }

          if(clickedButton == KEY_SELECT)
          {
            trimIdx++;
            if(trimIdx > 3)
              trimIdx = 0;
            audioToPlay = AUDIO_TRIM_MODE_X1 + trimIdx;
          }

          flight_mode_t* fmd = &Model.FlightMode[activeFmdIdx];
          
          //Adjust trim values
          switch(trimIdx)
          {
            case 0:
              //x1 axis
              if(Model.X1Trim.trimState == TRIM_COMMON)
                Model.X1Trim.commonTrim = adjustTrim(0, Model.X1Trim.commonTrim, KEY_UP, KEY_DOWN);
              else if(Model.X1Trim.trimState == TRIM_FLIGHT_MODE)
                fmd->x1Trim = adjustTrim(0, fmd->x1Trim, KEY_UP, KEY_DOWN);
              break;
              
            case 1:
              //y1 axis
              if(Model.Y1Trim.trimState == TRIM_COMMON)
                 Model.Y1Trim.commonTrim = adjustTrim(1, Model.Y1Trim.commonTrim, KEY_UP, KEY_DOWN);
              else if(Model.Y1Trim.trimState == TRIM_FLIGHT_MODE)
                fmd->y1Trim = adjustTrim(1, fmd->y1Trim, KEY_UP, KEY_DOWN);
              break;
            
            case 2:
              //x2 axis
              if(Model.X2Trim.trimState == TRIM_COMMON)
                Model.X2Trim.commonTrim = adjustTrim(2, Model.X2Trim.commonTrim, KEY_UP, KEY_DOWN);
              else if(Model.X2Trim.trimState == TRIM_FLIGHT_MODE)
                fmd->x2Trim = adjustTrim(2, fmd->x2Trim, KEY_UP, KEY_DOWN);
              break;
              
            case 3:
              //y2 axis
              if(Model.Y2Trim.trimState == TRIM_COMMON)
                 Model.Y2Trim.commonTrim = adjustTrim(3, Model.Y2Trim.commonTrim, KEY_UP, KEY_DOWN);
              else if(Model.Y2Trim.trimState == TRIM_FLIGHT_MODE)
                fmd->y2Trim = adjustTrim(3, fmd->y2Trim, KEY_UP, KEY_DOWN);
              break;
          }
        }
        else
        {
          if(heldButton == KEY_UP)
          {
            if(Sys.onscreenTrimEnabled)
            {
              isOnscreenTrimMode = true;
              killButtonEvents();
              audioToPlay = AUDIO_TRIM_MODE_ENTERED;
            }
          }

          if(clickedButton == KEY_SELECT)
          {
            if(mainMenuLocked) 
              changeToScreen(SCREEN_UNLOCK_MAIN_MENU);
            else 
              changeToScreen(SCREEN_MAIN_MENU);
          }

          if(clickedButton == KEY_UP)
            changeToScreen(SCREEN_CHANNEL_MONITOR);

          if(clickedButton == KEY_DOWN)
            changeToScreen(CONTEXT_MENU_HOME_SCREEN);
          
          //mute audible telemetry alarms
          if(heldButton == KEY_DOWN && millis() - buttonStartTime > 1000UL)
          {
            killButtonEvents();
            telemetryMuteAlarms = !telemetryMuteAlarms;
            audioToPlay = AUDIO_TELEM_MUTE_CHANGED;
            makeToast(telemetryMuteAlarms ? PSTR("Telemetry muted") : PSTR("Telemetry unmuted") , 2000, 0);
          }
        }
      }
      break;
      
    case SCREEN_CHANNEL_MONITOR:
      {
        // display.setInterlace(false); 
        
        drawHeader(PSTR("Outputs"));
        
        //scrollable 
        uint8_t numPages = (NUM_RC_CHANNELS + 9) / 10;
        static uint8_t thisPage = 1;
        
        static bool viewInitialised = false;
        if(!viewInitialised) 
        {
          if(lastScreen == CONTEXT_MENU_OUTPUTS) //start in the page that has the channel we want to view
            thisPage = (thisChIdx + 10) / 10;
          else
            thisPage = 1;
          viewInitialised = true;
        }

        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        uint8_t startIdx = (thisPage - 1) * 10;
        for(uint8_t i = startIdx; i < startIdx + 10 && i < NUM_RC_CHANNELS; i++)
        {
          if((i - startIdx) < 5)
            display.setCursor(0, 10 + (i - startIdx) * 11);
          else
            display.setCursor(66, 10 + (i - (startIdx + 5)) * 11);
          display.print(F("Ch"));
          display.print(1 + i);  
          if(lastScreen == CONTEXT_MENU_OUTPUTS && i == thisChIdx)
            display.write(0xB1);
          else
            display.print(F(":"));
          if(i < 9)
            display.print(F(" "));
          // display.print(channelOut[i] / 5);
          //show as decimal value
          int16_t val = channelOut[i];
          display.print(val / 5);
          val = abs(val);
          if(val < 500)
          {
            display.print(F("."));
            display.print((val % 5) * 2);
          }
        }
        //show scrollbar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          if(lastScreen == CONTEXT_MENU_OUTPUTS)
            changeToScreen(SCREEN_OUTPUTS);
          else
            changeToScreen(SCREEN_HOME);
        }
      }
      break;
      
    case CONTEXT_MENU_HOME_SCREEN:
      {
        enum {
          ITEM_SETUP_WIDGETS,
          ITEM_START_STOP_TIMER_FIRST,
          ITEM_START_STOP_TIMER_LAST = ITEM_START_STOP_TIMER_FIRST + NUM_TIMERS - 1,
          ITEM_RESET_TIMER_FIRST,
          ITEM_RESET_TIMER_LAST = ITEM_RESET_TIMER_FIRST + NUM_TIMERS - 1,
        };
        
        static char const startTimerStr[][20] PROGMEM = {"Start timer 1", "Start timer 2", "Start timer 3"};
        static char const stopTimerStr[][20] PROGMEM = {"Stop timer 1", "Stop timer 2", "Stop timer 3"};
        static char const resetTimerStr[][20] PROGMEM = {"Reset timer 1", "Reset timer 2", "Reset timer 3"};

        //dynamically add items
        contextMenuInitialise();
        for(uint8_t idx = 0; idx < NUM_TIMERS; idx++)
        {
          //only add a timer if the timer has a widget associated
          for(uint8_t i = 0; i < NUM_WIDGETS; i++)
          {
            if(Model.Widget[i].type == WIDGET_TYPE_TIMERS && Model.Widget[i].src == idx)
            {
              if(Model.Timer[idx].swtch == CTRL_SW_NONE)
              {
                if(timerIsRunning[idx]) 
                  contextMenuAddItem(stopTimerStr[idx], ITEM_START_STOP_TIMER_FIRST + idx);
                else 
                  contextMenuAddItem(startTimerStr[idx], ITEM_START_STOP_TIMER_FIRST + idx);
              }
              contextMenuAddItem(resetTimerStr[idx], ITEM_RESET_TIMER_FIRST + idx);
              break; 
            }
          }
        }
        if(!mainMenuLocked)        
          contextMenuAddItem(PSTR("Setup widgets"), ITEM_SETUP_WIDGETS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID >= ITEM_START_STOP_TIMER_FIRST && contextMenuSelectedItemID <= ITEM_START_STOP_TIMER_LAST)
        {
          uint8_t idx = contextMenuSelectedItemID - ITEM_START_STOP_TIMER_FIRST;
          timerForceRun[idx] = timerIsRunning[idx] ? false : true;
          changeToScreen(SCREEN_HOME);
        }
        if(contextMenuSelectedItemID >= ITEM_RESET_TIMER_FIRST && contextMenuSelectedItemID <= ITEM_RESET_TIMER_LAST)
        {
          uint8_t idx = contextMenuSelectedItemID - ITEM_RESET_TIMER_FIRST;
          resetTimerRegister(idx);
          changeToScreen(SCREEN_HOME);
        }
        if(contextMenuSelectedItemID == ITEM_SETUP_WIDGETS)
          changeToScreen(SCREEN_WIDGET_SETUP);
          
        if(heldButton == KEY_SELECT || contextMenuGetItemCount() == 0) //exit
          changeToScreen(SCREEN_HOME);
      }
      break;

    case SCREEN_WIDGET_SETUP:
      {
        drawHeader(PSTR("Widgets"));
        
        widget_params_t *widget = &Model.Widget[thisWidgetIdx];
        
        display.setCursor(8, 9);
        display.print(F("Widget"));
        display.print(thisWidgetIdx + 1);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);

        //dynamic list
        enum {
          ITEM_TYPE,
          ITEM_SRC,
          ITEM_DISP,
          ITEM_GAUGE_MIN,
          ITEM_GAUGE_MAX,

          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT];
        uint8_t listItemCount = 0;

        //add items to list
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(widget->type == WIDGET_TYPE_TIMERS 
            || widget->type == WIDGET_TYPE_COUNTERS
            || (widget->type == WIDGET_TYPE_TELEMETRY && widget->src == WIDGET_SRC_AUTO))
          {
            if(i == ITEM_DISP || i == ITEM_GAUGE_MIN || i == ITEM_GAUGE_MAX)
              continue;
          }
          if(widget->disp == WIDGET_DISP_NUMERICAL)
          {
            if(i == ITEM_GAUGE_MIN || i == ITEM_GAUGE_MAX)
              continue;
          }
          listItemIDs[listItemCount++] = i;
        }

        //handle navigation
        uint8_t numFocusable = listItemCount + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }

        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(40, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_TYPE:
              {
                display.print(F("Type:"));
                display.setCursor(48, ypos);
                display.print(findStringInIdStr(enum_WidgetType, widget->type));
                if(edit)
                {
                  uint8_t prevType = widget->type;
                  widget->type = incDec(widget->type, 0, WIDGET_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  if(widget->type != prevType)
                  {
                    widget->disp = WIDGET_DISP_NUMERICAL;
                    widget->gaugeMin = -100;
                    widget->gaugeMax = 100;
                    if(widget->type == WIDGET_TYPE_TELEMETRY)
                    {
                      widget->src = WIDGET_SRC_AUTO;
                      widget->gaugeMin = 0;
                    }
                    if(widget->type == WIDGET_TYPE_MIXSOURCES)
                      widget->src = SRC_NONE;
                    if(widget->type == WIDGET_TYPE_OUTPUTS 
                      || widget->type == WIDGET_TYPE_COUNTERS 
                      || widget->type == WIDGET_TYPE_TIMERS)
                    {
                      widget->src = 0;
                    }
                  }
                }
              }
              break;
            
            case ITEM_SRC:
              {
                display.print(F("Src:"));
                display.setCursor(48, ypos);
                if(widget->type == WIDGET_TYPE_TELEMETRY)
                {
                  if(widget->src == WIDGET_SRC_AUTO) 
                    display.print(findStringInIdStr(enum_WidgetSource, WIDGET_SRC_AUTO));
                  else 
                    display.print(Model.Telemetry[widget->src].name);
                  if(edit)
                  {
                    //skip empty telemetry (non-extant sensors)
                    //skip special telemetry
                    //use an array to hold extant sensors
                    uint8_t srcQQ[NUM_CUSTOM_TELEMETRY + 1]; //1 added for auto
                    uint8_t srcCount = 0;
                    for(uint8_t i = 0; i < sizeof(srcQQ) - 1; i++)
                    {
                      if(isEmptyStr(Model.Telemetry[i].name, sizeof(Model.Telemetry[0].name)))
                        continue;
                      if(Model.Telemetry[i].type == TELEMETRY_TYPE_GNSS)
                        continue;
                      srcQQ[srcCount] = i;
                      srcCount++;
                    }
                    srcQQ[srcCount] = WIDGET_SRC_AUTO;
                    srcCount++;
        
                    uint8_t idxQQ = 0;
                    for(uint8_t i = 0; i < srcCount; i++) //search for a match
                    {
                      if(srcQQ[i] == widget->src)
                      {
                        idxQQ = i;
                        break;
                      }
                    }
                    idxQQ = incDec(idxQQ, 0, srcCount - 1, INCDEC_WRAP, INCDEC_SLOW);
                    uint8_t prevSrc = widget->src;
                    widget->src = srcQQ[idxQQ];
                    if(widget->src != prevSrc)
                    {
                      widget->gaugeMin = 0;
                      widget->gaugeMax = 100;
                    }
                  }
                }
                else if(widget->type == WIDGET_TYPE_OUTPUTS)
                {
                  display.print(F("Ch"));
                  display.print(widget->src + 1);
                  if(!isEmptyStr(Model.Channel[widget->src].name, sizeof(Model.Channel[0].name)))
                  {
                    display.print(F(" "));
                    display.print(Model.Channel[widget->src].name);
                  }
                  if(edit)
                    widget->src = incDec(widget->src, 0, NUM_RC_CHANNELS - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
                else if(widget->type == WIDGET_TYPE_MIXSOURCES)
                {
                  getSrcName(textBuff, widget->src, sizeof(textBuff));
                  display.print(textBuff);
                  if(edit)
                  {
                    //detect moved source
                    uint8_t movedSrc = procMovedSource(getMovedSource());
                    if(movedSrc != SRC_NONE)
                      widget->src = movedSrc;
                    //inc dec
                    widget->src = incDecSource(widget->src, INCDEC_FLAG_MIX_SRC);
                  }
                }
                else if(widget->type == WIDGET_TYPE_COUNTERS)
                {
                  if(!isEmptyStr(Model.Counter[widget->src].name, sizeof(Model.Counter[0].name)))
                    display.print(Model.Counter[widget->src].name);
                  else
                  {
                    display.print(F("Counter"));
                    display.print(widget->src + 1);
                  }
                  if(edit)
                    widget->src = incDec(widget->src, 0, NUM_COUNTERS - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
                else if(widget->type == WIDGET_TYPE_TIMERS)
                {
                  display.print(F("Timer"));
                  display.print(widget->src + 1);
                  if(!isEmptyStr(Model.Timer[widget->src].name, sizeof(Model.Timer[0].name)))
                  {
                    display.print(F(" "));
                    display.print(Model.Timer[widget->src].name);
                  }
                  if(edit)
                    widget->src = incDec(widget->src, 0, NUM_TIMERS - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
              }
              break;
            
            case ITEM_DISP:
              {
                display.print(F("Disp:"));
                display.setCursor(48, ypos);
                display.print(findStringInIdStr(enum_WidgetDisplay, widget->disp));
                if(edit)
                {
                  uint8_t prevDisp =  widget->disp;
                  do {
                    widget->disp = incDec(widget->disp, 0, WIDGET_DISP_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  } while(widget->type == WIDGET_TYPE_TELEMETRY && widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED);
                  
                  if(widget->disp != prevDisp)
                  {
                    if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
                    {
                      if(widget->gaugeMin > 0)
                        widget->gaugeMin = 0 - widget->gaugeMin;
                      widget->gaugeMax = 0 - widget->gaugeMin;
                    }
                  }
                }
              }
              break;
            
            case ITEM_GAUGE_MIN:
              {
                display.print((widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED) ? F("Left:") : F("Min:"));
                display.setCursor(48, ypos);
                if(widget->type == WIDGET_TYPE_TELEMETRY)
                {
                  display.setCursor(display.getCursorX(), display.getCursorY());
                  printTelemParam(widget->src, widget->gaugeMin, true);
                }
                else if(widget->type == WIDGET_TYPE_OUTPUTS || widget->type == WIDGET_TYPE_MIXSOURCES)
                  display.print(widget->gaugeMin);
                if(edit)
                {
                  if(widget->type == WIDGET_TYPE_TELEMETRY)
                    widget->gaugeMin = incDec(widget->gaugeMin, -30000, widget->gaugeMax, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                  else if(widget->type == WIDGET_TYPE_MIXSOURCES || widget->type == WIDGET_TYPE_OUTPUTS)
                    widget->gaugeMin = incDec(widget->gaugeMin, -100, widget->gaugeMax, INCDEC_NOWRAP, INCDEC_NORMAL);
                  if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
                    widget->gaugeMax = 0 - widget->gaugeMin;
                }
              }
              break;
            
            case ITEM_GAUGE_MAX:
              {
                display.print((widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED) ? F("Right:") : F("Max:"));
                display.setCursor(48, ypos);
                if(widget->type == WIDGET_TYPE_TELEMETRY)
                {
                  display.setCursor(display.getCursorX(), display.getCursorY());
                  printTelemParam(widget->src, widget->gaugeMax, true);
                }
                else if(widget->type == WIDGET_TYPE_OUTPUTS || widget->type == WIDGET_TYPE_MIXSOURCES)
                  display.print(widget->gaugeMax);
                if(edit)
                {
                  if(widget->type == WIDGET_TYPE_TELEMETRY)
                    widget->gaugeMax = incDec(widget->gaugeMax, widget->gaugeMin, 30000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                  else if(widget->type == WIDGET_TYPE_MIXSOURCES || widget->type == WIDGET_TYPE_OUTPUTS)
                    widget->gaugeMax = incDec(widget->gaugeMax, widget->gaugeMin, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
                  if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
                    widget->gaugeMin = 0 - widget->gaugeMax;
                }
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);

        //open context menu
        if(focusedItem == numFocusable && isEditMode)
          changeToScreen(CONTEXT_MENU_WIDGETS);
        
        //change to next widget
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisWidgetIdx = incDec(thisWidgetIdx, 0, NUM_WIDGETS - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_HOME);
      }
      break;
      
    case CONTEXT_MENU_WIDGETS:
      {
        enum {
          ITEM_COPY_WIDGET,
          ITEM_MOVE_WIDGET,
          ITEM_RESET_WIDGET,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_WIDGET);
        contextMenuAddItem(PSTR("Move to"), ITEM_MOVE_WIDGET);
        contextMenuAddItem(PSTR("Reset widget"), ITEM_RESET_WIDGET);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RESET_WIDGET)
        {
          resetWidgetParams(thisWidgetIdx);
          changeToScreen(SCREEN_WIDGET_SETUP);
        }
        if(contextMenuSelectedItemID == ITEM_COPY_WIDGET)
        {
          destWidgetIdx = thisWidgetIdx;
          changeToScreen(DIALOG_COPY_WIDGET);
        }
        if(contextMenuSelectedItemID == ITEM_MOVE_WIDGET)
        {
          destWidgetIdx = thisWidgetIdx;
          changeToScreen(DIALOG_MOVE_WIDGET);
        }
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_WIDGET_SETUP);
      }
      break;
      
    case DIALOG_COPY_WIDGET:
    case DIALOG_MOVE_WIDGET:
      {
        isEditMode = true;
        destWidgetIdx = incDec(destWidgetIdx, 0, NUM_WIDGETS - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Widget"), thisWidgetIdx, destWidgetIdx, theScreen == DIALOG_COPY_WIDGET);
        if(clickedButton == KEY_SELECT)
        {
          if(theScreen == DIALOG_COPY_WIDGET)
            Model.Widget[destWidgetIdx] = Model.Widget[thisWidgetIdx];
          else
          {
            uint8_t newPos = destWidgetIdx;
            uint8_t oldPos = thisWidgetIdx;
            //copy from old position to temp
            widget_params_t Temp_Widget = Model.Widget[oldPos];
            //shift elements
            uint8_t thisPos = oldPos;
            if(newPos < oldPos)
            {
              while(thisPos > newPos)
              {
                Model.Widget[thisPos] = Model.Widget[thisPos - 1];
                thisPos--;
              }
            }
            else if(newPos > oldPos)
            {
              while(thisPos < newPos)
              {
                Model.Widget[thisPos] = Model.Widget[thisPos + 1];
                thisPos++;
              }
            }
            //copy from temp into new position
            Model.Widget[newPos] = Temp_Widget;
          }
          thisWidgetIdx = destWidgetIdx;
          changeToScreen(SCREEN_WIDGET_SETUP);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_WIDGET_SETUP);
      }
      break;
      
    ////////////////////////////////// MAIN MENU ///////////////////////////////////////////////////
    
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
      
    ////////////////////////////////// MODEL ///////////////////////////////////////////////////////
      
    case SCREEN_UNLOCK_MODEL_MANAGER:
      {
        validatePassword(SCREEN_MODEL, SCREEN_MAIN_MENU);
      }
      break;
      
    case SCREEN_MODEL:
      {
        drawHeader(mainMenu[MAIN_MENU_MODEL]);
        
        //------ scrollable list of models ----------
        const uint8_t numVisible = 6;
        
        static uint8_t topItem = 1;
        static bool viewInitialised = false;
        static bool modelListNeedsUpdate = true;
        if(!viewInitialised)
        {
          viewInitialised = true;
          modelListNeedsUpdate = true;
          thisModelIdx = Sys.activeModelIdx;
        }
        
        // handle navigation
        focusedItem = thisModelIdx + 1;
        changeFocusOnUpDown(maxNumOfModels);
        uint8_t prevTopItem = topItem;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + numVisible)
          topItem++;
        if(topItem != prevTopItem)
          modelListNeedsUpdate = true;

        // fill list
        //Check at once and only update on scroll or just entered this screen so we 
        //don't have to constantly read all names from eeprom (a somewhat slow process)
        static char mdlStr[numVisible][sizeof(Model.name)]; //string table
        if(modelListNeedsUpdate)
        {
          modelListNeedsUpdate = false;
          if(maxNumOfModels > 1)
          {
            for(uint8_t i = 0; i < numVisible && i < maxNumOfModels; i++)
            {
              uint8_t modelIdx = topItem - 1 + i;
              if(eeModelIsFree(modelIdx)) // write 0xFF to indicate it is free
                mdlStr[i][0] = 0xFF;
              else //get the name
              {
                eeGetModelName(textBuff, modelIdx, sizeof(textBuff));
                strlcpy(&mdlStr[i][0], textBuff, sizeof(Model.name));
              }
            }
          }
          else
          {
            strlcpy(&mdlStr[0][0], Model.name, sizeof(Model.name));
          }
        }
        
        for(uint8_t line = 0; line < numVisible && line < maxNumOfModels; line++)
        {
          uint8_t ypos = 10 + line * 9;
          uint8_t item = topItem + line;
          if(focusedItem == topItem + line) //highlight
          {
            display.fillRect(0, ypos - 1, item < 10 ? 7 : 13, 9, BLACK);
            display.setTextColor(WHITE);
          }
          display.setCursor(1, ypos);
          display.print(item);
          display.setTextColor(BLACK);
          
          uint8_t modelIdx = item - 1;
          display.setCursor(16, ypos);
          if((uint8_t)mdlStr[line][0] != 0xFF)
            printModelName(mdlStr[line], modelIdx);
          if(modelIdx == Sys.activeModelIdx) //indicate it is active
            display.drawBitmap(display.getCursorX() + 6, ypos + 1, icon_checkmark_bold, 7, 6, BLACK);
        }
        
        //scroll bar
        drawScrollBar(127, 9, maxNumOfModels, topItem, numVisible, numVisible * 9);

        //----- end of list ----------------------
        
        if(focusedItem <= maxNumOfModels)
          thisModelIdx = focusedItem - 1;
        
        if(clickedButton == KEY_SELECT)
        {
          if(thisModelIdx == Sys.activeModelIdx)
            changeToScreen(CONTEXT_MENU_ACTIVE_MODEL);
          else if(!eeModelIsFree(thisModelIdx))
            changeToScreen(CONTEXT_MENU_INACTIVE_MODEL);
          else if(eeModelIsFree(thisModelIdx))
            changeToScreen(CONTEXT_MENU_FREE_MODEL);
          //force the list to be updated
          modelListNeedsUpdate = true; 
        }

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;//reset view
          changeToScreen(SCREEN_MAIN_MENU);
        }
      }
      break;
      
    case CONTEXT_MENU_ACTIVE_MODEL:
      {
        enum {
          ITEM_RENAME_MODEL,
          ITEM_DUPLICATE_MODEL,
          ITEM_COPY_FROM,
          ITEM_RESET_MODEL,
          ITEM_BACKUP_MODEL,
          ITEM_RESTORE_MODEL,
          ITEM_NEW_MODEL,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Rename model"), ITEM_RENAME_MODEL);
        if(maxNumOfModels > 1)
        {
          contextMenuAddItem(PSTR("Duplicate"), ITEM_DUPLICATE_MODEL);
          contextMenuAddItem(PSTR("Copy from"), ITEM_COPY_FROM);
        }
        contextMenuAddItem(PSTR("Reset model"), ITEM_RESET_MODEL);
        if(maxNumOfModels == 1)
          contextMenuAddItem(PSTR("New model"), ITEM_NEW_MODEL);
        if(sdHasCard())
        {
          contextMenuAddItem(PSTR("Backup to SD"), ITEM_BACKUP_MODEL);
          contextMenuAddItem(PSTR("Restore from SD"), ITEM_RESTORE_MODEL);
        }
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RENAME_MODEL) 
          changeToScreen(DIALOG_RENAME_MODEL);
        if(contextMenuSelectedItemID == ITEM_DUPLICATE_MODEL)
        {          
          //Find the nearest free slot and copy to it
          uint8_t nearestLeft = 0xff;
          uint8_t nearestRight = 0xff;
          for(uint8_t i = thisModelIdx + 1; i < maxNumOfModels; i++) //search in right direction
          {
            if(eeModelIsFree(i)) 
            {
              nearestRight = i; 
              break;
            }
          }
          for(int8_t i = thisModelIdx - 1; i >= 0; i--) //search in left direction
          {
            if(eeModelIsFree(i))
            {
              nearestLeft = i; 
              break;
            }
          }
          if(nearestLeft == 0xff && nearestRight == 0xff) //no free slots
          {
            makeToast(PSTR("No free slots"), 2000, 0);
            changeToScreen(SCREEN_MODEL);
          }
          else
          {
            uint8_t destination;
            uint8_t distRight = (nearestRight == 0xff) ? 0xff : nearestRight - thisModelIdx;
            uint8_t distLeft = (nearestLeft == 0xff) ? 0xff : thisModelIdx - nearestLeft;
            if(distRight == 0xff)
              destination = nearestLeft;
            else if(distLeft == 0xff)
              destination = nearestRight;
            else
            {
              destination = (distRight <= distLeft) ? nearestRight : nearestLeft;
            }
            //--- copy
            showWaitMessage();
            stopTones();
            //save the active model
            eeSaveModelData(Sys.activeModelIdx);
            //save to this slot and set it active
            eeSaveModelData(destination);
            Sys.activeModelIdx = destination;
            thisModelIdx = destination;
            //exit
            changeToScreen(SCREEN_MODEL);
          }
        }
        if(contextMenuSelectedItemID == ITEM_COPY_FROM)    
          changeToScreen(DIALOG_COPYFROM_MODEL);
        if(contextMenuSelectedItemID == ITEM_RESET_MODEL)  
          changeToScreen(CONFIRMATION_MODEL_RESET);
        if(contextMenuSelectedItemID == ITEM_BACKUP_MODEL)
          changeToScreen(CONFIRMATION_MODEL_BACKUP);
        if(contextMenuSelectedItemID == ITEM_RESTORE_MODEL)
          changeToScreen(DIALOG_RESTORE_MODEL);
        if(contextMenuSelectedItemID == ITEM_NEW_MODEL)
          changeToScreen(CONFIRMATION_CREATE_MODEL);
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONTEXT_MENU_INACTIVE_MODEL:
      {
        enum {
          ITEM_LOAD_MODEL,
          ITEM_DELETE_MODEL,
          ITEM_BACKUP_MODEL,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Load model"), ITEM_LOAD_MODEL);
        contextMenuAddItem(PSTR("Delete model"), ITEM_DELETE_MODEL);
        if(sdHasCard())
          contextMenuAddItem(PSTR("Backup to SD"), ITEM_BACKUP_MODEL);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_LOAD_MODEL) 
        {
          showWaitMessage();
          stopTones();
          //Save the active model before changing to another model
          eeSaveModelData(Sys.activeModelIdx);
          //load model and set it active
          eeReadModelData(thisModelIdx);
          Sys.activeModelIdx = thisModelIdx; 
          //Safety checks
          handleSafetyWarnUI();
          //reinitialise other stuff
          resetTimerRegisters();
          resetCounterRegisters();
          Sys.rfEnabled = false;
          reinitialiseMixerCalculations();
          restoreTimerRegisters();
          restoreCounterRegisters();
          //exit
          changeToScreen(SCREEN_MODEL);
          resetPages();
        }
        if(contextMenuSelectedItemID == ITEM_DELETE_MODEL)
          changeToScreen(CONFIRMATION_MODEL_DELETE);
        if(contextMenuSelectedItemID == ITEM_BACKUP_MODEL)
          changeToScreen(CONFIRMATION_MODEL_BACKUP);
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONTEXT_MENU_FREE_MODEL:
      {
        enum {
          ITEM_NEW_MODEL,
          ITEM_RESTORE_MODEL,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("New model"), ITEM_NEW_MODEL);
        if(sdHasCard())
          contextMenuAddItem(PSTR("Restore from SD"), ITEM_RESTORE_MODEL);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_NEW_MODEL)
          changeToScreen(DIALOG_MODEL_TYPE);
        if(contextMenuSelectedItemID == ITEM_RESTORE_MODEL)
          changeToScreen(DIALOG_RESTORE_MODEL);

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONFIRMATION_CREATE_MODEL:
      {
        printFullScreenMessage(PSTR("Model data will\nbe overwritten.\nContinue?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
          changeToScreen(DIALOG_MODEL_TYPE);
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case DIALOG_MODEL_TYPE:
      {
        isEditMode = true;
        
        static uint8_t mdlType;
        static bool initialised = false;
        if(!initialised)
        {
          initialised = true;
          mdlType = Model.type;
        }
        
        mdlType = incDec(mdlType, 0, MODEL_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);

        drawBoundingBox(11, 14, 105, 35,BLACK);
        drawCursor(21, 28);
        display.setCursor(17, 18);
        display.print(F("Model type"));
        display.setCursor(29, 28);
        display.print(findStringInIdStr(enum_ModelType, mdlType));
        
        if(clickedButton == KEY_SELECT) //create the model
        {
          showWaitMessage();
          stopTones();
          //Save the current active model first
          if(maxNumOfModels > 1)
            eeSaveModelData(Sys.activeModelIdx);
          //reinitialise other stuff
          resetTimerRegisters();
          resetCounterRegisters();
          Sys.rfEnabled = false;
          reinitialiseMixerCalculations();
          //create model and set it active
          if(maxNumOfModels > 1)
          {
            eeCreateModel(thisModelIdx);
            Sys.activeModelIdx = thisModelIdx;
          }
          else
          {
            resetModelName();
            resetModelParams();
          }
          //write model type
          Model.type = mdlType;
          //load default mixer template
          if(mdlType == MODEL_TYPE_AIRPLANE || mdlType == MODEL_TYPE_MULTICOPTER)
          {
            if(Sys.mixerTemplatesEnabled)
              loadMixerTemplateBasic(0);
          }
          //save
          if(maxNumOfModels > 1)
            eeSaveModelData(Sys.activeModelIdx);
          // changeToScreen(SCREEN_MODEL);
          changeToScreen(DIALOG_RENAME_MODEL);
          resetPages();
        }
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case DIALOG_RENAME_MODEL:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Model name"), Model.name, sizeof(Model.name), true, true, false);
        if(!isEditTextDialog) //exited
        {
          showWaitMessage();
          stopTones();
          if(maxNumOfModels > 1)
            eeSaveModelData(Sys.activeModelIdx);
          changeToScreen(SCREEN_MODEL); 
        }
      }
      break;
      
    case DIALOG_COPYFROM_MODEL:
      {
        //change source model
        isEditMode = true;
        do {
          thisModelIdx = incDec(thisModelIdx, 0, maxNumOfModels - 1 , INCDEC_WRAP, INCDEC_SLOW);
        } while(eeModelIsFree(thisModelIdx) || Model.type != eeGetModelType(thisModelIdx));
        
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Copy from"));
        display.setCursor(29, 28);
        eeGetModelName(textBuff, thisModelIdx, sizeof(textBuff)); 
        printModelName(textBuff, thisModelIdx);
        drawCursor(21, 28);

        if(clickedButton == KEY_SELECT)
        {
          if(thisModelIdx == Sys.activeModelIdx)
            changeToScreen(SCREEN_MODEL);
          else
            changeToScreen(CONFIRMATION_MODEL_COPY);
        }
        else if(heldButton == KEY_SELECT) //exit
        {
          thisModelIdx = Sys.activeModelIdx;
          changeToScreen(SCREEN_MODEL);
        }
      }
      break;
      
    case DIALOG_RESTORE_MODEL:
      {
        static bool initialised = false;
        static uint16_t mdlCount = 0;
        static uint16_t thisPos = 0; //Better name
        static char nameStr[13]; //buffer to hold name. Only 8.3 names supported
        
        if(!initialised)
        {
          initialised = true;
          //show wait message as getting the count may take some time when there are many models
          showWaitMessage();
          stopTones();
          //find how many models we have backed up on the sd card
          mdlCount = sdGetModelCount();
          thisPos = 0;
          sdGetModelName(nameStr, thisPos, sizeof(nameStr));
        }
        
        if(mdlCount == 0) //no backups exist on the sd card
        {
          makeToast(PSTR("No models on SD"), 2000, 0);
          initialised = false;
          changeToScreen(SCREEN_MODEL);
          break;
        }
          
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Restore from SD"));
        display.setCursor(29, 28);
        display.print(nameStr);
        drawCursor(21, 28);
        
        //get the name
        isEditMode = true;
        uint8_t lastPos = thisPos;
        thisPos = incDec(thisPos, 0, mdlCount - 1, INCDEC_NOWRAP, INCDEC_SLOW); //strictly No Wrap here
        if(thisPos != lastPos) //changed, refresh
          sdGetModelName(nameStr, thisPos, sizeof(nameStr));
        
        if(clickedButton == KEY_SELECT)
        {
          initialised = false;
          //Copy name into textBuff. 
          //Assumption is textBuff won't be modified by something else. This isn't much of an issue 
          //since we can always retry if the restore fails.         
          strlcpy(textBuff, nameStr, sizeof(textBuff));
          changeToScreen(CONFIRMATION_MODEL_RESTORE);
        }

        if(heldButton == KEY_SELECT) //exit
        {
          initialised = false;
          changeToScreen(SCREEN_MODEL);
        }
      }
      break;
      
    case CONFIRMATION_MODEL_RESTORE:
      {
        if(thisModelIdx == Sys.activeModelIdx)
          printFullScreenMessage(PSTR("Model data will be\noverwritten.\nContinue?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP || thisModelIdx != Sys.activeModelIdx)
        {
          showWaitMessage();
          stopTones();
          //save active model first but only if we aren't restoring to active slot
          //otherwise there is no point in saving first
          if(maxNumOfModels > 1 && thisModelIdx != Sys.activeModelIdx)
            eeSaveModelData(Sys.activeModelIdx);
          //restore the model
          if(sdRestoreModel(textBuff))
          {
            //save it to eeprom
            if(maxNumOfModels > 1)
              eeSaveModelData(thisModelIdx);
            //if we have restored into the active model slot, reinitialise some items
            if(thisModelIdx == Sys.activeModelIdx)
            {
              //safety checks
              handleSafetyWarnUI();
              //reinitialise other stuff
              resetTimerRegisters();
              resetCounterRegisters();
              Sys.rfEnabled = false;
              reinitialiseMixerCalculations();
              restoreTimerRegisters();
              restoreCounterRegisters();
              resetPages();
            }
            else
            {
              //read back the active model from eeprom
              if(maxNumOfModels > 1)
                eeReadModelData(Sys.activeModelIdx);
            }
            //exit
            changeToScreen(SCREEN_MODEL);
          }
          else
          {
            //exit
            makeToast(PSTR("Restore failed"), 2000, 0);
            changeToScreen(SCREEN_MODEL);
          }
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONFIRMATION_MODEL_BACKUP:
      {
        static bool initialised = false;
        static bool modelBackupExists = false;
        static char nameStr[13]; //buffer to hold model name. Only 8.3 names supported
        
        if(!initialised)
        {
          initialised = true;
          modelBackupExists = false;

          //get the model name into the nameStr buffer
          if(maxNumOfModels > 1)
            eeGetModelName(nameStr, thisModelIdx, sizeof(nameStr));
          else
            strlcpy(nameStr, Model.name, sizeof(nameStr));
          
          bool nameEmpty = false;
          if(isEmptyStr(nameStr, sizeof(nameStr))) 
            nameEmpty = true;
          else
          {
            //Force conforming to 8.3 naming rules. Basic checks only
            
            // Strip leading periods
            uint8_t k = 0;
            while(nameStr[k] == '.') 
            {
              if(!isalnum(nameStr[k]) && nameStr[k] != '_')                  
                memmove(&nameStr[k], &nameStr[k + 1], strlen(nameStr) - k);
              else
                k++;
            }
            
            // Convert name to uppercase and replace invalid characters with '_'
            // Also limit the base name to 8 characters max
            uint8_t len = strlen(nameStr);
            uint8_t i = 0;
            char basename[9];
            for(i = 0; i < len; i++) 
            {
              if(i == 8) 
                break;
              if(nameStr[i] == ' ' || nameStr[i] == '/') 
                basename[i] = '_';
              else 
                basename[i] = toupper(nameStr[i]);
            }
            basename[i] = '\0';
            
            //check again if empty
            if(i == 0) 
              nameEmpty = true;
            else if(basename[i - 1] == '.') 
            {
              // If the basename ends with a period, remove it
              i--;
              basename[i] = '\0';
            }
            
            //copy back to nameStr
            strlcpy(nameStr, basename, sizeof(nameStr));
          }
          
          if(nameEmpty) //make a generic name
          {
            strlcpy_P(nameStr, PSTR("MODEL"), sizeof(nameStr));
            char suffix[5];
            memset(suffix, 0, sizeof(suffix));
            itoa(thisModelIdx + 1, suffix, 10);
            strlcat(nameStr, suffix, sizeof(nameStr));
          }
          
          //append file extension
          // strlcat(nameStr, ".MDL", sizeof(nameStr)); //is this really necessary??
          
          //show wait message as checking may take some time when there are many models
          showWaitMessage();
          stopTones();
          
          //check if the name already exists
          if(sdSimilarModelExists(nameStr))
            modelBackupExists = true;
        }
        
        if(modelBackupExists)
          printFullScreenMessage(PSTR("A backup with\na similar name\nexists.\nOverwrite it?\n\nYes [Up] \nNo [Down]"));
        
        if(clickedButton == KEY_UP || !modelBackupExists)
        {
          showWaitMessage();
          stopTones();
          
          if(maxNumOfModels > 1)
          {
            //save the active model first
            eeSaveModelData(Sys.activeModelIdx);
            //read into ram the model we want to back up to sd card
            eeReadModelData(thisModelIdx);
          }
          
          //back up to the sd card
          if(!sdBackupModel(nameStr))
            makeToast(PSTR("Backup failed"), 2000, 0);
          
          if(maxNumOfModels > 1)
          {
            //read back the active model from eeprom
            eeReadModelData(Sys.activeModelIdx);
          }

          //exit
          initialised = false;
          changeToScreen(SCREEN_MODEL);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
        {
          initialised = false;
          changeToScreen(SCREEN_MODEL);
        }
      }
      break;
      
    case CONFIRMATION_MODEL_COPY:
      {
        printFullScreenMessage(PSTR("Model data will\nbe overwritten.\nContinue?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          showWaitMessage();
          stopTones();
          //temporarily store model name as we shall maintain it 
          strlcpy(textBuff, Model.name, sizeof(textBuff));
          //load source model into ram
          eeReadModelData(thisModelIdx);
          //restore the model name
          strlcpy(Model.name, textBuff, sizeof(Model.name));
          //save
          eeSaveModelData(Sys.activeModelIdx);
          //reinitiailise other stuff
          resetTimerRegisters();
          resetCounterRegisters();
          Sys.rfEnabled = false;
          reinitialiseMixerCalculations();
          thisModelIdx = Sys.activeModelIdx; 
          //exit
          changeToScreen(SCREEN_MODEL);
          resetPages();
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
        {
          thisModelIdx = Sys.activeModelIdx;
          changeToScreen(SCREEN_MODEL);
        }
      }
      break;
      
    case CONFIRMATION_MODEL_RESET:
      {
        printFullScreenMessage(PSTR("Model data will\nbe reset.\nContinue?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          showWaitMessage();
          stopTones();
          //temporarily store the model type for later restoring
          uint8_t mdlType = Model.type;
          //reset
          resetModelParams();
          //load the default mixer template
          if(mdlType == MODEL_TYPE_AIRPLANE || mdlType == MODEL_TYPE_MULTICOPTER)
          {
            if(Sys.mixerTemplatesEnabled)
              loadMixerTemplateBasic(0);
          }
          //restore the model type
          Model.type = mdlType;
          //save to eeprom
          if(maxNumOfModels > 1)
            eeSaveModelData(Sys.activeModelIdx);
          //reinitialise other stuff
          resetTimerRegisters();
          resetCounterRegisters();
          Sys.rfEnabled = false;
          reinitialiseMixerCalculations();
          //exit
          changeToScreen(SCREEN_MODEL);
          resetPages();
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONFIRMATION_MODEL_DELETE:
      {
        printFullScreenMessage(PSTR("Delete model?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          showWaitMessage();
          stopTones();
          eeDeleteModel(thisModelIdx);
          changeToScreen(SCREEN_MODEL);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MODEL);
      }
      break;
    
    ////////////////////////////////// INPUTS //////////////////////////////////////////////////////
    
    case SCREEN_INPUTS:
      {
        drawHeader(mainMenu[MAIN_MENU_INPUTS]);

        enum { 
          PAGE_AIL_CURVE, 
          PAGE_ELE_CURVE, 
          PAGE_RUD_CURVE, 
          PAGE_THR_CURVE,
          PAGE_STICKS,
          PAGE_KNOBS,
          PAGE_SWITCHES,
          
          NUM_PAGES
        };
        
        static uint8_t page;
        
        //check if the page is actually available to prevent getting stuck in an infinite loop
        if(Model.type == MODEL_TYPE_OTHER 
          && (page == PAGE_RUD_CURVE || page == PAGE_AIL_CURVE || page == PAGE_ELE_CURVE || page == PAGE_THR_CURVE))
        {
          page = PAGE_STICKS;
        }
        
        toggleEditModeOnSelectClicked();
        if(focusedItem == 1)
        {
          uint8_t lastPage = page;

          if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
            page = incDec(page, 0, NUM_PAGES - 1, INCDEC_WRAP, INCDEC_SLOW);
          else if(Model.type == MODEL_TYPE_OTHER) 
          {
            //hide rud, ail, ele, thr curves
            do {
              page = incDec(page, 0, NUM_PAGES - 1, INCDEC_WRAP, INCDEC_SLOW);
            } while(page == PAGE_RUD_CURVE || page == PAGE_AIL_CURVE || page == PAGE_ELE_CURVE || page == PAGE_THR_CURVE);
          }

          if(page != lastPage)
            graphYCoordinatesInvalid = true;

          //Show cursor
          drawCursor(0, 9);
        }
          
        //------ RUD, AIL, ELE CURVES 
        if(page == PAGE_RUD_CURVE || page == PAGE_AIL_CURVE || page == PAGE_ELE_CURVE)  
        {  
          changeFocusOnUpDown(5);
          
          uint8_t qqPage[3] = {PAGE_RUD_CURVE, PAGE_AIL_CURVE, PAGE_ELE_CURVE};
          uint8_t qqSrc[3] = {SRC_RUD, SRC_AIL, SRC_ELE};
          
          rate_expo_t* qqRateExpo[3];
          qqRateExpo[0] = &Model.RudDualRate;
          qqRateExpo[1] = &Model.AilDualRate;
          qqRateExpo[2] = &Model.EleDualRate;
          
          uint8_t* qqSrcRaw[3];
          qqSrcRaw[0] = &Model.rudSrcRaw;
          qqSrcRaw[1] = &Model.ailSrcRaw;
          qqSrcRaw[2] = &Model.eleSrcRaw;
          
          int16_t qqValIn[3];
          qqValIn[0] = mixSources[Model.rudSrcRaw];
          qqValIn[1] = mixSources[Model.ailSrcRaw];
          qqValIn[2] = mixSources[Model.eleSrcRaw];
          
          uint8_t idx = 0;
          for(uint8_t i = 0; i < 3; i++) //find the idx
          {
            if(qqPage[i] == page)
            {
              idx = i;
              break;
            }
          }
          
          int8_t  *rate = &(qqRateExpo[idx]->rate1);
          int8_t  *expo = &(qqRateExpo[idx]->expo1);
          uint8_t *swtch = &(qqRateExpo[idx]->swtch);
          if(*swtch != CTRL_SW_NONE && checkSwitchCondition(*swtch))
          {
            rate = &(qqRateExpo[idx]->rate2);
            expo = &(qqRateExpo[idx]->expo2);
          }
          
          //--- Edit values
          if(focusedItem == 2 && isEditMode)
          {
            //auto detect moved
            uint8_t movedSrc = getMovedSource();
            if(movedSrc != SRC_NONE)
            {
              if(movedSrc >= SRC_RAW_ANALOG_FIRST && movedSrc <= SRC_RAW_ANALOG_LAST)
                *qqSrcRaw[idx] = movedSrc;
            }
            //inc dec
            *qqSrcRaw[idx] = incDecSource(*qqSrcRaw[idx], INCDEC_FLAG_MIX_SRC_RAW_ANALOG);
          }
          else if(focusedItem == 3)
            *rate = incDec(*rate, 0, 100, INCDEC_NOWRAP, INCDEC_NORMAL); 
          else if(focusedItem == 4)
            *expo = incDec(*expo, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
          else if(focusedItem == 5 && isEditMode)
            *swtch = incDecControlSwitch(*swtch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
          
          //--- Draw text
          
          display.setCursor(8, 9);
          getSrcName(textBuff, qqSrc[idx], sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
          
          display.setCursor(0, 20);
          display.print(F("Src:"));
          display.setCursor(42, 20);
          getSrcName(textBuff, *qqSrcRaw[idx], sizeof(textBuff));
          display.print(textBuff);

          display.setCursor(0, 29);
          display.print(F("Rate:"));
          display.setCursor(42, 29);
          display.print(*rate);
          display.print(F("%"));
          
          display.setCursor(0, 38);
          display.print(F("Expo:"));
          display.setCursor(42, 38);
          display.print(*expo);
          display.print(F("%"));
          
          display.setCursor(0, 47);
          display.print(F("D/R:"));
          display.setCursor(42, 47);
          getControlSwitchName(textBuff, *swtch, sizeof(textBuff));
          display.print(textBuff);
          
          //--- Show the current D/R in use. Only show if we have dual rates enabled
          if(*swtch != CTRL_SW_NONE)
          {
            display.setCursor(0, 56);
            if(checkSwitchCondition(*swtch))
              display.print(F("(Rate2)"));
            else
              display.print(F("(Rate1)"));
          }
          
          if(focusedItem > 1)
            drawCursor(34, (focusedItem * 9) + 2);

          //--- Draw graph
          
          //draw x y axis lines
          display.drawVLine(100, 11, 51, BLACK);
          display.drawHLine(75, 36, 51, BLACK);
          
          //draw stick input marker
          static int8_t lastVal = 0;
          static uint32_t lastMovedTime = 0;
          int16_t difference = qqValIn[idx]/5 - lastVal;
          if(difference >= 5 || difference <= -5)
          {
            lastVal = qqValIn[idx]/5;
            lastMovedTime = millis();
          }
          if(millis() - lastMovedTime < 3000)
          {
            display.setInterlace(false);
            drawDottedVLine(100 + qqValIn[idx]/20, 11, 51, BLACK, WHITE);
            int8_t y = calcRateExpo(qqValIn[idx], *rate, *expo) / 20;
            drawDottedHLine(75, 36 - y, 51, BLACK, WHITE);
          }
          
          //cache the y values so we don't have to recalculate them every time
          static int8_t lastRate, lastExpo;
          if(lastRate != *rate || lastExpo != *expo || graphYCoordinatesInvalid)
          {
            lastRate = *rate;
            lastExpo = *expo;
            graphYCoordinatesInvalid = false;
            for(int16_t i = 0; i <= 25; i++)
              graphYCoord[i] = calcRateExpo(i * 20, *rate, *expo) / 20;
          }
          
          //plot the points
          for(int16_t i = 0; i <= 25; i++)
          {
            display.drawPixel(100 + i, 36 - graphYCoord[i], BLACK);
            display.drawPixel(100 - i, 36 + graphYCoord[i], BLACK);
            if(i > 0)
            {
              int8_t y0 = graphYCoord[i - 1];
              int8_t y1 = graphYCoord[i];
              if(abs(y1 - y0) > 1)
              {
                display.drawLine(100 + i - 1, 36 - y0, 100 + i, 36 - y1, BLACK);
                display.drawLine(100 - i + 1, 36 + y0, 100 - i, 36 + y1, BLACK);
              }
            }
          }
        }
    
        //------ THROTTLE CURVE
        if(page == PAGE_THR_CURVE)
        {
          display.setCursor(8, 9);
          getSrcName(textBuff, SRC_THR, sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
          
          custom_curve_t *crv = &Model.ThrottleCurve;
          
          static uint8_t thisPt = 0;
          if(thisPt >= crv->numPoints)
            thisPt = 0;
          
          //scrollable list
          
          enum {
            ITEM_THR_SRC_RAW,
            ITEM_CURVE_NUM_POINTS,
            ITEM_CURVE_POINT,
            ITEM_CURVE_XVAL,
            ITEM_CURVE_YVAL,
            ITEM_CURVE_SMOOTH,
            
            ITEM_COUNT
          };

          //handle navigation
          changeFocusOnUpDown(ITEM_COUNT + 1); //+1 for title focus
          static uint8_t topItem = 1;
          if(focusedItem == 1)
            topItem = 1;
          else if(focusedItem > 1)
          {
            if(focusedItem - 1 < topItem)
              topItem = focusedItem - 1;
            while(focusedItem - 1 >= topItem + 5)
              topItem++;
          }
          
          bool markNode = false;
          
          //fill list and edit items
          for(uint8_t line = 0; line < 5 && line < ITEM_COUNT; line++)
          {
            uint8_t ypos = 20 + line*9;
            if(focusedItem - 1 == topItem + line)
              drawCursor(32, ypos);
            
            if((topItem - 1 + line) >= ITEM_COUNT)
              break;
            
            uint8_t itemID = topItem - 1 + line;
            bool isFocused = (focusedItem > 1 && itemID == focusedItem - 2);
            bool edit = (isFocused && isEditMode);
            
            display.setCursor(0, ypos);
            switch(itemID)
            {
              case ITEM_THR_SRC_RAW:
                {
                  display.print(F("Src:"));
                  display.setCursor(40, ypos);
                  getSrcName(textBuff, Model.thrSrcRaw, sizeof(textBuff));
                  display.print(textBuff);
                  if(edit)
                  {
                    //auto detect moved
                    uint8_t movedSrc = getMovedSource();
                    if(movedSrc != SRC_NONE)
                    {
                      if(movedSrc >= SRC_RAW_ANALOG_FIRST && movedSrc <= SRC_RAW_ANALOG_LAST)
                        Model.thrSrcRaw = movedSrc;
                    }
                    //inc dec
                    Model.thrSrcRaw = incDecSource(Model.thrSrcRaw, INCDEC_FLAG_MIX_SRC_RAW_ANALOG);
                  }
                }
                break;
              
              case ITEM_CURVE_NUM_POINTS:
                {
                  display.print(F("Curv:"));
                  display.setCursor(40, ypos);
                  display.print(crv->numPoints);
                  display.print(F("pts"));
                  if(edit)
                  {
                    uint8_t prevNumPoints = crv->numPoints;
                    crv->numPoints = incDec(crv->numPoints, MIN_NUM_POINTS_CUSTOM_CURVE, 
                                                    MAX_NUM_POINTS_CUSTOM_CURVE, INCDEC_NOWRAP, INCDEC_SLOW);
                    if(crv->numPoints != prevNumPoints) //recalculate points if changed
                    {
                      calcNewCurvePts(crv, prevNumPoints);
                      thisPt = 0;
                    }
                  }
                }
                break;
              
              case ITEM_CURVE_POINT:
                {
                  display.print(F("Pt:"));
                  display.setCursor(40, ypos);
                  display.write(97 + thisPt);
                  if(isFocused)
                    markNode = true;
                  if(edit)
                    thisPt = incDec(thisPt, 0, crv->numPoints - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
                break;
                
              case ITEM_CURVE_XVAL:
                {
                  display.print(F("Xval:"));
                  display.setCursor(40, ypos);
                  display.print(crv->xVal[thisPt]);
                  if(isFocused)
                    markNode = true;
                  if(edit && thisPt > 0 && thisPt < crv->numPoints - 1)
                  {
                    int8_t _minVal = crv->xVal[thisPt - 1];
                    int8_t _maxVal = crv->xVal[thisPt + 1];
                    crv->xVal[thisPt] = incDec(crv->xVal[thisPt], _minVal, _maxVal, INCDEC_NOWRAP, INCDEC_NORMAL);
                  }
                }
                break;
                
              case ITEM_CURVE_YVAL:
                {
                  display.print(F("Yval:"));
                  display.setCursor(40, ypos);
                  display.print(crv->yVal[thisPt]);
                  if(isFocused)
                    markNode = true;
                  if(edit)
                    crv->yVal[thisPt] = incDec(crv->yVal[thisPt], -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
                }
                break;
                
              case ITEM_CURVE_SMOOTH:
                {
                  display.print(F("Smth:"));
                  drawCheckbox(40, ypos, crv->smooth);
                  if(edit)
                    crv->smooth = incDec(crv->smooth, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                }
                break;
            }
          }
          
          //scrollbar
          drawScrollBar(71, 19, ITEM_COUNT, topItem, 5, 5 * 9);
        
          //--- draw graph
          //draw stick input marker
          static int8_t lastVal = mixSources[Model.thrSrcRaw]/5;
          static uint32_t lastMovedTime = 0;
          bool moved = false;
          int16_t difference = mixSources[Model.thrSrcRaw]/5 - lastVal;
          if(difference >= 5 || difference <= -5)
          {
            lastVal = mixSources[Model.thrSrcRaw]/5;
            lastMovedTime = millis();
          }
          if(millis() - lastMovedTime < 3000)
          {
            moved = true;
            display.setInterlace(false);
          }
          //draw graph
          drawCustomCurve(crv, markNode ? thisPt : 0xff, moved ? Model.thrSrcRaw : (uint8_t)SRC_NONE);
        }

        //------ STICKS
        if(page == PAGE_STICKS)
        {
          display.setCursor(8, 9);
          strlcpy_P(textBuff, PSTR("Sticks"), sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);

          uint8_t cntr = 0;
          for(uint8_t i = 0; i < NUM_STICK_AXES && i < 10; i++)
          {
            if(Sys.StickAxis[i].type == STICK_AXIS_ABSENT)
              continue;
            uint8_t xpos = 0;
            uint8_t ypos = 20 + cntr*9;
            if(cntr >= 5)
            {
              xpos = 69;
              ypos = 20 + (cntr-5)*9;
            }
            display.setCursor(xpos, ypos);
            getSrcName(textBuff, SRC_STICK_AXIS_FIRST + i, sizeof(textBuff));
            display.print(textBuff);
            display.print(F(":"));
            display.setCursor(display.getCursorX() + 6, ypos);
            display.print(stickAxisIn[i]/5);
            cntr++;
          }
        }
        
        //------ KNOBS
        if(page == PAGE_KNOBS)
        {
          display.setCursor(8, 9);
          strlcpy_P(textBuff, PSTR("Knobs"), sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
          
          for(uint8_t i = 0; i < NUM_KNOBS; i++)
          {
            if(Sys.Knob[i].type == KNOB_ABSENT)
              continue;
            uint8_t xpos = 0;
            uint8_t ypos = 20 + i*9;
            display.setCursor(xpos, ypos);
            getSrcName(textBuff, SRC_KNOB_FIRST + i, sizeof(textBuff));
            display.print(textBuff);
            display.print(F(":"));
            display.setCursor(display.getCursorX() + 6, ypos);
            display.print(knobIn[i]/5);
          }
        }
        
        //------ SWITCHES
        if(page == PAGE_SWITCHES)
        {
          display.setCursor(8, 9);
          strlcpy_P(textBuff, PSTR("Switches"), sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
          
          //show switches
          uint8_t cntr = 0;
          for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
          {
            if(Sys.swType[i] == SW_ABSENT)
              continue;
            
            uint8_t xpos = 0;
            uint8_t ypos = 20 + cntr * 9;
            if(cntr >= 5)
            {
              xpos = 69;
              ypos = 20 + (cntr - 5)*9;
            }
            display.setCursor(xpos, ypos);
            getSrcName(textBuff, SRC_SW_PHYSICAL_FIRST + i, sizeof(textBuff));
            display.print(textBuff);
            display.print(F(": "));
            if(swState[i] == SWUPPERPOS) display.print(F("-100"));
            else if(swState[i] == SWLOWERPOS) display.print(F("100"));
            else display.print(F("0"));
            
            cntr++;
          }
        }
        
        //------ Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MAIN_MENU);
      }
      break;
      
    ////////////////////////////////// MIXER ///////////////////////////////////////////////////////
      
    case SCREEN_MIXER:
      {
        drawHeader(mainMenu[MAIN_MENU_MIXER]);
        
        mixer_params_t *mxr = &Model.Mixer[thisMixIdx];
        
        //For safety and to prevent unintended effects, we do not directly edit the mixer output variable 
        //but instead use some kind of delayed action, where we edit a temporary variable 
        //and later assign it to the output in the right conditions.
        static uint8_t tempMixerOutput;
        static bool tempInitialised = false;
        if(!tempInitialised)
        {
          tempMixerOutput = mxr->output;
          tempInitialised = true;
        }
        
        display.setCursor(8, 9);
        display.print(F("Mix"));
        display.print(thisMixIdx + 1);
        if(!isEmptyStr(mxr->name, sizeof(mxr->name)))
        {
          display.setCursor(display.getCursorX() + 6, 9);
          display.print(mxr->name);
        }
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);

        //--- Dynamic list ---
        
        enum {
          ITEM_MIX_OUTPUT,
          ITEM_MIX_SWITCH,
          ITEM_MIX_OPERATION,
          ITEM_MIX_INPUT,
          ITEM_MIX_WEIGHT,
          ITEM_MIX_OFFSET,
          ITEM_MIX_CURVE_TYPE,
          ITEM_MIX_CURVE_VAL,
          ITEM_MIX_TRIM_ENABLED,
          ITEM_MIX_FLIGHT_MODE,
          ITEM_MIX_DELAY_UP,
          ITEM_MIX_DELAY_DOWN,
          ITEM_MIX_SLOW_UP,
          ITEM_MIX_SLOW_DOWN,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; //stores the added item IDs
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(mxr->operation == MIX_HOLD)
          {
            if(i != ITEM_MIX_OUTPUT && i != ITEM_MIX_SWITCH && i!= ITEM_MIX_OPERATION)
              continue;
          }
          if(i == ITEM_MIX_FLIGHT_MODE)
          {
            if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
              listItemIDs[listItemCount++] = i;
            continue;
          }
          listItemIDs[listItemCount++] = i;
        }
        
        //initialise the view
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
        
        //handle navigatation
        focusedItem = lastFocusedItem;
        uint8_t numFocusable = listItemCount + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        lastFocusedItem = focusedItem;
        
        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(52, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
    
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (focusedItem > 1 && itemID == listItemIDs[focusedItem - 2] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_MIX_OUTPUT:
              {
                display.print(F("Output:"));
                display.setCursor(60, ypos);
                getSrcName(textBuff, tempMixerOutput, sizeof(textBuff));
                display.print(textBuff);
                if(tempMixerOutput >= SRC_CH1 && tempMixerOutput < SRC_CH1 + NUM_RC_CHANNELS)
                {
                  uint8_t chIdx = tempMixerOutput - SRC_CH1;
                  display.setCursor(display.getCursorX() + 6, ypos);
                  display.print(Model.Channel[chIdx].name);
                }
                if(edit)
                {
                  do {
                  tempMixerOutput = incDecSource(tempMixerOutput, INCDEC_FLAG_MIX_SRC);
                  } while(tempMixerOutput > SRC_NONE && tempMixerOutput < SRC_CH1); //skip over
                }                
              }
              break;

            case ITEM_MIX_SWITCH:
              {
                display.print(F("Switch:"));
                display.setCursor(60, ypos);
                getControlSwitchName(textBuff, mxr->swtch, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  mxr->swtch = incDecControlSwitch(mxr->swtch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
              }
              break;
            
            case ITEM_MIX_OPERATION:
              {
                display.print(F("Opertn:"));
                display.setCursor(60, ypos);
                display.print(findStringInIdStr(enum_MixerOperation, mxr->operation));
                if(edit)
                  mxr->operation = incDec(mxr->operation, 0, MIX_OPERATOR_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_MIX_INPUT:
              {
                display.print(F("Input:"));
                display.setCursor(60, ypos);
                getSrcName(textBuff, mxr->input, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                {
                  //auto detect moved source
                  uint8_t movedSrc = procMovedSource(getMovedSource());
                  if(movedSrc != SRC_NONE)
                    mxr->input = movedSrc;
                  //inc dec
                  mxr->input = incDecSource(mxr->input, INCDEC_FLAG_MIX_SRC);
                }
              }
              break;
              
            case ITEM_MIX_WEIGHT:
              {
                display.print(F("Weight:"));
                display.setCursor(60, ypos);
                display.print(mxr->weight);
                if(edit)
                  mxr->weight = incDec(mxr->weight, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_OFFSET:
              {
                display.print(F("Offset:"));
                display.setCursor(60, ypos);
                display.print(mxr->offset);
                if(edit)
                  mxr->offset = incDec(mxr->offset, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_CURVE_TYPE:
              {
                display.print(F("Curve:"));
                display.setCursor(60, ypos);
                display.print(findStringInIdStr(enum_MixerCurveType, mxr->curveType));
                if(edit)
                {
                  uint8_t prevCurveType = mxr->curveType;
                  mxr->curveType = incDec(mxr->curveType, 0, MIX_CURVE_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  if(mxr->curveType != prevCurveType) 
                    mxr->curveVal = 0; //reset value if changed
                }
              }
              break;
              
            case ITEM_MIX_CURVE_VAL:
              {
                display.print(F("Crv val:"));
                display.setCursor(60, ypos);
                if(mxr->curveType == MIX_CURVE_TYPE_DIFF || mxr->curveType == MIX_CURVE_TYPE_EXPO)
                  display.print(mxr->curveVal);
                else if(mxr->curveType == MIX_CURVE_TYPE_FUNCTION)
                  display.print(findStringInIdStr(enum_MixerCurveType_Func, mxr->curveVal));
                else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
                {
                  if(isEmptyStr(Model.CustomCurve[mxr->curveVal].name, sizeof(Model.CustomCurve[0].name)))
                  {
                    display.print(F("Crv"));
                    display.print(mxr->curveVal + 1);
                  }
                  else
                    display.print(Model.CustomCurve[mxr->curveVal].name);
                }
                if(edit)
                {
                  if(mxr->curveType == MIX_CURVE_TYPE_DIFF || mxr->curveType == MIX_CURVE_TYPE_EXPO)
                    mxr->curveVal = incDec(mxr->curveVal, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
                  else if(mxr->curveType == MIX_CURVE_TYPE_FUNCTION)
                    mxr->curveVal = incDec(mxr->curveVal, 0, MIX_CURVE_FUNC_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
                    mxr->curveVal = incDec(mxr->curveVal, 0, NUM_CUSTOM_CURVES - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
              }
              break;
              
            case ITEM_MIX_TRIM_ENABLED:
              {
                display.print(F("Trim:"));
                display.setCursor(60, ypos);
                uint8_t input = mxr->input;
                if(input == SRC_RUD) input = Model.rudSrcRaw;
                if(input == SRC_THR) input = Model.thrSrcRaw;
                if(input == SRC_AIL) input = Model.ailSrcRaw;
                if(input == SRC_ELE) input = Model.eleSrcRaw;
                if(input ==  SRC_X1_AXIS || input == SRC_Y1_AXIS || input == SRC_X2_AXIS || input == SRC_Y2_AXIS)
                {
                  drawCheckbox(60, ypos, mxr->trimEnabled);
                  if(edit)
                    mxr->trimEnabled = incDec(mxr->trimEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                }
                else
                  display.print(F("N/A"));
              }
              break;
              
            case ITEM_MIX_FLIGHT_MODE:
              {
                display.print(F("F-mode:"));
                display.setCursor(60, ypos);
                if(mxr->flightMode == 0xff)
                  display.print(F("All"));
                else
                {
                  bool delimit = false;
                  for(uint8_t i = 0; i < NUM_FLIGHT_MODES; i++)
                  {
                    if((mxr->flightMode >> i) & 0x01)
                    {
                      if(delimit)
                        display.print(F(","));
                      display.print(i+1);
                      delimit = true;
                    }
                  }
                }
                if(edit)
                  changeToScreen(DIALOG_MIX_FLIGHT_MODE);
              }
              break;
              
            case ITEM_MIX_DELAY_UP:
              {
                display.print(F("Dly up:"));
                display.setCursor(60, ypos);
                printSeconds(mxr->delayUp);
                if(edit)
                  mxr->delayUp = incDec(mxr->delayUp, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_DELAY_DOWN:
              {
                display.print(F("Dly dn:"));
                display.setCursor(60, ypos);
                printSeconds(mxr->delayDown);
                if(edit)
                  mxr->delayDown = incDec(mxr->delayDown, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_SLOW_UP:
              {
                display.print(F("Slow up:"));
                display.setCursor(60, ypos);
                printSeconds(mxr->slowUp);
                if(edit)
                  mxr->slowUp = incDec(mxr->slowUp, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_SLOW_DOWN:
              {
                display.print(F("Slow dn:"));
                display.setCursor(60, ypos);
                printSeconds(mxr->slowDown);
                if(edit)
                  mxr->slowDown = incDec(mxr->slowDown, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //Show context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);
        
        //Assign from temp
        if(!isEditMode || (buttonCode == 0 && millis() - buttonReleaseTime > 1000))
        {
          mxr->output = tempMixerOutput;
          tempInitialised = false;
        }
        
        //change to next mix
        if(focusedItem == 1)
        {
          drawCursor(0, 9);
          if(isEditMode)
          {
            //assign from temp
            mxr->output = tempMixerOutput;
            tempInitialised = false;
            //change to another mix
            thisMixIdx = incDec(thisMixIdx, 0, NUM_MIX_SLOTS - 1, INCDEC_WRAP, INCDEC_SLOW);
          }
        }
        
        //Open context menu
        if(focusedItem == numFocusable && clickedButton == KEY_SELECT)
        {
          changeToScreen(CONTEXT_MENU_MIXER);
          viewInitialised = false;
          //assign from temp
          mxr->output = tempMixerOutput;
          tempInitialised = false;
        }

        //Exit
        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_MAIN_MENU);
          //assign from temp
          mxr->output = tempMixerOutput; 
          tempInitialised = false;
        } 
      }
      break;
      
    case DIALOG_MIX_FLIGHT_MODE:
      {
        //--- scrollable list

        //handle navigation
        changeFocusOnUpDown(NUM_FLIGHT_MODES);
        static uint8_t topItem = 1;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 4)
          topItem++;
        
        //Calculate y coord for item 0. Items are center aligned
        uint8_t numVisible = NUM_FLIGHT_MODES <= 4 ? NUM_FLIGHT_MODES : 4;
        uint8_t y0 = ((display.height() - (numVisible * 9)) / 2) + 1;  //9 is line height
        
        //draw bounding box
        drawBoundingBox(11, y0 - 3, 105, numVisible * 9 + 4, BLACK);
        
        //fill list
        for(uint8_t line = 0; line < numVisible; line++)
        {
          uint8_t ypos = y0 + line * 9;
          uint8_t item = topItem + line;
          if(focusedItem == item)
            drawCursor(17, ypos);
          uint8_t idx = item - 1;
          drawCheckbox(25, ypos, Model.Mixer[thisMixIdx].flightMode & (1 << idx));
          display.setCursor(35, ypos);
          display.print(F("Fmd"));
          display.print(idx + 1);
          display.print(F(" "));
          display.print(Model.FlightMode[idx].name);
        }
        
        //scroll bar
        uint8_t  y = Sys.useRoundRect ? y0 : y0 - 1;
        uint16_t h = Sys.useRoundRect ? numVisible * 9 - 2 : numVisible * 9;
        drawScrollBar(112, y, NUM_FLIGHT_MODES, topItem, numVisible, h);
        
        //--- edit items
        toggleEditModeOnSelectClicked();
        if(isEditMode && (pressedButton == KEY_UP || pressedButton == KEY_DOWN))
        {
          uint8_t idx = focusedItem - 1;
          Model.Mixer[thisMixIdx].flightMode ^= 1 << idx; //toggle bit
        }
        
        //--- exit 
        if(heldButton == KEY_SELECT)
        {
          if((Model.Mixer[thisMixIdx].flightMode & (uint8_t)((1 << NUM_FLIGHT_MODES) - 1)) == 0) //nothing selected
          {
            killButtonEvents();
            makeToast(PSTR("Select at least one"), 2000, 0);
          }
          else 
            changeToScreen(SCREEN_MIXER);
        }
      }
      break;

    case CONTEXT_MENU_MIXER:
      {
        enum {
          ITEM_MIXER_OUTPUTS,
          ITEM_COPY_MIX,
          ITEM_MOVE_MIX,
          ITEM_INSERT_FREE,
          ITEM_RENAME_MIX,
          ITEM_RESET_MIX,
          ITEM_RESET_ALL_MIXES,
          ITEM_COMPACT_MIXES,
          ITEM_MIXER_TEMPLATES,
          ITEM_MIXER_OVERVIEW,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_MIXER_OUTPUTS);
        contextMenuAddItem(PSTR("Copy mix to"), ITEM_COPY_MIX);
        contextMenuAddItem(PSTR("Move mix to"), ITEM_MOVE_MIX);
        contextMenuAddItem(PSTR("Insert free"), ITEM_INSERT_FREE);
        contextMenuAddItem(PSTR("Rename mix" ), ITEM_RENAME_MIX);
        contextMenuAddItem(PSTR("Reset mix" ), ITEM_RESET_MIX);
        contextMenuAddItem(PSTR("Reset all mixes"), ITEM_RESET_ALL_MIXES);
        contextMenuAddItem(PSTR("Compact mixes"), ITEM_COMPACT_MIXES);
        if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
        {
          if(Sys.mixerTemplatesEnabled)
            contextMenuAddItem(PSTR("Templates"), ITEM_MIXER_TEMPLATES);
        }
        contextMenuAddItem(PSTR("Show overview"), ITEM_MIXER_OVERVIEW);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_MIXER_OUTPUTS) 
          changeToScreen(SCREEN_MIXER_OUTPUT);
        if(contextMenuSelectedItemID == ITEM_COPY_MIX)
        {
          destMixIdx = thisMixIdx;
          changeToScreen(DIALOG_COPY_MIX);
        }
        if(contextMenuSelectedItemID == ITEM_MOVE_MIX)
        {
          destMixIdx = thisMixIdx;
          changeToScreen(DIALOG_MOVE_MIX);
        }
        if(contextMenuSelectedItemID == ITEM_INSERT_FREE)
        {
          //Find a slot that is empty.
          //If the slot exists after thisMixIdx, move it here, otherwise we dont.
          uint8_t freeSlot = 0xFF;
          uint8_t mixIdx = NUM_MIX_SLOTS - 1;
          uint8_t occupiedCount = 0;
          while(mixIdx > thisMixIdx)
          {
            if(freeSlot == 0xFF && Model.Mixer[mixIdx].output == SRC_NONE && Model.Mixer[mixIdx].input == SRC_NONE)
              freeSlot = mixIdx;
            //count how many occupied slots exist between the free slot and the destination
            if(freeSlot != 0xFF && (Model.Mixer[mixIdx].output != SRC_NONE || Model.Mixer[mixIdx].input != SRC_NONE))
              occupiedCount++;
            mixIdx--;
          }
          if(freeSlot == 0xFF || (occupiedCount == 0 && Model.Mixer[thisMixIdx].output == SRC_NONE 
                                  && Model.Mixer[thisMixIdx].input == SRC_NONE))
          {
            makeToast(PSTR("Insert failed"), 2000, 0);
          }
          else
          {
            resetMixerParams(freeSlot);
            moveMix(thisMixIdx, freeSlot);
          }
          changeToScreen(SCREEN_MIXER);
        }
        if(contextMenuSelectedItemID == ITEM_RENAME_MIX)
          changeToScreen(DIALOG_RENAME_MIX);
        if(contextMenuSelectedItemID == ITEM_RESET_MIX)
        {
          resetMixerParams(thisMixIdx);
          changeToScreen(SCREEN_MIXER);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_ALL_MIXES)
          changeToScreen(CONFIRMATION_MIXES_RESET);
        if(contextMenuSelectedItemID == ITEM_COMPACT_MIXES)
        {
          //Find all empty slots and move them to the end.
          //This is similar to the classic 'move all zeros to end of array' problem.
          
          //find the rearmost occupied slot
          uint8_t rearmostIdx = 0;
          for(uint8_t mixIdx = NUM_MIX_SLOTS - 1; mixIdx > 0; mixIdx--)
          {
            if(Model.Mixer[mixIdx].output != SRC_NONE || Model.Mixer[mixIdx].input != SRC_NONE)
            {
              rearmostIdx = mixIdx;
              break;
            }
          }
          //find and move all free slots to end
          uint8_t mixIdx = 0;
          while(mixIdx < rearmostIdx)
          {
            if(Model.Mixer[mixIdx].output == SRC_NONE && Model.Mixer[mixIdx].input == SRC_NONE)
            {
              moveMix(NUM_MIX_SLOTS - 1, mixIdx);
              rearmostIdx--;
              //we don't immediately increment the mixIdx to also move any adjacent free slots
            }
            else
              mixIdx++;
          }
          thisMixIdx = 0;
          changeToScreen(SCREEN_MIXER);
        }
        if(contextMenuSelectedItemID == ITEM_MIXER_TEMPLATES)
          changeToScreen(CONTEXT_MENU_MIXER_TEMPLATES);
        if(contextMenuSelectedItemID == ITEM_MIXER_OVERVIEW)
          changeToScreen(SCREEN_MIXER_OVERVIEW);
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MIXER);
      }
      break;
      
    case CONTEXT_MENU_MIXER_TEMPLATES:
    case CONFIRMATION_LOAD_MIXER_TEMPLATE:
      {
        enum {
          ITEM_BASIC,
          ITEM_ELEVON,
          ITEM_VTAIL,
          ITEM_DIFF_THRUST
        };
        
        if(theScreen == CONTEXT_MENU_MIXER_TEMPLATES)
        {
          contextMenuInitialise();
          contextMenuAddItem(PSTR("Basic"), ITEM_BASIC);
          if(Model.type == MODEL_TYPE_AIRPLANE)
          {
            contextMenuAddItem(PSTR("Elevon"), ITEM_ELEVON);
            contextMenuAddItem(PSTR("Vtail"), ITEM_VTAIL);
            contextMenuAddItem(PSTR("Diff thrust"), ITEM_DIFF_THRUST);
          }
          contextMenuDraw();
          
          if(contextMenuSelectedItemID != 0xff) //selection
            theScreen = CONFIRMATION_LOAD_MIXER_TEMPLATE;

          if(heldButton == KEY_SELECT) //exit
            changeToScreen(SCREEN_MIXER);
        }
        else if(theScreen == CONFIRMATION_LOAD_MIXER_TEMPLATE)
        {
          bool hasWarning = false;
          //basic
          if(contextMenuSelectedItemID == ITEM_BASIC) 
          {
            if(hasEnoughMixSlots(thisMixIdx, 4))
            {
              if(hasOccupiedMixSlots(thisMixIdx, 4))
                hasWarning = true;
              if(clickedButton == KEY_UP || !hasWarning)
              {
                loadMixerTemplateBasic(thisMixIdx);
                changeToScreen(SCREEN_MIXER);
              }
            }
            else
              changeToScreen(SCREEN_MIXER);
          }
          //elevon
          if(contextMenuSelectedItemID == ITEM_ELEVON) 
          {
            if(hasEnoughMixSlots(thisMixIdx, 4))
            {
              if(hasOccupiedMixSlots(thisMixIdx, 4))
                hasWarning = true;
              if(clickedButton == KEY_UP || !hasWarning)
              {
                loadMixerTemplateElevon(thisMixIdx);
                changeToScreen(SCREEN_MIXER);
              }
            }
            else
              changeToScreen(SCREEN_MIXER);
          }
          //vtail
          if(contextMenuSelectedItemID == ITEM_VTAIL) 
          {
            if(hasEnoughMixSlots(thisMixIdx, 4))
            {
              if(hasOccupiedMixSlots(thisMixIdx, 4))
                hasWarning = true;
              if(clickedButton == KEY_UP || !hasWarning)
              {
                loadMixerTemplateVtail(thisMixIdx);
                changeToScreen(SCREEN_MIXER);
              }
            }
            else
              changeToScreen(SCREEN_MIXER);
          }
          //differential thrust
          if(contextMenuSelectedItemID == ITEM_DIFF_THRUST) 
          {
            if(hasEnoughMixSlots(thisMixIdx, 4))
            {
              if(hasOccupiedMixSlots(thisMixIdx, 4))
                hasWarning = true;
              if(clickedButton == KEY_UP || !hasWarning)
              {
                loadMixerTemplateDiffThrust(thisMixIdx);
                changeToScreen(SCREEN_MIXER);
              }
            }
            else
              changeToScreen(SCREEN_MIXER);
          }
          
          //show warning
          if(hasWarning)
            printFullScreenMessage(PSTR("One or more\ndestination slots\nalready occupied.\nContinue?\n\nYes [Up] \nNo [Down]"));
          
          //exit
          if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
            changeToScreen(SCREEN_MIXER);
        }
      }
      break;

    case DIALOG_RENAME_MIX:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Mix name"), Model.Mixer[thisMixIdx].name, sizeof(Model.Mixer[0].name), true, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_MIXER);
      }
      break;
      
    case CONFIRMATION_MIXES_RESET:
      {
        printFullScreenMessage(PSTR("Reset all mixes?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          resetMixerParams();
          thisMixIdx = 0;
          destMixIdx = 0;
          changeToScreen(SCREEN_MIXER);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MIXER);
      }
      break;
      
    case SCREEN_MIXER_OUTPUT:
      {
        display.setInterlace(false); 
        
        drawHeader(PSTR("Mixer output"));
        
        uint8_t numPagesPrpCh = (NUM_RC_CHANNELS + 7) / 8;
        uint8_t numPagesVrtCh = (NUM_VIRTUAL_CHANNELS + 7) / 8;
        uint8_t numPages = numPagesPrpCh + numPagesVrtCh;
        static uint8_t thisPage = 1;
        static bool viewInitialised = false;
        if(!viewInitialised) //start in the page that has the channel we want to view
        {
          uint8_t _outputIdx = Model.Mixer[thisMixIdx].output;
          if(_outputIdx >= SRC_CH1 && _outputIdx < (SRC_CH1 + NUM_RC_CHANNELS))
            thisPage = ((_outputIdx - SRC_CH1) + 8) / 8;
          else if(_outputIdx >= SRC_VIRTUAL_FIRST && _outputIdx <= SRC_VIRTUAL_LAST)
            thisPage = numPagesPrpCh + ((_outputIdx - SRC_VIRTUAL_FIRST) + 8) / 8;
          viewInitialised = true;
        }
        
        //change page
        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        //--- draw graphs---
        uint8_t startIdx = (thisPage - 1) * 8;
        uint8_t endIdx = NUM_RC_CHANNELS - 1;
        uint8_t offsetIdx = 0;
        display.setCursor(0, 7);
        if(thisPage <= numPagesPrpCh) //show real channels 
          display.print(F("Ch"));
        else //show virtual channels
        {  
          display.print(F("Virt"));
          startIdx = NUM_RC_CHANNELS + (8 * (thisPage - (numPagesPrpCh + 1)));
          endIdx = NUM_RC_CHANNELS + NUM_VIRTUAL_CHANNELS - 1;
          offsetIdx = NUM_RC_CHANNELS;
        }
        uint8_t count = 0;
        for(uint8_t i = startIdx; i < startIdx + 8 && i <= endIdx; i++)
        {
          int8_t outVal = mixSources[SRC_CH1 + i] / 25;
          uint8_t xOffset = (i - startIdx) * 16;
          if(outVal > 0)
            display.fillRect(6 + xOffset, 36 - outVal, 3, outVal , BLACK);
          else if(outVal < 0)
          {
            outVal = -outVal;
            display.fillRect(6 + xOffset, 35, 3, outVal, BLACK);
          }
          drawDottedVLine(7 + xOffset, 16, 39, BLACK, WHITE);
          //show the channel number
          uint8_t num = (i + 1) - offsetIdx;
          if(num < 10)
            display.setCursor(5 + xOffset, 57);
          else
            display.setCursor(2 + xOffset, 57);
          display.print(num);
          count++;
        }
        //draw midpoint
        display.drawHLine(3, 35, 9 + 16*(count - 1) , BLACK); 
        //show scrollbar
        drawScrollBar(127, 8, numPages, thisPage, 1, 1 * 56);

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_MIXER);
        }
      }
      break;
    
    case DIALOG_COPY_MIX:
    case DIALOG_MOVE_MIX:
      {
        isEditMode = true;
        destMixIdx = incDec(destMixIdx, 0, NUM_MIX_SLOTS - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Mix"), thisMixIdx, destMixIdx, theScreen == DIALOG_COPY_MIX);
        if(clickedButton == KEY_SELECT)
        {
          if(theScreen == DIALOG_COPY_MIX)
            Model.Mixer[destMixIdx] =  Model.Mixer[thisMixIdx];
          else
            moveMix(destMixIdx, thisMixIdx);
          thisMixIdx = destMixIdx; 
          changeToScreen(SCREEN_MIXER); 
        }
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MIXER);
      }
      break;
      
    case SCREEN_MIXER_OVERVIEW:
      {
        drawHeader(PSTR("Overview"));
        
        //--- dynamic scrollable list
        
        uint8_t listItemIDs[NUM_MIX_SLOTS];
        uint8_t listItemCount = 0;
        //add to list
        for(uint8_t i = 0; i < NUM_MIX_SLOTS; i++)
        {
          if(Model.Mixer[i].output == SRC_NONE)
            continue;
          if(Model.Mixer[i].input == SRC_NONE && Model.Mixer[i].operation != MIX_HOLD)
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        static uint8_t thisPage = 1;
        
        if(listItemCount == 0)
          printFullScreenMessage(PSTR("No mixes defined yet"));
        else
        {
          uint8_t numPages = (listItemCount + 5) / 6;
          isEditMode = true;
          thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
          
          uint8_t startIdx = (thisPage - 1) * 6;
          for(uint8_t i = startIdx; i < startIdx + 6 && i < listItemCount; i++)
          {
            uint8_t ypos = 9 * (i - startIdx + 1);
            uint8_t mixIdx = listItemIDs[i];
            
            //output
            display.setCursor(0, ypos);
            getSrcName(textBuff, Model.Mixer[mixIdx].output, sizeof(textBuff));
            display.print(textBuff);
            
            //operation
            display.setCursor(30, ypos);
            if(Model.Mixer[mixIdx].operation == MIX_ADD) display.print(F("+="));
            if(Model.Mixer[mixIdx].operation == MIX_MULTIPLY) display.print(F("*="));
            if(Model.Mixer[mixIdx].operation == MIX_REPLACE) display.print(F("="));
            if(Model.Mixer[mixIdx].operation == MIX_HOLD) display.print(F("Hold"));
            
            if(Model.Mixer[mixIdx].operation != MIX_HOLD)
            {
              //weight. Right align
              uint8_t digits = 1;
              uint8_t num = abs(Model.Mixer[mixIdx].weight);          
              while(num >= 10)
              {
                num /= 10;
                digits++;
              }
              uint8_t xpos = 60 - (digits - 1)*6;
              if(Model.Mixer[mixIdx].weight < 0)
                xpos -= 6;
              display.setCursor(xpos, ypos);
              display.print(Model.Mixer[mixIdx].weight);
              
              //input 
              display.setCursor(66, ypos);
              getSrcName(textBuff, Model.Mixer[mixIdx].input, sizeof(textBuff));
              display.print(textBuff);
            }
            
            //swtch, right align
            if(Model.Mixer[mixIdx].swtch != CTRL_SW_NONE)
            {
              getControlSwitchName(textBuff, Model.Mixer[mixIdx].swtch, sizeof(textBuff));
              uint8_t len = strlen(textBuff);
              uint8_t xpos = 126 - len * 6;
              display.fillRect(xpos, ypos, (len * 6) - 1, 8, WHITE); //clear area just in case the preceding text overflowed
              display.setCursor(xpos, ypos);
              display.print(textBuff);
            }
          }
          
          //scrollbar
          drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);
        }

        if(heldButton == KEY_SELECT)
        {
          thisPage = 1;
          changeToScreen(SCREEN_MIXER);
        }
      }
      break;
      
    ////////////////////////////////// OUTPUTS /////////////////////////////////////////////////////

    case SCREEN_OUTPUTS:
      {
        channel_params_t *ch = &Model.Channel[thisChIdx];

        drawHeader(mainMenu[MAIN_MENU_OUTPUTS]);
        
        display.setCursor(8, 9);
        display.print(F("Ch"));
        display.print(thisChIdx + 1);
        if(!isEmptyStr(ch->name, sizeof(ch->name)))
        {
          display.setCursor(display.getCursorX() + 6, 9);
          display.print(ch->name);
        }
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //--- scrollable list 
        
        enum {
          ITEM_REVERSE,
          ITEM_SUBTRIM,
          ITEM_OVERRIDE_SWITCH,
          ITEM_OVERRIDE_VALUE,
          ITEM_FAILSAFE,
          ITEM_ENDPOINT_L,
          ITEM_ENDPOINT_R,
          ITEM_CURVE,
          
          ITEM_COUNT
        };
        
        static const uint8_t qqTab[ITEM_COUNT][2] PROGMEM = {
          //{Item, Vertical offset} 
          //Items should be ordered as in the enumeration above 
          {ITEM_REVERSE, 0},
          {ITEM_SUBTRIM, 1},
          {ITEM_OVERRIDE_SWITCH, 2},
          {ITEM_OVERRIDE_VALUE, 2},
          {ITEM_FAILSAFE, 3},
          {ITEM_ENDPOINT_L, 4},
          {ITEM_ENDPOINT_R, 4},
          {ITEM_CURVE, 5}
        };
        
        const uint8_t totalLines = 6;
        const uint8_t maxVisibleLines = 5;
        
        static uint8_t topLine = 0;
        
        //handle navigation
        uint8_t numFocusable = ITEM_COUNT + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topLine = 0;
        else if(focusedItem > 1)
        {
          uint8_t verticalOffset = pgm_read_byte(&(qqTab[focusedItem - 2][1]));
          if(verticalOffset < topLine)
            topLine = verticalOffset;
          while(verticalOffset >= topLine + maxVisibleLines)
            topLine++;
        }

        //fill list and edit items
        uint8_t itemID = topLine;
        while(itemID < ITEM_COUNT)
        {
          bool isFocused = (focusedItem > 1 && focusedItem != numFocusable && itemID == focusedItem - 2);
          bool edit = (isFocused && isEditMode);
          
          uint8_t ypos = 20 + (pgm_read_byte(&(qqTab[itemID][1])) - topLine) * 9;
          if(ypos > (20 + (maxVisibleLines - 1) * 9))
            break;

          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_REVERSE:
              {
                display.print(F("Reverse:"));
                drawCheckbox(62, ypos, ch->reverse);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->reverse = incDec(ch->reverse, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_SUBTRIM:
              {
                display.print(F("Subtrim:"));
                display.setCursor(62, ypos);
                int16_t val = ch->subtrim;
                if(val < 0)
                {
                  display.print(F("-"));
                  val = -val;
                }
                display.print(val / 10);
                display.print(F("."));
                display.print(val % 10);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                {
                  //restrict to multiples of 2
                  int16_t val = ch->subtrim / 2;
                  val = incDec(val, TRIM_MIN_VAL / 2, TRIM_MAX_VAL / 2, INCDEC_NOWRAP, INCDEC_NORMAL);
                  ch->subtrim = val * 2;
                }
              }
              break;
            
            case ITEM_OVERRIDE_SWITCH:
              {
                display.print(F("Override:"));
                display.setCursor(62, ypos);
                getControlSwitchName(textBuff, ch->overrideSwitch, sizeof(textBuff));
                display.print(textBuff);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->overrideSwitch = incDecControlSwitch(ch->overrideSwitch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
              }
              break;
            
            case ITEM_OVERRIDE_VALUE:
              {
                display.setCursor(98, ypos);
                if(ch->overrideSwitch == CTRL_SW_NONE) 
                  display.print(F("--"));
                else 
                  display.print(ch->overrideVal);
                if(isFocused)
                  drawCursor(90, ypos);
                if(edit && ch->overrideSwitch != CTRL_SW_NONE)
                  ch->overrideVal = incDec(ch->overrideVal, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_FAILSAFE:
              {
                display.print(F("Failsafe:"));
                display.setCursor(62, ypos);
                if(ch->failsafe < -100)
                  display.print(findStringInIdStr(enum_ChannelFailsafe, ch->failsafe));
                else
                  display.print(ch->failsafe);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->failsafe = incDec(ch->failsafe, -102, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_ENDPOINT_L:
              {
                display.print(F("Endpts:"));
                display.setCursor(62, ypos);
                display.print(ch->endpointL);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->endpointL = incDec(ch->endpointL, -100, 0, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
            
            case ITEM_ENDPOINT_R:
              {
                display.setCursor(98, ypos);
                display.print(ch->endpointR);
                if(isFocused)
                  drawCursor(90, ypos);
                if(edit)
                  ch->endpointR = incDec(ch->endpointR, 0, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_CURVE:
              {
                display.print(F("Curve:"));
                display.setCursor(62, ypos);
                if(ch->curve == -1)
                {
                  display.print(findStringInIdStr(enum_ChannelCurve, ch->curve));
                  // display.print(F("--"));
                }
                else
                {
                  if(isEmptyStr(Model.CustomCurve[ch->curve].name, sizeof(Model.CustomCurve[0].name)))
                  {
                    display.print(F("Crv"));
                    display.print(ch->curve + 1);
                  }
                  else
                    display.print(Model.CustomCurve[ch->curve].name);
                }
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->curve = incDec(ch->curve, -1, NUM_CUSTOM_CURVES - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
          }

          itemID++;
        }
        
        //scrollbar
        drawScrollBar(127, 19, totalLines, topLine + 1, maxVisibleLines, maxVisibleLines * 9);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);

        //open context menu
        if(focusedItem == numFocusable && isEditMode)
          changeToScreen(CONTEXT_MENU_OUTPUTS);
        
        //change to next
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisChIdx = incDec(thisChIdx, 0, NUM_RC_CHANNELS - 1, INCDEC_WRAP, INCDEC_SLOW); 
        }

        // Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MAIN_MENU);
      }
      break;
 
    case CONTEXT_MENU_OUTPUTS:
      {
        enum {
          ITEM_CHANNEL_MONITOR,
          ITEM_RENAME_CHANNEL,
          ITEM_RESET_SETTINGS,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_CHANNEL_MONITOR);
        contextMenuAddItem(PSTR("Rename channel"), ITEM_RENAME_CHANNEL);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RENAME_CHANNEL)
          changeToScreen(DIALOG_RENAME_CHANNEL);
        if(contextMenuSelectedItemID == ITEM_CHANNEL_MONITOR)
          changeToScreen(SCREEN_CHANNEL_MONITOR);
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetChannelParams(thisChIdx);
          changeToScreen(SCREEN_OUTPUTS);
        }
        
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_OUTPUTS);
      }
      break;
      
    case DIALOG_RENAME_CHANNEL:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Channel name"), Model.Channel[thisChIdx].name, sizeof(Model.Channel[0].name), true, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_OUTPUTS);
      }
      break;
      
    ////////////////////////////////// EXTRAS //////////////////////////////////////////////////////
      
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
      
    case SCREEN_CUSTOM_CURVES:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_CUSTOM_CURVES]);
        
        custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx]; 
        
        display.setCursor(8, 9);
        display.print(F("Crv"));
        display.print(thisCrvIdx + 1);
        if(!isEmptyStr(crv->name, sizeof(crv->name)))
        {
          display.setCursor(display.getCursorX() + 6, 9);
          display.print(crv->name);
        }
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        display.setCursor(0, 20);
        display.print(F("Type:"));
        display.setCursor(42, 20);
        display.print(crv->numPoints);
        display.print(F("pts"));
        
        display.setCursor(0, 29);
        display.print(F("Pt:"));
        display.setCursor(42, 29);
        display.write(97 + thisCrvPt);
        
        display.setCursor(0, 38);
        display.print(F("Xval:"));
        display.setCursor(42, 38);
        display.print(crv->xVal[thisCrvPt]);
        
        display.setCursor(0, 47);
        display.print(F("Yval:"));
        display.setCursor(42, 47);
        display.print(crv->yVal[thisCrvPt]);
        
        display.setCursor(0, 56);
        display.print(F("Smth:"));
        drawCheckbox(42, 56, crv->smooth);

        //draw cursor
        if(focusedItem == 1) 
          drawCursor(0, 9);
        else if(focusedItem < 7)
          drawCursor(34, (focusedItem * 9) + 2);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == 7 ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);
        
        //show moved input on graph
        static uint8_t movedSrc = SRC_NONE;
        bool moved = false;
        uint8_t tmpMovedSrc = getMovedSource();
        if(tmpMovedSrc != SRC_NONE)
          movedSrc = tmpMovedSrc;
        if(movedSrc != SRC_NONE)
        {
          static int8_t lastVal = 0;
          static uint32_t lastMovedTime = 0;
          int16_t difference = mixSources[movedSrc]/5 - lastVal;
          if(difference >= 5 || difference <= -5)
          {
            lastVal = mixSources[movedSrc]/5;
            lastMovedTime = millis();
          }
          if(millis() - lastMovedTime < 3000)
          {
            moved = true;
            display.setInterlace(false);
          }
          else
            movedSrc = SRC_NONE;
        }

        //draw graph
        drawCustomCurve(crv, (focusedItem >= 3 && focusedItem < 6) ? thisCrvPt : 0xff, moved ? movedSrc : (uint8_t)SRC_NONE);

        //change focus
        changeFocusOnUpDown(7);
        toggleEditModeOnSelectClicked();
        
        //---- edit params
        if(focusedItem == 1) //change to next or previous curve
        {
          uint8_t _prevCrvIdx = thisCrvIdx;
          thisCrvIdx = incDec(thisCrvIdx, 0, NUM_CUSTOM_CURVES - 1, INCDEC_WRAP, INCDEC_SLOW);
          if(thisCrvIdx != _prevCrvIdx)
            thisCrvPt = 0;
        }
        else if(focusedItem == 2) //change number of points
        {
          uint8_t prevNumPoints = crv->numPoints;
          crv->numPoints = incDec(crv->numPoints, MIN_NUM_POINTS_CUSTOM_CURVE, MAX_NUM_POINTS_CUSTOM_CURVE, 
                                          INCDEC_NOWRAP, INCDEC_SLOW);
          if(crv->numPoints != prevNumPoints) //recalculate points if changed
          {
            calcNewCurvePts(crv, prevNumPoints);
            thisCrvPt = 0;
          }
        }
        else if(focusedItem == 3) //move to next/prev point
          thisCrvPt = incDec(thisCrvPt, 0, crv->numPoints - 1, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 4 && isEditMode) //edit x values
        {
          if(thisCrvPt > 0 && thisCrvPt < crv->numPoints - 1) //only edit inner x values
          {
            int8_t _minVal = crv->xVal[thisCrvPt - 1];
            int8_t _maxVal = crv->xVal[thisCrvPt + 1];
            crv->xVal[thisCrvPt] = incDec(crv->xVal[thisCrvPt], _minVal, _maxVal, INCDEC_NOWRAP, INCDEC_NORMAL);
          }
        }
        else if(focusedItem == 5) //edit Y values
          crv->yVal[thisCrvPt] = incDec(crv->yVal[thisCrvPt], -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
        else if(focusedItem == 6) //smoothing
          crv->smooth = incDec(crv->smooth, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 7 && isEditMode) //context menu
          changeToScreen(CONTEXT_MENU_CUSTOM_CURVE);

        ////// Exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_EXTRAS_MENU);
          thisCrvPt = 0;
        }
      }
      break;

    case CONTEXT_MENU_CUSTOM_CURVE:
      {
        enum {
          ITEM_COPY_CURVE,
          ITEM_RENAME_CURVE,
          ITEM_RESET_CURVE,
          ITEM_FLIP_HORIZONTAL,
          ITEM_FLIP_VERTICAL,
          ITEM_INSERT_POINT,
          ITEM_DELETE_POINT
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Copy curve to"), ITEM_COPY_CURVE);
        contextMenuAddItem(PSTR("Rename curve"), ITEM_RENAME_CURVE);
        contextMenuAddItem(PSTR("Reset curve"), ITEM_RESET_CURVE);
        contextMenuAddItem(PSTR("Flip horizontal"), ITEM_FLIP_HORIZONTAL);
        contextMenuAddItem(PSTR("Flip vertical"), ITEM_FLIP_VERTICAL);
        contextMenuAddItem(PSTR("Insert point"), ITEM_INSERT_POINT);
        contextMenuAddItem(PSTR("Delete point"), ITEM_DELETE_POINT);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_COPY_CURVE)
        {
          destCrvIdx = thisCrvIdx;
          changeToScreen(DIALOG_COPY_CUSTOM_CURVE);
        }
        if(contextMenuSelectedItemID == ITEM_RENAME_CURVE)
          changeToScreen(DIALOG_RENAME_CUSTOM_CURVE);
        if(contextMenuSelectedItemID == ITEM_RESET_CURVE)
        {
          resetCustomCurveParams(thisCrvIdx);
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        if(contextMenuSelectedItemID == ITEM_FLIP_HORIZONTAL)
        {
          //Reflect about y axis.
          //If say a point M(h,k), then M'(-h,k). This however affects the order of the points so 
          //we have to counter this by reversing the arrays.
          custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx];
          for(uint8_t i = 0; i < crv->numPoints; i++)
            crv->xVal[i] = 0 - crv->xVal[i];
          //reverse the arrays
          uint8_t start = 0;
          uint8_t end = crv->numPoints - 1;
          while(start < end)
          {
            int8_t tmp = crv->xVal[start];
            crv->xVal[start] = crv->xVal[end];
            crv->xVal[end] = tmp;
            tmp = crv->yVal[start];
            crv->yVal[start] = crv->yVal[end];
            crv->yVal[end] = tmp;
            start++;
            end--;
          }
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        if(contextMenuSelectedItemID == ITEM_FLIP_VERTICAL)
        {
          //Reflect about x axis.
          //If say a point M(h,k), then M'(h,-k)
          custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx];
          for(uint8_t i = 0; i < crv->numPoints; i++)
            crv->yVal[i] = 0 - crv->yVal[i];
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        if(contextMenuSelectedItemID == ITEM_INSERT_POINT)
        {
          if(Model.CustomCurve[thisCrvIdx].numPoints == MAX_NUM_POINTS_CUSTOM_CURVE)
          {
            makeToast(PSTR("Maximum pts reached"), 2000, 0);
            changeToScreen(SCREEN_CUSTOM_CURVES);
          }
          else 
            changeToScreen(DIALOG_INSERT_CURVE_POINT);
        }
        if(contextMenuSelectedItemID == ITEM_DELETE_POINT)
        {
          if(Model.CustomCurve[thisCrvIdx].numPoints == MIN_NUM_POINTS_CUSTOM_CURVE)
          {
            makeToast(PSTR("Minimum pts reached"), 2000, 0);
            changeToScreen(SCREEN_CUSTOM_CURVES);
          }
          else 
            changeToScreen(DIALOG_DELETE_CURVE_POINT);
        }
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_CUSTOM_CURVES);
      }
      break;
      
    case DIALOG_RENAME_CUSTOM_CURVE:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Curve name"), Model.CustomCurve[thisCrvIdx].name, sizeof(Model.CustomCurve[0].name), 
                       true, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_CUSTOM_CURVES);
      }
      break;
      
    case DIALOG_COPY_CUSTOM_CURVE:
      {
        isEditMode = true;
        destCrvIdx = incDec(destCrvIdx, 0, NUM_CUSTOM_CURVES - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Curve"), thisCrvIdx, destCrvIdx, true);
        if(clickedButton == KEY_SELECT)
        {
          Model.CustomCurve[destCrvIdx] = Model.CustomCurve[thisCrvIdx]; //copy struct
          thisCrvIdx = destCrvIdx;
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_CUSTOM_CURVES);
      }
      break;
      
    case DIALOG_INSERT_CURVE_POINT:
      {
        custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx];
        
        static bool initialised = false;
        static uint8_t insertionPt = 1; //Point b
        if(!initialised)
        {
          initialised = true;
          if(thisCrvPt >= 1 && thisCrvPt <= crv->numPoints - 1)
            insertionPt = thisCrvPt;  
          else
            insertionPt = 1; 
        }
        
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Insert point"));
        display.setCursor(17, 28);
        display.print(F("before:"));
        display.setCursor(71, 28);
        display.write(97 + insertionPt);
        drawCursor(63, 28);
        
        isEditMode = true;
        insertionPt = incDec(insertionPt, 1, crv->numPoints - 1, INCDEC_WRAP, INCDEC_SLOW);
        
        if(clickedButton == KEY_SELECT)
        {
          //calculate x and y values for the new point
          //For non-smooth curve, it is simply the midpoint. If the curve has smoothing enabled,
          //use cubic interpolation to get the new value for y, as long as the xVal are different.
          int8_t xNew = ((int16_t)crv->xVal[insertionPt - 1] + crv->xVal[insertionPt]) / 2;
          int8_t yNew = ((int16_t)crv->yVal[insertionPt - 1] + crv->yVal[insertionPt]) / 2;
          if(crv->smooth && (crv->xVal[insertionPt - 1] != crv->xVal[insertionPt]))
          {
            int16_t xQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
            int16_t yQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
            for(uint8_t pt = 0; pt < crv->numPoints; pt++)
            {
              xQQ[pt] = 5 * crv->xVal[pt];
              yQQ[pt] = 5 * crv->yVal[pt];
            }
            yNew = cubicHermiteInterpolate(xQQ, yQQ, crv->numPoints, xNew * 5) / 5;
          }
          //shift array elements right and write the new value at the insertionPt
          uint8_t i = MAX_NUM_POINTS_CUSTOM_CURVE - 1;
          while(i > insertionPt)
          {
            crv->xVal[i] = crv->xVal[i-1];
            crv->yVal[i] = crv->yVal[i-1];
            i--;
          }
          crv->xVal[insertionPt] = xNew;
          crv->yVal[insertionPt] = yNew;
          //update point count
          crv->numPoints += 1;
          //exit
          initialised = false;
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        
        if(heldButton == KEY_SELECT) //exit
        {
          initialised = false;
          changeToScreen(SCREEN_CUSTOM_CURVES);
        }
      }
      break;
      
    case DIALOG_DELETE_CURVE_POINT:
      {
        custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx];
        
        static bool initialised = false;
        static uint8_t deletePt = 1; //Point b
        if(!initialised)
        {
          initialised = true;
          if(thisCrvPt >= 1 && thisCrvPt <= crv->numPoints - 2)
            deletePt = thisCrvPt; 
          else
            deletePt = 1;
        }

        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Delete point"));
        display.setCursor(17, 28);
        display.print(F("Point:"));
        display.setCursor(71, 28);
        display.write(97 + deletePt);
        drawCursor(63, 28);
        
        //Only allow 'deleting' of interior points. We don't technically delete but simply
        //move the element to the end and decrement numPoints by one.
        
        isEditMode = true;
        deletePt = incDec(deletePt, 1, crv->numPoints - 2, INCDEC_WRAP, INCDEC_SLOW);
        
        if(clickedButton == KEY_SELECT)
        {
          //shift all elements that are after the deletionIdx to the left of the deletion Idx
          for(uint8_t i = deletePt; i < MAX_NUM_POINTS_CUSTOM_CURVE - 1; i++) 
          {
            crv->xVal[i] = crv->xVal[i+1];
            crv->yVal[i] = crv->yVal[i+1];
          }
          //update point count
          crv->numPoints -= 1;
          //exit
          initialised = false;
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        
        if(heldButton == KEY_SELECT) //exit
        {
          initialised = false;
          changeToScreen(SCREEN_CUSTOM_CURVES);
        }
      }
      break;

    case SCREEN_LOGICAL_SWITCHES:
      {
        display.setInterlace(false); 
        drawHeader(extrasMenu[EXTRAS_MENU_LOGICAL_SWITCHES]);

        logical_switch_t *ls = &Model.LogicalSwitch[thisLsIdx];
        
        //draw title
        display.setCursor(8, 9);
        getSrcName(textBuff, SRC_SW_LOGICAL_FIRST + thisLsIdx, sizeof(textBuff));
        display.print(textBuff);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //draw switch icon
        if(checkSwitchCondition(CTRL_SW_LOGICAL_FIRST + thisLsIdx))
          display.drawBitmap(115, 9, icon_switch_is_on, 13, 8, BLACK);
        else
          display.drawBitmap(115, 9, icon_switch_is_off, 13, 8, BLACK);
        
        //-- dynamic scrollable list
        
        enum {
          ITEM_LS_FUNC,
          ITEM_LS_VAL1,
          ITEM_LS_VAL2,
          ITEM_LS_VAL3,
          ITEM_LS_VAL4,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(i == ITEM_LS_VAL4 && (ls->func == LS_FUNC_LATCH || ls->func == LS_FUNC_TOGGLE || ls->func == LS_FUNC_PULSE))
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        uint8_t numFocusable = listItemCount + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        
        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(52, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_LS_FUNC:
              {
                display.print(F("Functn:"));
                display.setCursor(60, ypos);
                if(ls->func == LS_FUNC_NONE)
                  display.print(F("--"));
                else if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
                  display.print(F("|\xEB|>x"));
                else
                  display.print(findStringInIdStr(enum_LogicalSwitch_Func, ls->func));
                
                if(edit)
                {
                  uint8_t oldGroup = getLSFuncGroup(ls->func);
                  ls->func = incDec(ls->func, 0, LS_FUNC_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  uint8_t newGroup = getLSFuncGroup(ls->func);
                  if(newGroup != oldGroup) //reset values if group changed
                  {
                    ls->val3 = 0;
                    ls->val4 = 0;
                    if(ls->func <= LS_FUNC_GROUP3_LAST)     
                    {
                      ls->val1 = SRC_NONE; 
                      ls->val2 = 0; 
                      if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X) 
                        ls->val3 = 2;
                    }
                    else if(ls->func <= LS_FUNC_GROUP4_LAST)
                    {
                      ls->val1 = SRC_NONE; 
                      ls->val2 = SRC_NONE;
                    }
                    else if(ls->func <= LS_FUNC_GROUP5_LAST)
                    {
                      ls->val1 = CTRL_SW_NONE; 
                      ls->val2 = CTRL_SW_NONE;
                    }
                    else if(ls->func == LS_FUNC_TOGGLE) 
                    {
                      ls->val1 = CTRL_SW_NONE; 
                      ls->val2 = 0; 
                      ls->val3 = CTRL_SW_NONE;
                    }
                    else if(ls->func == LS_FUNC_PULSE)
                    {
                      ls->val1 = 5; 
                      ls->val2 = 10;
                    }
                  }
                }
              }
              break;
              
            case ITEM_LS_VAL1:
              {
                if(ls->func == LS_FUNC_PULSE) display.print(F("Width:"));
                else if(ls->func == LS_FUNC_LATCH) display.print(F("Set:"));
                else if(ls->func == LS_FUNC_TOGGLE) display.print(F("Clock:"));
                else display.print(F("Value1:"));
                
                display.setCursor(60, ypos);
                
                if(ls->func == LS_FUNC_NONE)
                  display.print(F("--"));
                else if(ls->func <= LS_FUNC_GROUP4_LAST)
                {
                  if(ls->val1 == SRC_NONE)
                    display.print(F("--"));
                  else
                  {
                    getSrcName(textBuff, ls->val1, sizeof(textBuff));
                    display.print(textBuff);
                  }
                }
                else if(ls->func <= LS_FUNC_GROUP5_LAST || ls->func == LS_FUNC_TOGGLE)
                {
                  getControlSwitchName(textBuff, ls->val1, sizeof(textBuff));
                  display.print(textBuff);
                }
                else if(ls->func == LS_FUNC_PULSE)
                  printSeconds(ls->val1);
                
                if(edit && ls->func != LS_FUNC_NONE)
                {
                  if(ls->func <= LS_FUNC_GROUP3_LAST)
                  {
                    //auto detect moved source
                    uint8_t movedSrc = procMovedSource(getMovedSource());
                    if(movedSrc != SRC_NONE)
                      ls->val1 = movedSrc;
                    //inc dec
                    ls->val1 = incDecSource(ls->val1, INCDEC_FLAG_MIX_SRC 
                                                    | INCDEC_FLAG_COUNTER_AS_SRC 
                                                    | INCDEC_FLAG_TIMER_AS_SRC
                                                    | INCDEC_FLAG_TELEM_AS_SRC 
                                                    | INCDEC_FLAG_INACTIVITY_TIMER_AS_SRC 
                                                    | INCDEC_FLAG_TX_BATTV_AS_SRC);
                    //reset ls->val2 if out of range
                    if(ls->val1 < MIX_SOURCES_COUNT)
                    {
                      if(ls->val2 > 100 || ls->val2 < -100)
                        ls->val2 = 0;
                    }
                    else if(ls->val1 >= SRC_COUNTER_FIRST && ls->val1 <= SRC_COUNTER_LAST)
                    {
                      if(ls->val2 > 9999 || ls->val2 < 0)
                        ls->val2 = 0;
                    }
                    else if(ls->val1 == SRC_INACTIVITY_TIMER || ls->val1 == SRC_TX_BATTERY_VOLTAGE)
                    {
                      if(ls->val2 < 0)
                        ls->val2 = 0;
                    }
                  }
                  else if(ls->func <= LS_FUNC_GROUP4_LAST)
                  {
                    //auto detect moved source
                    uint8_t movedSrc = procMovedSource(getMovedSource());
                    if(movedSrc != SRC_NONE)
                      ls->val1 = movedSrc;
                    //inc dec
                    ls->val1 = incDecSource(ls->val1, INCDEC_FLAG_MIX_SRC);
                  }
                  else if(ls->func <= LS_FUNC_GROUP5_LAST || ls->func == LS_FUNC_TOGGLE)
                  {
                    if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
                      ls->val1 = incDecControlSwitch(ls->val1, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW | INCDEC_FLAG_FMODE_AS_SW | INCDEC_FLAG_TRIM_AS_SW);
                    else
                      ls->val1 = incDecControlSwitch(ls->val1, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
                  }
                  else if(ls->func == LS_FUNC_PULSE)
                    ls->val1 = incDec(ls->val1, 1, ls->val2 - 1, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
            
            case ITEM_LS_VAL2:
              {
                if(ls->func == LS_FUNC_PULSE) display.print(F("Period:"));
                else if(ls->func == LS_FUNC_LATCH) display.print(F("Reset:"));
                else if(ls->func == LS_FUNC_TOGGLE) display.print(F("Edge:"));
                else display.print(F("Value2:"));
                
                display.setCursor(60, ypos);
                if(ls->func == LS_FUNC_NONE)
                  display.print(F("--"));
                else if(ls->func <= LS_FUNC_GROUP3_LAST)
                {
                  if(ls->val1 < MIX_SOURCES_COUNT)
                    display.print(ls->val2);
                  else if(ls->val1 >= SRC_COUNTER_FIRST && ls->val1 <= SRC_COUNTER_LAST)
                    display.print(ls->val2);
                  else if(ls->val1 >= SRC_TIMER_FIRST && ls->val1 <= SRC_TIMER_LAST)
                    printHHMMSS((int32_t)ls->val2 * 1000);
                  else if(ls->val1 >= SRC_TELEMETRY_FIRST && ls->val1 <= SRC_TELEMETRY_LAST)
                  {
                    display.setCursor(display.getCursorX(), display.getCursorY());
                    printTelemParam(ls->val1 - SRC_TELEMETRY_FIRST, ls->val2, true);
                  }
                  else if(ls->val1 == SRC_INACTIVITY_TIMER)
                    printHHMMSS((int32_t) ls->val2 * 1000);
                  else if(ls->val1 == SRC_TX_BATTERY_VOLTAGE)
                    printVoltage(ls->val2);
                }
                else if(ls->func <= LS_FUNC_GROUP4_LAST)
                {
                  if(ls->val2 == SRC_NONE)
                    display.print(F("--"));
                  else
                  {
                    getSrcName(textBuff, ls->val2, sizeof(textBuff));
                    display.print(textBuff);
                  }
                }
                else if(ls->func <= LS_FUNC_GROUP5_LAST)
                {
                  getControlSwitchName(textBuff, ls->val2, sizeof(textBuff));
                  display.print(textBuff);
                }
                else if(ls->func == LS_FUNC_TOGGLE)
                  display.print(findStringInIdStr(enum_ClockEdge, ls->val2));
                else if(ls->func == LS_FUNC_PULSE)
                  printSeconds(ls->val2);

                if(edit && ls->func != LS_FUNC_NONE)
                {
                  if(ls->func <= LS_FUNC_GROUP3_LAST)
                  {
                    if(ls->val1 < MIX_SOURCES_COUNT)
                      ls->val2 = incDec(ls->val2, (ls->func <= LS_FUNC_GROUP1_LAST) ? -100 : 0, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
                    else if(ls->val1 >= SRC_COUNTER_FIRST && ls->val1 <= SRC_COUNTER_LAST)
                      ls->val2 = incDec(ls->val2, 0, 9999, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                    else if(ls->val1 >= SRC_TIMER_FIRST && ls->val1 <= SRC_TIMER_LAST)
                      ls->val2 = incDec(ls->val2, (ls->func <= LS_FUNC_GROUP1_LAST) ? -30000 : 0, 30000, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_FAST);
                    else if(ls->val1 >= SRC_TELEMETRY_FIRST && ls->val1 <= SRC_TELEMETRY_LAST)
                      ls->val2 = incDec(ls->val2, (ls->func <= LS_FUNC_GROUP1_LAST) ? -30000 : 0, 30000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                    else if(ls->val1 == SRC_INACTIVITY_TIMER)
                      ls->val2 = incDec(ls->val2, 0, 30000, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_FAST);
                    else if(ls->val1 == SRC_TX_BATTERY_VOLTAGE)
                    {
                      ls->val2 /= 10;
                      ls->val2 = incDec(ls->val2, 0, 3000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                      ls->val2 *= 10;
                    }
                  }
                  else if(ls->func <= LS_FUNC_GROUP4_LAST)
                  {
                    //auto detect moved source
                    uint8_t movedSrc = procMovedSource(getMovedSource());
                    if(movedSrc != SRC_NONE)
                      ls->val2 = movedSrc;
                    //inc dec
                    ls->val2 = incDecSource(ls->val2, INCDEC_FLAG_MIX_SRC);
                  }
                  else if(ls->func <= LS_FUNC_GROUP5_LAST)
                  {
                    if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
                      ls->val2 = incDecControlSwitch(ls->val2, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW | INCDEC_FLAG_FMODE_AS_SW | INCDEC_FLAG_TRIM_AS_SW);
                    else
                      ls->val2 = incDecControlSwitch(ls->val2, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
                  }
                  else if(ls->func == LS_FUNC_TOGGLE) //change edge type
                    ls->val2 = incDec(ls->val2, 0, 2, INCDEC_WRAP, INCDEC_SLOW);
                  else if(ls->func == LS_FUNC_PULSE) //adjust period
                    ls->val2 = incDec(ls->val2, ls->val1 + 1, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
              
            case ITEM_LS_VAL3:
              {
                if(ls->func == LS_FUNC_TOGGLE)
                {
                  display.print(F("Clear:"));
                  display.setCursor(60, ypos);
                  getControlSwitchName(textBuff, ls->val3, sizeof(textBuff));
                  display.print(textBuff);
                }
                else if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
                {
                  display.print(F("Dirctn:"));
                  display.setCursor(60, ypos);
                  display.print(findStringInIdStr(enum_DirectionOfChange, ls->val3));
                }
                else
                {
                  display.print(F("Delay:"));
                  display.setCursor(60, ypos);
                  if(ls->val3 == 0 || ls->func == LS_FUNC_NONE)
                    display.print(F("--"));
                  else
                    printSeconds(ls->val3);
                }

                if(edit && ls->func != LS_FUNC_NONE)
                {
                  if(ls->func == LS_FUNC_TOGGLE)
                  {
                    if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
                      ls->val3 = incDecControlSwitch(ls->val3, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW | INCDEC_FLAG_FMODE_AS_SW | INCDEC_FLAG_TRIM_AS_SW);
                    else
                      ls->val3 = incDecControlSwitch(ls->val3, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
                  }
                  else if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
                    ls->val3 = incDec(ls->val3, 0, 2, INCDEC_WRAP, INCDEC_SLOW);
                  else //adjust delay
                    ls->val3 = incDec(ls->val3, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
              
            case ITEM_LS_VAL4:
              {
                display.print(F("Duratn:"));
                display.setCursor(60, ypos);
                if(ls->val4 == 0 || ls->func == LS_FUNC_NONE)
                  display.print(F("--"));
                else
                  printSeconds(ls->val4);
                
                if(edit && ls->func != LS_FUNC_NONE)
                {
                  ls->val4 = incDec(ls->val4, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);

        //open context menu
        if(focusedItem == numFocusable && isEditMode)
          changeToScreen(CONTEXT_MENU_LOGICAL_SWITCHES);
        
        //change to next logical switch
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisLsIdx = incDec(thisLsIdx, 0, NUM_LOGICAL_SWITCHES - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        // Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
      
    case CONTEXT_MENU_LOGICAL_SWITCHES:
      {
        enum {
          ITEM_VIEW_OUTPUTS,
          ITEM_COPY_TO,
          ITEM_MOVE_TO,
          ITEM_RESET_SETTINGS
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_VIEW_OUTPUTS);
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_TO);
        contextMenuAddItem(PSTR("Move to"), ITEM_MOVE_TO);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_VIEW_OUTPUTS)
          changeToScreen(SCREEN_LOGICAL_SWITCH_OUTPUTS);
        if(contextMenuSelectedItemID == ITEM_COPY_TO)
        {
          destLsIdx = thisLsIdx;
          changeToScreen(DIALOG_COPY_LOGICAL_SWITCH);
        }
        if(contextMenuSelectedItemID == ITEM_MOVE_TO)
        {
          destLsIdx = thisLsIdx;
          changeToScreen(DIALOG_MOVE_LOGICAL_SWITCH);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetLogicalSwitchParams(thisLsIdx);
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
      }
      break;
    
    case DIALOG_COPY_LOGICAL_SWITCH:
    case DIALOG_MOVE_LOGICAL_SWITCH:
      {
        isEditMode = true;
        destLsIdx = incDec(destLsIdx, 0, NUM_LOGICAL_SWITCHES - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("L"), thisLsIdx, destLsIdx, theScreen == DIALOG_COPY_LOGICAL_SWITCH);
        if(clickedButton == KEY_SELECT)
        {
          if(theScreen == DIALOG_COPY_LOGICAL_SWITCH)
          {
            Model.LogicalSwitch[destLsIdx] = Model.LogicalSwitch[thisLsIdx];
            thisLsIdx = destLsIdx;
          }
          else
          {
            if(moveLogicalSwitch(destLsIdx, thisLsIdx))
              thisLsIdx = destLsIdx;
            else
              makeToast(PSTR("Moving failed"), 2000, 0);
          }
          
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
      }
      break;

    case SCREEN_LOGICAL_SWITCH_OUTPUTS:
      {
        display.setInterlace(false); 
        drawHeader(PSTR("Logical sw outputs"));
        
        //scrollable 
        uint8_t numPages = (NUM_LOGICAL_SWITCHES + 9) / 10;
        static uint8_t thisPage = 1;
        
        static bool viewInitialised = false;
        if(!viewInitialised) 
        {
          //start in the page that has the switch we want to view
          thisPage = (thisLsIdx + 10) / 10;
          viewInitialised = true;
        }

        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        uint8_t startIdx = (thisPage - 1) * 10;
        for(uint8_t i = startIdx; i < startIdx + 10 && i < NUM_LOGICAL_SWITCHES; i++)
        {
          if((i - startIdx) < 5)
          {
            if(i == thisLsIdx)
            {
              display.setCursor(0, 10 + (i - startIdx) * 11);
              display.write(0xB1);
            }
            display.setCursor(6, 10 + (i - startIdx) * 11);
          }
          else
          { 
            if(i == thisLsIdx)
            {
              display.setCursor(65, 10 + (i - (startIdx + 5)) * 11);
              display.write(0xB1);
            }        
            display.setCursor(71, 10 + (i - (startIdx + 5)) * 11);
          }
          display.print(F("L"));          
          display.print(1 + i);  
          display.print(F(":"));
          if(i < 9)
            display.print(F(" "));

          if(checkSwitchCondition(CTRL_SW_LOGICAL_FIRST + i))
            display.drawBitmap(display.getCursorX(), display.getCursorY(), icon_switch_is_on, 13, 8, BLACK);
          else
            display.drawBitmap(display.getCursorX(), display.getCursorY(), icon_switch_is_off, 13, 8, BLACK);
        }
        //show scrollbar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
        }
      }
      break;
      
    case SCREEN_FUNCTION_GENERATORS:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_FUNCTION_GENERATORS]);
        
        funcgen_t *fgen = &Model.Funcgen[thisFgenIdx];
        
        if(fgen->waveform == FUNCGEN_WAVEFORM_PULSE)
        {
          if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_FIXED && fgen->width >= fgen->period)
          {
            if(fgen->width >= 600)
              fgen->width = 599;
            fgen->period = fgen->width + 1;
          }
        }
        else
        {
          if(fgen->periodMode == FUNCGEN_PERIODMODE_VARIABLE && fgen->period1 >= fgen->period2)
          {
            if(fgen->period1 >= 600)
              fgen->period1 = 599;
            fgen->period2 = fgen->period1 + 1;
          }
        }
        
        display.setCursor(8, 9);
        getSrcName(textBuff, SRC_FUNCGEN_FIRST + thisFgenIdx, sizeof(textBuff));
        display.print(textBuff);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //--- dynamic scrollable list 
        
        enum {
          ITEM_WAVEFORM,
          ITEM_PERIOD_MODE,
          ITEM_PERIOD1,
          ITEM_PERIOD2,
          ITEM_WIDTH_MODE,
          ITEM_MODULATOR_SRC,
          ITEM_REVERSE_MODULATOR,
          ITEM_WIDTH,
          ITEM_PERIOD, 
          ITEM_PHASE_MODE,
          ITEM_PHASE,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(fgen->waveform == FUNCGEN_WAVEFORM_PULSE)
          {
            if(i == ITEM_PERIOD_MODE || i == ITEM_PERIOD1 || i == ITEM_PERIOD2)
              continue;
            if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_FIXED)
            {
              if(i == ITEM_MODULATOR_SRC || i == ITEM_REVERSE_MODULATOR)
                continue;
            }
            if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_VARIABLE)
            {
              if(i == ITEM_WIDTH)
                continue;
            }
          }
          else
          {
            if(i == ITEM_WIDTH_MODE || i == ITEM_WIDTH || i == ITEM_PERIOD)
              continue;
            if(fgen->periodMode == FUNCGEN_PERIODMODE_FIXED)
            {
              if(i == ITEM_PERIOD2 || i == ITEM_MODULATOR_SRC || i == ITEM_REVERSE_MODULATOR)
                continue;
            }
            if(fgen->periodMode == FUNCGEN_PERIODMODE_VARIABLE /*|| fgen->waveform == FUNCGEN_WAVEFORM_RANDOM*/)
            {
              if(i == ITEM_PHASE_MODE || i == ITEM_PHASE)
                continue;
            }
          }

          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        uint8_t numFocusable = listItemCount + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        
        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(70, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_WAVEFORM:
              {
                display.print(F("Waveform:"));
                display.setCursor(78, ypos);
                display.print(findStringInIdStr(enum_FuncgenWaveform, fgen->waveform));
                if(edit)
                {
                  fgen->waveform = incDec(fgen->waveform, 0, FUNCGEN_WAVEFORM_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
              }
              break;
              
            case ITEM_PERIOD_MODE:
              {
                if(fgen->waveform == FUNCGEN_WAVEFORM_RANDOM) 
                  display.print(F("Intrvl mode:"));
                else 
                  display.print(F("Period mode:"));
                display.setCursor(78, ypos);
                display.print(findStringInIdStr(enum_FuncgenPeriodMode, fgen->periodMode));
                if(edit)
                  fgen->periodMode = incDec(fgen->periodMode, 0, FUNCGEN_PERIODMODE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;

            case ITEM_PERIOD1:
              {
                if(fgen->waveform == FUNCGEN_WAVEFORM_RANDOM)
                {
                  if(fgen->periodMode == FUNCGEN_PERIODMODE_FIXED)
                    display.print(F("Interval:"));
                  else
                    display.print(F("Min intrvl:"));
                }
                else
                {
                  if(fgen->periodMode == FUNCGEN_PERIODMODE_FIXED)
                    display.print(F("Period:"));
                  else
                    display.print(F("Min period:"));
                }
                display.setCursor(78, ypos);
                printSeconds(fgen->period1);
                if(edit)
                {
                  uint16_t max = 600;
                  if(fgen->periodMode == FUNCGEN_PERIODMODE_VARIABLE)
                    max = fgen->period2 - 1;
                  fgen->period1 = incDec(fgen->period1, 1, max, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
              
            case ITEM_PERIOD2:
              {
                if(fgen->waveform == FUNCGEN_WAVEFORM_RANDOM)
                  display.print(F("Max intrvl:"));
                else
                  display.print(F("Max period:"));
                display.setCursor(78, ypos);
                printSeconds(fgen->period2);
                if(edit)
                  fgen->period2 = incDec(fgen->period2, fgen->period1 + 1, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;

            case ITEM_WIDTH_MODE:
              {
                display.print(F("Width mode:"));
                display.setCursor(78, ypos);
                display.print(findStringInIdStr(enum_FuncgenWidthMode, fgen->widthMode));
                if(edit)
                  fgen->widthMode = incDec(fgen->widthMode, 0, FUNCGEN_PULSE_WIDTH_MODE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_WIDTH:
              {
                display.print(F("Width:"));
                display.setCursor(78, ypos);
                printSeconds(fgen->width);
                if(edit)
                  fgen->width = incDec(fgen->width, 1, fgen->period - 1, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
            
            case ITEM_PERIOD:
              {
                display.print(F("Period:"));
                display.setCursor(78, ypos);
                printSeconds(fgen->period);
                if(edit)
                {
                  uint16_t min = 1;
                  if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_FIXED)
                    min = fgen->width + 1;
                  fgen->period = incDec(fgen->period, min, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
            
            case ITEM_MODULATOR_SRC:
              {
                display.print(F("Modulator:"));
                display.setCursor(78, ypos);
                getSrcName(textBuff, fgen->modulatorSrc, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                {
                  uint8_t movedSrc = procMovedSource(getMovedSource());
                  if(movedSrc != SRC_NONE)
                    fgen->modulatorSrc = movedSrc;
                  //inc dec
                  fgen->modulatorSrc = incDecSource(fgen->modulatorSrc, INCDEC_FLAG_MIX_SRC);
                }
              }
              break;
              
            case ITEM_REVERSE_MODULATOR:
              {
                display.print(F("Reverse:"));
                drawCheckbox(78, ypos, fgen->reverseModulator);
                if(edit)
                  fgen->reverseModulator = incDec(fgen->reverseModulator, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_PHASE_MODE:
              {
                display.print(F("Phase mode:"));
                display.setCursor(78, ypos);
                display.print(findStringInIdStr(enum_FuncgenPhaseMode, fgen->phaseMode));
                if(edit)
                  fgen->phaseMode = incDec(fgen->phaseMode, 0, FUNCGEN_PHASEMODE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_PHASE:
              {
                display.print(F("Phase:"));
                display.setCursor(78, ypos);
                if(fgen->phaseMode == FUNCGEN_PHASEMODE_AUTO)
                  display.print(F("[Sync]"));
                else if(fgen->phaseMode == FUNCGEN_PHASEMODE_FIXED)
                {
                  display.print(fgen->phase);
                  display.write(248); //degree symbol
                }
                
                if(edit)
                {
                  if(fgen->phaseMode == FUNCGEN_PHASEMODE_AUTO)
                  {
                    syncWaveform(thisFgenIdx);
                    makeToast(PSTR("Synced"), 2000, 0);
                    isEditMode = false;
                  }
                  else if(fgen->phaseMode == FUNCGEN_PHASEMODE_FIXED)
                    fgen->phase = incDec(fgen->phase, 0, 360, INCDEC_NOWRAP, INCDEC_NORMAL);
                }
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);

        //open context menu
        if(focusedItem == numFocusable && isEditMode)
          changeToScreen(CONTEXT_MENU_FUNCGEN);
        
        //change to next function generator
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisFgenIdx = incDec(thisFgenIdx, 0, NUM_FUNCGEN - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
      
    case CONTEXT_MENU_FUNCGEN:
      {
        enum {
          ITEM_VIEW_OUTPUTS,
          ITEM_COPY_FUNCGEN,
          ITEM_RESET_FUNCGEN,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_VIEW_OUTPUTS);
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_FUNCGEN);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_FUNCGEN);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RESET_FUNCGEN)
        {
          resetFuncgenParams(thisFgenIdx);
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
        }
        if(contextMenuSelectedItemID == ITEM_COPY_FUNCGEN)
        {
          destFgenIdx = thisFgenIdx;
          changeToScreen(DIALOG_COPY_FUNCGEN);
        }
        if(contextMenuSelectedItemID == ITEM_VIEW_OUTPUTS)
          changeToScreen(SCREEN_FUNCGEN_OUTPUTS);

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
      }
      break;
      
    case DIALOG_COPY_FUNCGEN:
      {
        isEditMode = true;
        destFgenIdx = incDec(destFgenIdx, 0, NUM_FUNCGEN - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Fgen"), thisFgenIdx, destFgenIdx, true);
        if(clickedButton == KEY_SELECT)
        {
          Model.Funcgen[destFgenIdx] = Model.Funcgen[thisFgenIdx];
          thisFgenIdx = destFgenIdx;
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
      }
      break;
      
    case SCREEN_FUNCGEN_OUTPUTS:
      {
        display.setInterlace(false); 
        drawHeader(PSTR("Funcgen outputs"));

        //--- scrollable 
        
        uint8_t numPages = (NUM_FUNCGEN + 4) / 5;
        static uint8_t thisPage = 1;
        
        static bool viewInitialised = false;
        if(!viewInitialised) 
        {
          thisPage = (thisFgenIdx + 5) / 5; //start in the page with the function generator we want to view
          viewInitialised = true;
        }

        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        uint8_t startIdx = (thisPage - 1) * 5;
        for(uint8_t i = startIdx; i < startIdx + 5 && i < NUM_FUNCGEN; i++)
        {
          uint8_t ypos = 10 + (i - startIdx) * 11;
          //show marker
          if(i == thisFgenIdx) 
          {
            display.setCursor(0, ypos);
            display.write(0xB1);
          }
          //show name
          display.setCursor(7, ypos);
          getSrcName(textBuff, SRC_FUNCGEN_FIRST + i, sizeof(textBuff));
          display.print(textBuff);
          display.print(F(":"));
          //draw graph
          drawHorizontalBarChartZeroCentered(45, ypos + 2, 39, 3, BLACK, mixSources[SRC_FUNCGEN_FIRST + i]/5, 200);
          //show value
          display.setCursor(94, ypos);
          display.print(mixSources[SRC_FUNCGEN_FIRST + i]/5);
        }
        
        //show scrollbar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        if(heldButton == KEY_SELECT) //exit
        {
          viewInitialised = false;
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
        }
      }
      break;

    case SCREEN_COUNTERS:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_COUNTERS]);
        
        counter_params_t *counter = &Model.Counter[thisCounterIdx];
        
        display.setCursor(8, 9);
        display.print(F("Counter"));
        display.print(thisCounterIdx + 1);
        if(!isEmptyStr(counter->name, sizeof(counter->name)))
        {
          display.setCursor(display.getCursorX() + 6, 9);
          display.print(counter->name);
        }
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //show the value of the counter. Right align.
        //using a hack to measure the width of the text before actually printing.
        display.setCursor(0, 64);
        display.print(counterOut[thisCounterIdx]);
        uint8_t len_px = display.getCursorX();
        //now print on screen
        uint8_t xpos = 127 - len_px;
        display.setCursor(xpos, 9);
        display.print(counterOut[thisCounterIdx]);
        display.drawRect(xpos - 2, 7, len_px + 3, 11, BLACK);
        
        //--- scrollable list 
  
        enum {
          ITEM_CLOCK,
          ITEM_EDGE,
          ITEM_MODULUS,
          ITEM_DIRECTION,
          ITEM_CLEAR,
          ITEM_PERSISTENT,
          
          ITEM_COUNT
        };

        //handle navigation
        uint8_t numFocusable = ITEM_COUNT + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        
        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < ITEM_COUNT; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(58, ypos);
          
          if((topItem - 1 + line) >= ITEM_COUNT)
            break;
          
          uint8_t itemID = topItem - 1 + line;
          bool edit = (focusedItem > 1 && focusedItem != numFocusable && itemID == focusedItem - 2 && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_CLOCK:
              {
                display.print(F("Clock:"));
                display.setCursor(66, ypos);
                getControlSwitchName(textBuff, counter->clock, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  counter->clock = incDecControlSwitch(counter->clock, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
              }
              break;
              
            case ITEM_EDGE:
              {
                display.print(F("Edge:"));
                display.setCursor(66, ypos);
                display.print(findStringInIdStr(enum_ClockEdge, counter->edge));
                if(edit)
                  counter->edge = incDec(counter->edge, 0, 2, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
            
            case ITEM_MODULUS:
              {
                display.print(F("Modulus:"));
                display.setCursor(66, ypos);
                display.print(counter->modulus);
                if(edit)
                  counter->modulus = incDec(counter->modulus, 2, 10000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
              }
              break;
            
            case ITEM_DIRECTION:
              {
                display.print(F("Direction:"));
                display.setCursor(66, ypos);
                display.print(findStringInIdStr(enum_CounterDirection, counter->direction));
                if(edit)
                  counter->direction = incDec(counter->direction, 0, 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
            
            case ITEM_CLEAR:
              {
                display.print(F("Clear:"));
                display.setCursor(66, ypos);
                getControlSwitchName(textBuff, counter->clear, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  counter->clear = incDecControlSwitch(counter->clear, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
              }
              break;
              
            case ITEM_PERSISTENT:
              {
                display.print(F("Persist:"));
                drawCheckbox(66, ypos, counter->isPersistent);
                if(edit)
                  counter->isPersistent = incDec(counter->isPersistent, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, ITEM_COUNT, topItem, 5, 5 * 9);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);

        //open context menu
        if(focusedItem == numFocusable && isEditMode)
          changeToScreen(CONTEXT_MENU_COUNTERS);
        
        //change to next counter
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisCounterIdx = incDec(thisCounterIdx, 0, NUM_COUNTERS - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        // Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
      
    case CONTEXT_MENU_COUNTERS:
      {
        enum {
          ITEM_RENAME_COUNTER,
          ITEM_RESET_SETTINGS,
          ITEM_COPY_COUNTER,
          ITEM_CLEAR_COUNTER,
          ITEM_CLEAR_ALL_COUNTERS,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_COUNTER);
        contextMenuAddItem(PSTR("Rename counter"), ITEM_RENAME_COUNTER);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuAddItem(PSTR("Clear counter"), ITEM_CLEAR_COUNTER);
        contextMenuAddItem(PSTR("Clear all"), ITEM_CLEAR_ALL_COUNTERS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetCounterParams(thisCounterIdx);
          resetCounterRegister(thisCounterIdx); //also reset the register
          changeToScreen(SCREEN_COUNTERS);
        }
        if(contextMenuSelectedItemID == ITEM_CLEAR_COUNTER)
        {
          resetCounterRegister(thisCounterIdx);
          changeToScreen(SCREEN_COUNTERS);  
        }
        if(contextMenuSelectedItemID == ITEM_CLEAR_ALL_COUNTERS)
          changeToScreen(CONFIRMATION_CLEAR_ALL_COUNTERS);  
        if(contextMenuSelectedItemID == ITEM_RENAME_COUNTER)
          changeToScreen(DIALOG_RENAME_COUNTER);
        if(contextMenuSelectedItemID == ITEM_COPY_COUNTER)
        {
          destCounterIdx = thisCounterIdx;
          changeToScreen(DIALOG_COPY_COUNTER);  
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_COUNTERS);
      }
      break;
      
    case DIALOG_RENAME_COUNTER:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Counter name"), Model.Counter[thisCounterIdx].name, sizeof(Model.Counter[0].name), 
                       true, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_COUNTERS);
      }
      break;
      
    case DIALOG_COPY_COUNTER:
      {
        isEditMode = true;
        destCounterIdx = incDec(destCounterIdx, 0, NUM_COUNTERS - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Counter"), thisCounterIdx, destCounterIdx, true);
        if(clickedButton == KEY_SELECT)
        {
          Model.Counter[destCounterIdx] = Model.Counter[thisCounterIdx];
          thisCounterIdx = destCounterIdx;
          changeToScreen(SCREEN_COUNTERS);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_COUNTERS);
      }
      break;
      
    case CONFIRMATION_CLEAR_ALL_COUNTERS:
      {
        printFullScreenMessage(PSTR("Clear all counter\nregisters?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          resetCounterRegisters();
          changeToScreen(SCREEN_COUNTERS);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_COUNTERS);
      }
      break;
      
    case SCREEN_TIMERS:
      {
        timer_params_t *tmr = &Model.Timer[thisTimerIdx]; 
        
        if(isEditTextDialog) 
        {
          editTextDialog(PSTR("Timer name"), tmr->name, sizeof(tmr->name), true, true, false);
          break;
        }
        
        drawHeader(extrasMenu[EXTRAS_MENU_TIMERS]);

        static bool viewInitialised = false;
        static uint8_t lastFocusedItem = 1;
        if(!viewInitialised)
        {
          viewInitialised = true;
          lastFocusedItem = 1;
        }

        display.setCursor(8, 9);
        display.print(F("Timer"));
        display.print(thisTimerIdx + 1);
        display.drawHLine(8, 17, display.getCursorX() - 8, BLACK);

        display.setCursor(0, 20);
        display.print(F("Name:"));
        display.setCursor(60, 20);
        if(isEmptyStr(tmr->name, sizeof(tmr->name)))
          display.print(F("--"));
        else
          display.print(tmr->name);

        display.setCursor(0, 29);
        display.print(F("Switch:"));
        display.setCursor(60, 29);
        getControlSwitchName(textBuff, tmr->swtch, sizeof(textBuff));
        display.print(textBuff);
        
        display.setCursor(0, 38);
        display.print(F("Reset:"));
        display.setCursor(60, 38);
        getControlSwitchName(textBuff, tmr->resetSwitch, sizeof(textBuff));
        display.print(textBuff);
        
        display.setCursor(0, 47);
        display.print(F("Initial:"));
        display.setCursor(60, 47);
        if(tmr->initialSeconds == 0)
          display.print(F("--"));
        else
          printHHMMSS((uint32_t) tmr->initialSeconds * 1000);
        
        display.setCursor(0, 56);
        display.print(F("Persist:"));
        drawCheckbox(60, 56, tmr->isPersistent);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == 7 ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);
        
        //Show the value of the timer, right aligned.
        //Here we are employing a hack to measure the width of the text 
        //by intentionally printing off screen and using the difference in the x position.
        display.setCursor(0, 64);
        printTimerValue(thisTimerIdx);
        uint8_t len_px = display.getCursorX();
        //now print on screen
        uint8_t xpos = 127 - len_px;
        display.setCursor(xpos, 9);
        printTimerValue(thisTimerIdx);
        display.drawRect(xpos - 2, 7, len_px + 3, 11, BLACK);
        
        //show cursor
        if(focusedItem == 1)
          drawCursor(0, 9);
        else if(focusedItem <= 6)
          drawCursor(52, 20 + 9 * (focusedItem - 2));
        
        focusedItem = lastFocusedItem;
        changeFocusOnUpDown(7);
        toggleEditModeOnSelectClicked();
        lastFocusedItem = focusedItem;
        
        //edit items
        if(focusedItem == 1)
          thisTimerIdx = incDec(thisTimerIdx, 0, NUM_TIMERS - 1, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 2 && isEditMode)
          isEditTextDialog = true;
        else if(focusedItem == 3 && isEditMode)
          tmr->swtch = incDecControlSwitch(tmr->swtch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
        else if(focusedItem == 4 && isEditMode)
          tmr->resetSwitch = incDecControlSwitch(tmr->resetSwitch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
        else if(focusedItem == 5 && isEditMode)
          changeToScreen(DIALOG_TIMER_INITIAL_TIME);
        else if(focusedItem == 6)
          tmr->isPersistent = incDec(tmr->isPersistent, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 7 && isEditMode)
        {
          changeToScreen(CONTEXT_MENU_TIMERS);
          viewInitialised = false;
        }

        //exit 
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_EXTRAS_MENU);
          viewInitialised = false;
        }
      }
      break;

    case DIALOG_TIMER_INITIAL_TIME:
      {
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Initial time"));

        static bool initialised = false;

        static uint8_t idxQQ = 0;
        static uint8_t valQQ[3];

        if(!initialised)
        {
          initialised = true;

          uint32_t hh, mm, ss;
          ss = Model.Timer[thisTimerIdx].initialSeconds;
          hh = ss / 3600;
          ss = ss - hh * 3600;
          mm = ss / 60;
          ss = ss - mm * 60;

          valQQ[0] = hh;
          valQQ[1] = mm;
          valQQ[2] = ss;
        }

        isEditMode = true;
        valQQ[idxQQ] = incDec(valQQ[idxQQ], 0, (idxQQ == 0) ? 23 : 59, INCDEC_WRAP, INCDEC_SLOW, INCDEC_NORMAL);

        //draw values
        display.setCursor(41, 32);
        for(uint8_t i = 0; i < sizeof(valQQ)/sizeof(valQQ[0]); i++)
        {
          if(i != 0)
            display.print(F(":"));
          if(valQQ[i] < 10)
            display.print(F("0"));
          display.print(valQQ[i]);
        }

        //draw blinking cursor
        if((millis() - buttonReleaseTime) % 1000 < 500 || buttonCode > 0)
          display.fillRect(41 + idxQQ * 18, 40, 11, 2, BLACK);

        //change to next
        if(clickedButton == KEY_SELECT)
          idxQQ++;
        else if(heldButton == KEY_SELECT && idxQQ > 0)
        {
          //move to previous
          idxQQ--;
          killButtonEvents();
        }

        //done, exit
        if(idxQQ == sizeof(valQQ)/sizeof(valQQ[0]))
        {
          idxQQ = 0;
          Model.Timer[thisTimerIdx].initialSeconds = ((uint32_t) 3600 * valQQ[0]) + ((uint32_t) 60 * valQQ[1]) + valQQ[2];
          initialised = false;
          changeToScreen(SCREEN_TIMERS);
        }
      }
      break;
      
    case CONTEXT_MENU_TIMERS:
      {
        enum {
          ITEM_START_STOP_TIMER,
          ITEM_RESET_TIMER,
          ITEM_RESET_SETTINGS,
        };
        
        contextMenuInitialise();
        if(Model.Timer[thisTimerIdx].swtch == CTRL_SW_NONE)
        {
          if(timerIsRunning[thisTimerIdx]) 
            contextMenuAddItem(PSTR("Stop timer"), ITEM_START_STOP_TIMER);
          else 
            contextMenuAddItem(PSTR("Start timer"), ITEM_START_STOP_TIMER);
        }
        contextMenuAddItem(PSTR("Reset timer"), ITEM_RESET_TIMER);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_START_STOP_TIMER)
        {
          timerForceRun[thisTimerIdx] = timerIsRunning[thisTimerIdx] ? false : true;
          changeToScreen(SCREEN_TIMERS);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_TIMER)
        {
          resetTimerRegister(thisTimerIdx);
          changeToScreen(SCREEN_TIMERS);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetTimerParams(thisTimerIdx);
          resetTimerRegister(thisTimerIdx); //also reset the register
          changeToScreen(SCREEN_TIMERS);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TIMERS);
      }
      break;
      
    case SCREEN_SAFETY_CHECKS:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_SAFETY_CHECKS]);
        
        //--- dynamic scrollable list
        enum {
          ITEM_SAFETY_CHECK_THROTTLE,
          ITEM_SAFETY_CHECK_SW_FIRST,
          ITEM_SAFETY_CHECK_SW_LAST = ITEM_SAFETY_CHECK_SW_FIRST + NUM_PHYSICAL_SWITCHES - 1,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {        
          if(i == ITEM_SAFETY_CHECK_THROTTLE && Model.type == MODEL_TYPE_OTHER)
            continue;
          //skip absent switches
          if(i >= ITEM_SAFETY_CHECK_SW_FIRST && i <= ITEM_SAFETY_CHECK_SW_LAST)
          {
            uint8_t sw = i - ITEM_SAFETY_CHECK_SW_FIRST;
            if(Sys.swType[sw] == SW_ABSENT)
              continue; 
          }
          listItemIDs[listItemCount++] = i;
        }
        
        //initialise
        static uint8_t topItem;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }
        
        //handle navigation
        changeFocusOnUpDown(listItemCount);
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
          
          if(itemID == ITEM_SAFETY_CHECK_THROTTLE)
          {
            display.print(F("Throttle:"));
            drawCheckbox(72, ypos, Model.checkThrottle);
            if(edit)
              Model.checkThrottle = incDec(Model.checkThrottle, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
          }
          else if(itemID >= ITEM_SAFETY_CHECK_SW_FIRST && itemID <= ITEM_SAFETY_CHECK_SW_LAST)
          {
            uint8_t sw = itemID - ITEM_SAFETY_CHECK_SW_FIRST;
            display.print(F("Switch "));
            display.write(65 + sw);
            display.print(F(":"));
            display.setCursor(72, ypos);
            display.print(findStringInIdStr(enum_SwitchWarn, Model.switchWarn[sw]));
            if(edit)
            {
              if(Sys.swType[sw] == SW_2POS)
                Model.switchWarn[sw] = incDec(Model.switchWarn[sw], -1, 1, INCDEC_WRAP, INCDEC_SLOW);
              else if(Sys.swType[sw] == SW_3POS)
                Model.switchWarn[sw] = incDec(Model.switchWarn[sw], -1, 2, INCDEC_WRAP, INCDEC_SLOW);
            }
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 9, listItemCount, topItem, 6, 6 * 9);

        // Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
    
    case SCREEN_NOTIFICATION_SETUP:
      {
        notification_params_t *notification = &Model.CustomNotification[thisNotificationIdx]; 
        
        if(isEditTextDialog) 
        {
          editTextDialog(PSTR("Text"), notification->text, sizeof(notification->text), true, true, false);
          break;
        }
        
        drawHeader(extrasMenu[EXTRAS_MENU_NOTIFICATION_SETUP]);
        
        display.setCursor(8, 9);
        display.print(F("Notification"));
        display.print(thisNotificationIdx + 1);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //use a temporary variable for the swtch and assign later 
        //to avoid annoying playback while scrolling through the options
        static uint8_t tempSwtch;
        static bool tempInitialised = false;
        if(!tempInitialised)
        {
          tempSwtch = notification->swtch;
          tempInitialised = true;
        }
        
        //--- dynamic scrollable list
        
        enum {
          ITEM_NOTIFICATION_ENABLED,
          ITEM_NOTIFICATION_SWITCH,
          ITEM_NOTIFICATION_TONE,
          ITEM_NOTIFICATION_TEXT,

          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(!notification->enabled && i > ITEM_NOTIFICATION_ENABLED)
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        uint8_t numFocusable = listItemCount + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        
        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(46, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2]);
          bool edit = (isFocused && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_NOTIFICATION_ENABLED:
              {
                display.print(F("Enable:"));
                drawCheckbox(54, ypos, notification->enabled);
                if(edit)
                  notification->enabled = incDec(notification->enabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_NOTIFICATION_SWITCH:
              {
                display.print(F("Switch:"));
                display.setCursor(54, ypos);
                getControlSwitchName(textBuff, tempSwtch, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                {
                  if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
                    tempSwtch = incDecControlSwitch(tempSwtch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW | INCDEC_FLAG_FMODE_AS_SW);
                  else
                    tempSwtch = incDecControlSwitch(tempSwtch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
                }
              }
              break;
              
            case ITEM_NOTIFICATION_TONE:
              {
                display.print(F("Tone:"));
                display.setCursor(54, ypos);
                display.print(F("Tone"));
                display.print(notification->tone - AUDIO_NOTIFICATION_TONE_FIRST + 1);
                if(isFocused)
                {
                  //preview the tone while in edit mode
                  static bool editStarted = false;
                  static uint8_t prevTone;
                  if(isEditMode && !editStarted)
                  {
                    editStarted = true;
                    prevTone = notification->tone;
                  }
                  if(!isEditMode || heldButton == KEY_SELECT)
                  {
                    prevTone = notification->tone;
                    editStarted = false;
                  }
                  notification->tone = incDec(notification->tone, AUDIO_NOTIFICATION_TONE_FIRST, AUDIO_NOTIFICATION_TONE_LAST, INCDEC_WRAP, INCDEC_SLOW);
                  if(notification->tone != prevTone && buttonCode == 0 && millis() - buttonReleaseTime >= 500) 
                  {
                    prevTone = notification->tone;
                    audioToPlay = notification->tone;
                  }
                }
              }
              break;
              
            case ITEM_NOTIFICATION_TEXT:
              {
                display.print(F("Text:"));
                display.setCursor(54, ypos);
                if(isEmptyStr(notification->text, sizeof(notification->text)))
                  display.print(F("--"));
                else
                  display.print(notification->text);
                if(edit)
                  isEditTextDialog = true;
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);

        //open context menu
        if(focusedItem == numFocusable && isEditMode)
          changeToScreen(CONTEXT_MENU_NOTIFICATIONS);
        
        //change to next
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisNotificationIdx = incDec(thisNotificationIdx, 0, NUM_CUSTOM_NOTIFICATIONS - 1, INCDEC_WRAP, INCDEC_SLOW);
          //assign from temp
          notification->swtch = tempSwtch;
          tempInitialised = false;
        }

        //assign from temp
        if(!isEditMode || (buttonCode == 0 && millis() - buttonReleaseTime >= 500))
        {
          notification->swtch = tempSwtch;
          tempInitialised = false;
        }

        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_EXTRAS_MENU);
          //assign from temp
          notification->swtch = tempSwtch;
          tempInitialised = false;
        }
      }
      break;
      
    case CONTEXT_MENU_NOTIFICATIONS:
      {
        enum {
          ITEM_COPY_TO,
          ITEM_RESET_SETTINGS,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_TO);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_COPY_TO)
        {
          destNotificationIdx = thisNotificationIdx;
          changeToScreen(DIALOG_COPY_NOTIFICATION);
        }
        
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetNotificationParams(thisNotificationIdx);
          changeToScreen(SCREEN_NOTIFICATION_SETUP);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_NOTIFICATION_SETUP);
      }
      break;
      
    case DIALOG_COPY_NOTIFICATION:
      {
        isEditMode = true;
        destNotificationIdx = incDec(destNotificationIdx, 0, NUM_CUSTOM_NOTIFICATIONS - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("#"), thisNotificationIdx, destNotificationIdx, true);
        if(clickedButton == KEY_SELECT)
        {
          Model.CustomNotification[destNotificationIdx] = Model.CustomNotification[thisNotificationIdx];
          thisNotificationIdx = destNotificationIdx;
          changeToScreen(SCREEN_NOTIFICATION_SETUP);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_NOTIFICATION_SETUP);
      }
      break;

    case SCREEN_TRIM_SETUP:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_TRIM_SETUP]);
        
        uint8_t qqSrc[4] = {SRC_X1_TRIM, SRC_Y1_TRIM, SRC_X2_TRIM, SRC_Y2_TRIM};

        trim_params_t* qqTrim[4];
        qqTrim[0] = &Model.X1Trim;
        qqTrim[1] = &Model.Y1Trim;
        qqTrim[2] = &Model.X2Trim;
        qqTrim[3] = &Model.Y2Trim;

        int16_t fmdTrimVal[4];
        fmdTrimVal[0] = Model.FlightMode[activeFmdIdx].x1Trim;
        fmdTrimVal[1] = Model.FlightMode[activeFmdIdx].y1Trim;
        fmdTrimVal[2] = Model.FlightMode[activeFmdIdx].x2Trim;
        fmdTrimVal[3] = Model.FlightMode[activeFmdIdx].y2Trim;
        
        for(uint8_t i = 0; i < 4; i++)
        {
          display.setCursor(0, 9 + i * 9);
          getSrcName(textBuff, qqSrc[i], sizeof(textBuff));
          display.print(textBuff);
          display.print(F(":"));
          display.setCursor(42, 9 + i * 9);
          display.print(findStringInIdStr(enum_TrimState, qqTrim[i]->trimState));
          display.setCursor(84, 9 + i * 9);
          display.print(F("("));
          int16_t val = 0;
          if(qqTrim[i]->trimState == TRIM_COMMON) 
            val = qqTrim[i]->commonTrim;
          else if(qqTrim[i]->trimState == TRIM_FLIGHT_MODE) 
            val = fmdTrimVal[i];
          if(val < 0)
          {
            display.print(F("-"));
            val = -val;
          }
          display.print(val / 10);
          display.print(F("."));
          display.print(val % 10);
          display.print(F(")"));
        }
        
        display.setCursor(0, 45);
        display.print(F("Step:"));
        display.setCursor(42, 45);
        display.print(findStringInIdStr(enum_TrimStep, Model.trimStep));

        changeFocusOnUpDown(5);
        toggleEditModeOnSelectClicked();
        drawCursor(34, focusedItem * 9);
        
        //edit
        if(isEditMode && focusedItem <= 4)
        {
          uint8_t i = focusedItem - 1;
          do {
            qqTrim[i]->trimState = incDec(qqTrim[i]->trimState, 0, TRIM_STATE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
          } while(Model.type == MODEL_TYPE_OTHER && qqTrim[i]->trimState == TRIM_FLIGHT_MODE);
        }
        if(focusedItem == 5)
          Model.trimStep = incDec(Model.trimStep, 0, TRIM_STEP_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
      
    case SCREEN_FLIGHT_MODES:
      {
        flight_mode_t *fmd = &Model.FlightMode[thisFmdIdx];
        
        if(isEditTextDialog)
        {
          editTextDialog(PSTR("Flght mode name"), fmd->name, sizeof(fmd->name), true, true, false);
          break;
        }
        
        drawHeader(extrasMenu[EXTRAS_MENU_FLIGHT_MODES]);
        
        //-- draw --
        display.setCursor(8, 9);
        display.print(F("Fmd"));
        display.print(thisFmdIdx + 1);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        display.setCursor(0, 20);
        display.print(F("Name:"));
        display.setCursor(66, 20);
        if(isEmptyStr(fmd->name, sizeof(fmd->name)))
          display.print(F("--"));
        else
          display.print(fmd->name);
        
        display.setCursor(0, 29);
        display.print(F("Switch:"));
        display.setCursor(66, 29);
        if(thisFmdIdx == 0)
          display.print(F("N/A")); 
        else
        {
          getControlSwitchName(textBuff, fmd->swtch, sizeof(textBuff));
          display.print(textBuff);
        }
        
        display.setCursor(0, 38);
        display.print(F("Fade-in:"));
        display.setCursor(66, 38);
        printSeconds(fmd->transitionTime);
        
        //draw cursor
        if(focusedItem == 1) drawCursor(0, 9);
        else drawCursor(58, (focusedItem * 9) + 2);
        
        changeFocusOnUpDown(4);
        toggleEditModeOnSelectClicked();
        
        //edit
        if(focusedItem == 1)
          thisFmdIdx = incDec(thisFmdIdx, 0, NUM_FLIGHT_MODES - 1, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 2 && isEditMode)
          isEditTextDialog = true;
        else if(focusedItem == 3 && thisFmdIdx > 0 && isEditMode)
          fmd->swtch = incDecControlSwitch(fmd->swtch, INCDEC_FLAG_PHY_SW | INCDEC_FLAG_LGC_SW);
        else if(focusedItem == 4)
          fmd->transitionTime = incDec(fmd->transitionTime, 0, 50, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;

    ////////////////////////////////// TELEMETRY ///////////////////////////////////////////////////

    case SCREEN_TELEMETRY:
      {
        drawHeader(mainMenu[MAIN_MENU_TELEMETRY]);
        
        //--- scrollable list ----
        
        static uint8_t topItem = 1;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = thisTelemIdx + 1;
          viewInitialised = true;
        }
        
        //handle navigation
        uint8_t numFocusable = NUM_CUSTOM_TELEMETRY;
        changeFocusOnUpDown(numFocusable);
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 6)
          topItem++;
        
        //fill list
        for(uint8_t line = 0; line < 6 && line < NUM_CUSTOM_TELEMETRY; line++)
        {
          uint8_t ypos = 10 + line * 9;
          uint8_t item = topItem + line;

          if(focusedItem == topItem + line) //highlight
          {
            display.fillRect(0, ypos - 1, 7, 9, BLACK);
            display.fillRect(2, ypos + 2, 3, 3, WHITE);
          }
          else
            display.fillRect(2, ypos + 2, 3, 3, BLACK);

          uint8_t idx = item - 1;
          if(!isEmptyStr(Model.Telemetry[idx].name, sizeof(Model.Telemetry[0].name)))
          {
            display.setCursor(11, ypos);
            display.print(Model.Telemetry[idx].name);
            display.print(F(":"));
            if(Model.Telemetry[idx].type == TELEMETRY_TYPE_GENERAL)
            {
              drawTelemetryValue(67, ypos, idx, telemetryReceivedValue[idx], true);
            }
            else if(Model.Telemetry[idx].type == TELEMETRY_TYPE_GNSS)
            {
              display.setCursor(67, ypos);
              if(GNSSTelemetryData.satellitesInView > 0)
              {
                display.print(GNSSTelemetryData.satellitesInUse);
                display.print(F("/"));
                display.print(GNSSTelemetryData.satellitesInView);
                display.print(F(" sat"));
              }
              else
                display.print(F("No data"));
            }
          }
        }

        //scroll bar
        drawScrollBar(127, 9, NUM_CUSTOM_TELEMETRY, topItem, 6, 6 * 9);

        //open sensor context menu
        if(focusedItem <= NUM_CUSTOM_TELEMETRY)
        {
          thisTelemIdx = focusedItem - 1;
          if(clickedButton == KEY_SELECT)
          {
            viewInitialised = false;
            if(isEmptyStr(Model.Telemetry[thisTelemIdx].name, sizeof(Model.Telemetry[0].name)))
              changeToScreen(CONTEXT_MENU_FREE_SENSOR);
            else
              changeToScreen(CONTEXT_MENU_ACTIVE_SENSOR);
          }
        }

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_MAIN_MENU);
        }
      }
      break;

    case CONTEXT_MENU_FREE_SENSOR:
      {
        enum {
          ITEM_NEW_SENSOR,
          ITEM_SENSOR_TEMPLATES
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("New sensor"), ITEM_NEW_SENSOR);
        contextMenuAddItem(PSTR("Templates"), ITEM_SENSOR_TEMPLATES);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_NEW_SENSOR)
        { 
          resetTelemParams(thisTelemIdx);        
          changeToScreen(SCREEN_CREATE_SENSOR);
        }
        if(contextMenuSelectedItemID == ITEM_SENSOR_TEMPLATES)
        {
          resetTelemParams(thisTelemIdx);
          changeToScreen(CONTEXT_MENU_SENSOR_TEMPLATES);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;
      
    case SCREEN_CREATE_SENSOR:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Sensor name"), Model.Telemetry[thisTelemIdx].name, sizeof(Model.Telemetry[0].name), 
                       false, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_EDIT_SENSOR);
      } 
      break;
      
    case CONTEXT_MENU_SENSOR_TEMPLATES:
      {
        enum {
          ITEM_EXTVOLTS_2S,
          ITEM_EXTVOLTS_3S,
          ITEM_EXTVOLTS_4S,
          ITEM_RSSI,
          ITEM_LINK_QUALITY,
          ITEM_GNSS,
          ITEM_GNSS_MSL_ALTITUDE,
          ITEM_GNSS_AGL_ALTITUDE,
          ITEM_GNSS_DISTANCE,
          ITEM_GNSS_SPEED,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("ExtVolts 2S"), ITEM_EXTVOLTS_2S);
        contextMenuAddItem(PSTR("ExtVolts 3S"), ITEM_EXTVOLTS_3S);
        contextMenuAddItem(PSTR("ExtVolts 4S"), ITEM_EXTVOLTS_4S);
        contextMenuAddItem(PSTR("Link quality"), ITEM_LINK_QUALITY);
        contextMenuAddItem(PSTR("RSSI"), ITEM_RSSI);
        bool isGNSSAlreadyAdded = false;
        for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
        {
          if(Model.Telemetry[i].type == TELEMETRY_TYPE_GNSS)
          {
            isGNSSAlreadyAdded = true;
            contextMenuAddItem(PSTR("GNSS altitude AGL"), ITEM_GNSS_AGL_ALTITUDE);
            contextMenuAddItem(PSTR("GNSS altitude MSL"), ITEM_GNSS_MSL_ALTITUDE);
            contextMenuAddItem(PSTR("GNSS speed"), ITEM_GNSS_SPEED);
            contextMenuAddItem(PSTR("GNSS distance"), ITEM_GNSS_DISTANCE);
            break;
          }
        }
        if(!isGNSSAlreadyAdded)
          contextMenuAddItem(PSTR("GNSS/GPS"), ITEM_GNSS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_EXTVOLTS_2S)  loadSensorTemplateExtVolts2S(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_EXTVOLTS_3S)  loadSensorTemplateExtVolts3S(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_EXTVOLTS_4S)  loadSensorTemplateExtVolts4S(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_RSSI)         loadSensorTemplateRSSI(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_LINK_QUALITY) loadSensorTemplateLinkQuality(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_GNSS)         loadSensorTemplateGNSS(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_GNSS_SPEED)   loadSensorTemplateGNSSSpeed(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_GNSS_DISTANCE)  loadSensorTemplateGNSSDistance(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_GNSS_AGL_ALTITUDE)  loadSensorTemplateGNSSAGLAltitude(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_GNSS_MSL_ALTITUDE)  loadSensorTemplateGNSSMSLAltitude(thisTelemIdx);
        
        if(contextMenuSelectedItemID != 0xff) 
          changeToScreen(SCREEN_TELEMETRY);
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;
      
    case CONTEXT_MENU_ACTIVE_SENSOR:
      {
        enum {
          ITEM_VIEW_STATISTICS,
          ITEM_EDIT_SENSOR,
          ITEM_DELETE_SENSOR,
          ITEM_VIEW_GNSS_DATA,
          ITEM_RESET_AGL_ALTITUDE,
          ITEM_RESET_LAST_KNOWN_LOCATION,
          ITEM_RESET_STARTING_POINT,
        };
        
        contextMenuInitialise();
        if(Model.Telemetry[thisTelemIdx].type == TELEMETRY_TYPE_GENERAL)
        {
          contextMenuAddItem(PSTR("View statistics"), ITEM_VIEW_STATISTICS);
          contextMenuAddItem(PSTR("Edit sensor"), ITEM_EDIT_SENSOR);
          contextMenuAddItem(PSTR("Delete sensor"), ITEM_DELETE_SENSOR);
          if(Model.Telemetry[thisTelemIdx].identifier == SENSOR_ID_GNSS_AGL_ALTITUDE)
            contextMenuAddItem(PSTR("Reset altitude AGL"), ITEM_RESET_AGL_ALTITUDE);
          if(Model.Telemetry[thisTelemIdx].identifier == SENSOR_ID_GNSS_DISTANCE)
            contextMenuAddItem(PSTR("Reset start point"), ITEM_RESET_STARTING_POINT);
        }
        else if(Model.Telemetry[thisTelemIdx].type == TELEMETRY_TYPE_GNSS)
        {
          contextMenuAddItem(PSTR("View GNSS data"), ITEM_VIEW_GNSS_DATA);
          contextMenuAddItem(PSTR("Reset altitude AGL"), ITEM_RESET_AGL_ALTITUDE);
          contextMenuAddItem(PSTR("Reset start point"), ITEM_RESET_STARTING_POINT);
          contextMenuAddItem(PSTR("Reset last loctn"), ITEM_RESET_LAST_KNOWN_LOCATION);
          //add delete option if all the child items have been deleted
          bool hasChildren = false;
          for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
          {
            uint8_t id = Model.Telemetry[i].identifier;
            if(id == SENSOR_ID_GNSS_SPEED
              || id == SENSOR_ID_GNSS_DISTANCE
              || id == SENSOR_ID_GNSS_AGL_ALTITUDE
              || id == SENSOR_ID_GNSS_MSL_ALTITUDE)
            {
              hasChildren = true;
              break;
            }
          }
          if(!hasChildren)
            contextMenuAddItem(PSTR("Delete sensor"), ITEM_DELETE_SENSOR);
        }
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_VIEW_STATISTICS) changeToScreen(SCREEN_SENSOR_STATISTICS);
        if(contextMenuSelectedItemID == ITEM_EDIT_SENSOR) changeToScreen(SCREEN_EDIT_SENSOR);
        if(contextMenuSelectedItemID == ITEM_DELETE_SENSOR) changeToScreen(CONFIRMATION_DELETE_SENSOR);
        if(contextMenuSelectedItemID == ITEM_VIEW_GNSS_DATA) changeToScreen(SCREEN_TELEMETRY_GNSS);
        if(contextMenuSelectedItemID == ITEM_RESET_AGL_ALTITUDE)
        {
          if(GNSSTelemetryData.positionFix != 0)
            Model.gnssAltitudeOffset = GNSSTelemetryData.altitude;
          else
            Model.gnssAltitudeOffset = Model.gnssLastKnownAltitude;
          changeToScreen(SCREEN_TELEMETRY);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_STARTING_POINT)
        {
          Model.gnssHomeLatitude = GNSSTelemetryData.latitude;
          Model.gnssHomeLongitude = GNSSTelemetryData.longitude;
          changeToScreen(SCREEN_TELEMETRY);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_LAST_KNOWN_LOCATION)
        {
          Model.gnssLastKnownLatitude = 0;
          Model.gnssLastKnownLongitude = 0;
          Model.gnssLastKnownAltitude = 0;
          Model.gnssLastKnownDistanceFromHome = 0;
          changeToScreen(SCREEN_TELEMETRY);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;
      
    case SCREEN_SENSOR_STATISTICS:
      {
        drawHeader(PSTR("Statistics"));
        
        static uint8_t page = 0;
        static bool viewInitialised = false;
        static uint8_t topItem = 1;
        if(!viewInitialised)
        {
          page = thisTelemIdx;
          focusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }
        
        //--- dynamic scrollable list
        
        enum {
          ITEM_MINIMUM_VAL,
          ITEM_RESET_MINIMUM_VAL,
          ITEM_MAXIMUM_VAL,
          ITEM_RESET_MAXIMUM_VAL,
          ITEM_LAST_RECEIVED_VAL,
          ITEM_LAST_RECEIVED_TIME,
          ITEM_RESET_LAST_RECEIVED_VAL,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        //add items IDs to the list
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(!Model.Telemetry[page].recordMaximum && (i == ITEM_MAXIMUM_VAL || i == ITEM_RESET_MAXIMUM_VAL))
            continue;
          if(!Model.Telemetry[page].recordMinimum && (i == ITEM_MINIMUM_VAL || i == ITEM_RESET_MINIMUM_VAL))
            continue;
          //skip special telemetry
          if(Model.Telemetry[page].type == TELEMETRY_TYPE_GNSS)
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        uint8_t numFocusable = listItemCount + 1; //+1 for title focus
        changeFocusOnUpDown(numFocusable);
        if(focusedItem == 1) //title focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        
        //show title
        display.setCursor(8, 9);
        display.print(Model.Telemetry[page].name);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //show current val, right aligned.
        //using a hack to measure the width of the text before printing to actual position on screen
        drawTelemetryValue(0, 64, page, telemetryReceivedValue[page], false);
        uint8_t len_px = display.getCursorX();
        uint8_t xpos = 127 - len_px;
        drawTelemetryValue(xpos, 9, page, telemetryReceivedValue[page], true);
        display.drawRect(xpos - 2, 7, len_px + 3, 11, BLACK);

        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(58, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (focusedItem > 1 && itemID == listItemIDs[focusedItem - 2]);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_MAXIMUM_VAL:
              {
                display.print(F("Maximum:"));
                drawTelemetryValue(66, ypos, page, telemetryMaxReceivedValue[page], false);
              }
              break;
            
            case ITEM_RESET_MAXIMUM_VAL:
              {
                display.setCursor(66, ypos);
                display.print(F("[Reset]"));
                if(isFocused && clickedButton == KEY_SELECT)
                  telemetryMaxReceivedValue[page] = telemetryReceivedValue[page];
              }
              break;
            
            case ITEM_MINIMUM_VAL:
              {
                display.print(F("Minimum:"));
                drawTelemetryValue(66, ypos, page, telemetryMinReceivedValue[page], false);
              }
              break;
              
            case ITEM_RESET_MINIMUM_VAL:
              {
                display.setCursor(66, ypos);
                display.print(F("[Reset]"));
                if(isFocused && clickedButton == KEY_SELECT)
                  telemetryMinReceivedValue[page] = telemetryReceivedValue[page]; 
              }
              break;
              
            case ITEM_LAST_RECEIVED_VAL:
              {
                display.print(F("Last val:"));
                drawTelemetryValue(66, ypos, page, telemetryLastReceivedValue[page], false);
              }
              break;
              
            case ITEM_LAST_RECEIVED_TIME:
              {
                display.print(F("Time rcvd:"));
                display.setCursor(66, ypos);
                if(telemetryLastReceivedValue[page] == TELEMETRY_NO_DATA)
                  display.print(F("No data"));
                else
                {
                  uint32_t elapsedSeconds = (millis() - telemetryLastReceivedTime[page])/1000;
                  if(elapsedSeconds >= 3600)
                  {
                    uint8_t hours = elapsedSeconds / 3600;
                    display.print(hours);
                    display.setCursor(display.getCursorX() + 3, display.getCursorY());
                    display.print(F("hr"));
                  }
                  else if(elapsedSeconds >= 60)
                  {
                    uint8_t minutes = elapsedSeconds / 60;
                    display.print(minutes);
                    display.setCursor(display.getCursorX() + 3, display.getCursorY());
                    display.print(F("min"));
                  }
                  else
                  {
                    display.print(elapsedSeconds);
                    display.setCursor(display.getCursorX() + 3, display.getCursorY());
                    display.print(F("s"));
                  }
                  display.print(F(" ago"));
                }
              }
              break;
              
            case ITEM_RESET_LAST_RECEIVED_VAL:
              {
                display.setCursor(66, ypos);
                display.print(F("[Reset]"));
                if(isFocused && clickedButton == KEY_SELECT)
                  telemetryLastReceivedValue[page] = telemetryReceivedValue[page]; 
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //change to next sensor
        if(focusedItem == 1)
        { 
          drawCursor(0, 9);
          toggleEditModeOnSelectClicked();  
          if(isEditMode)
          {
            do {
              page = incDec(page, 0, NUM_CUSTOM_TELEMETRY - 1, INCDEC_WRAP, INCDEC_SLOW);
            } while(isEmptyStr(Model.Telemetry[page].name, sizeof(Model.Telemetry[0].name))); //skip empty
          }          
        }
        
        //exit
        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_TELEMETRY);
        }
      }
      break;
    
    case SCREEN_EDIT_SENSOR:
      {
        telemetry_params_t *tlm = &Model.Telemetry[thisTelemIdx]; 
        
        //--- dynamic scrollable list
        
        enum {
          ITEM_TELEMETRY_NAME,
          ITEM_TELEMETRY_UNITSNAME,
          ITEM_TELEMETRY_IDENTIFIER,
          ITEM_TELEMETRY_FACTOR10,
          ITEM_TELEMETRY_MULTIPLIER,
          ITEM_TELEMETRY_OFFSET,
          ITEM_TELEMETRY_ALARM_CONDITION,
          ITEM_TELEMETRY_ALARM_THRESHOLD,
          ITEM_TELEMETRY_SHOW_ON_HOME,
          ITEM_TELEMETRY_RECORD_MAXIMUM,
          ITEM_TELEMETRY_RECORD_MINIMUM,
          
          ITEM_COUNT
        };
        
        static bool isEditingName = false;
        static bool isEditingUnits = false;
        
        if(isEditTextDialog)
        {
          if(isEditingName) 
            editTextDialog(PSTR("Sensor name"), tlm->name, sizeof(tlm->name), false, true, false);
          else if(isEditingUnits)
            editTextDialog(PSTR("Units"),  tlm->unitsName, sizeof(tlm->unitsName), true, true, false);
          break;
        }
        //reset
        isEditingName = false;
        isEditingUnits = false;

        drawHeader(PSTR("Sensor"));
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(i == ITEM_TELEMETRY_ALARM_THRESHOLD && tlm->alarmCondition == TELEMETRY_ALARM_CONDITION_NONE)
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        changeFocusOnUpDown(listItemCount); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 7)
          topItem++;
        
        //fill list and edit items
        for(uint8_t line = 0; line < 7 && line < listItemCount; line++)
        {
          uint8_t ypos = 8 + line*8;
          if(focusedItem == topItem + line)
            drawCursor(58, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (itemID == listItemIDs[focusedItem - 1] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_TELEMETRY_NAME:
              {
                display.print(F("Name:"));
                display.setCursor(66, ypos);
                if(isEmptyStr(tlm->name, sizeof(tlm->name)))
                  display.print(F("--"));
                else
                  display.print(tlm->name);
                if(edit)
                {
                  isEditTextDialog = true;
                  isEditingName = true;
                }
              }
              break;
            
            case ITEM_TELEMETRY_UNITSNAME:
              {
                display.print(F("Units:"));
                display.setCursor(66, ypos);
                if(isEmptyStr(tlm->unitsName, sizeof(tlm->unitsName)))
                  display.print(F("--"));
                else 
                  display.print(tlm->unitsName);
                if(edit)
                {
                  isEditTextDialog = true;
                  isEditingUnits = true;
                }
              }
              break;
            
            case ITEM_TELEMETRY_IDENTIFIER: 
              {
                display.print(F("ID:"));
                display.setCursor(66, ypos);
                display.print(F("0x"));
                if(tlm->identifier < 16)
                  display.print(F("0"));
                display.print(tlm->identifier, HEX);
                if(edit)
                  tlm->identifier = incDec(tlm->identifier, 0x00, 0xFE, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;

            case ITEM_TELEMETRY_MULTIPLIER:
              {
                display.print(F("Multplr:"));
                display.setCursor(66, ypos);
                int16_t val = tlm->multiplier;
                display.print(val / 100);
                display.print(F("."));
                val = val % 100;
                if(val < 10) 
                  display.print(F("0"));
                display.print(val);
                if(edit)
                  tlm->multiplier = incDec(tlm->multiplier, 1, 1000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
              }
              break;
            
            case ITEM_TELEMETRY_FACTOR10:
              {
                display.print(F("Factor10:"));
                display.setCursor(66, ypos);
                display.print(tlm->factor10);
                if(edit)
                  tlm->factor10 = incDec(tlm->factor10, -3, 3, INCDEC_NOWRAP, INCDEC_SLOW);
              }
              break;
            
            case ITEM_TELEMETRY_OFFSET:
              {
                display.print(F("Offset:"));
                display.setCursor(66, ypos);
                printTelemParam(thisTelemIdx, tlm->offset, false);
                if(edit)
                  tlm->offset = incDec(tlm->offset, -30000, 30000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
              }
              break;

            case ITEM_TELEMETRY_ALARM_CONDITION:
              {
                display.print(F("Alerts:"));
                display.setCursor(66, ypos);
                display.print(findStringInIdStr(enum_TelemetryAlarmCondition, tlm->alarmCondition));
                if(edit)
                  tlm->alarmCondition = incDec(tlm->alarmCondition, 0, TELEMETRY_ALARM_CONDITION_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
            
            case ITEM_TELEMETRY_ALARM_THRESHOLD:
              {
                display.print(F("Threshold:"));
                display.setCursor(66, ypos);
                printTelemParam(thisTelemIdx, tlm->alarmThreshold, true);
                if(edit)
                  tlm->alarmThreshold = incDec(tlm->alarmThreshold, -30000, 30000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
              }
              break;
            
            case ITEM_TELEMETRY_SHOW_ON_HOME:
              {
                display.print(F("On home:"));
                drawCheckbox(66, ypos, tlm->showOnHome);
                if(edit)
                  tlm->showOnHome = incDec(tlm->showOnHome, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_TELEMETRY_RECORD_MAXIMUM:
              {
                display.print(F("RecordMax:"));
                drawCheckbox(66, ypos, tlm->recordMaximum);
                if(edit)
                  tlm->recordMaximum = incDec(tlm->recordMaximum, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_TELEMETRY_RECORD_MINIMUM:
              {
                display.print(F("RecordMin:"));
                drawCheckbox(66, ypos, tlm->recordMinimum);
                if(edit)
                  tlm->recordMinimum = incDec(tlm->recordMinimum, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 8, listItemCount, topItem, 7, 7 * 8);
        
        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;
      
    case CONFIRMATION_DELETE_SENSOR:
      {
        printFullScreenMessage(PSTR("Delete sensor?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          resetTelemParams(thisTelemIdx);
          //reset any associated logical switches
          for(uint8_t i = 0; i < NUM_LOGICAL_SWITCHES; i++)
          {
            if(Model.LogicalSwitch[i].func <= LS_FUNC_GROUP3_LAST 
               && Model.LogicalSwitch[i].val1 == (int16_t) SRC_TELEMETRY_FIRST + thisTelemIdx)
            {
              resetLogicalSwitchParams(i);
            }
          }
          //reset any associated widgets
          for(uint8_t i = 0; i < NUM_WIDGETS; i++)
          {
            if(Model.Widget[i].type == WIDGET_TYPE_TELEMETRY && Model.Widget[i].src == thisTelemIdx)
            {
              resetWidgetParams(i);
            }
          }
          //exit
          changeToScreen(SCREEN_TELEMETRY);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;

    case SCREEN_TELEMETRY_GNSS:
      {
        drawHeader(PSTR("GNSS data"));

        enum {
          ITEM_SATELLITES,
          ITEM_DISTANCE,
          ITEM_SPEED,
          ITEM_COURSE,
          ITEM_AGL_ALTITUDE,
          ITEM_MSL_ALTITUDE,
          ITEM_LATITUDE,
          ITEM_LONGITUDE,
          ITEM_SEPARATOR,
          ITEM_TITLE_HOME_LOCATION,
          ITEM_HOME_LATITUDE,
          ITEM_HOME_LONGITUDE,

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
            case ITEM_SATELLITES:
              {
                display.print(F("Satellite:"));
                display.setCursor(60, ypos);
                display.print(GNSSTelemetryData.satellitesInUse);
                display.print(F("/"));
                display.print(GNSSTelemetryData.satellitesInView);
                if(GNSSTelemetryData.positionFix != 0)
                {
                  display.setCursor(108, ypos);
                  display.print(F("Fix"));
                }
              }
              break;

            case ITEM_LATITUDE:
              {
                display.print(F("Latitude:"));
                display.setCursor(60, ypos);
                if(GNSSTelemetryData.positionFix != 0)
                  printFixedPointVal(GNSSTelemetryData.latitude, 5);
                else
                  printFixedPointVal(Model.gnssLastKnownLatitude, 5);
                display.write(0xF8);
              }
              break;

            case ITEM_LONGITUDE:
              {
                display.print(F("Longitude:"));
                display.setCursor(60, ypos);
                if(GNSSTelemetryData.positionFix != 0)
                  printFixedPointVal(GNSSTelemetryData.longitude, 5);
                else
                  printFixedPointVal(Model.gnssLastKnownLongitude, 5);
                display.write(0xF8);
              }
              break;

            case ITEM_TITLE_HOME_LOCATION:
              {
                display.print(F("Start point"));
              }
              break;
            
            case ITEM_HOME_LATITUDE:
              {
                display.print(F("Latitude:"));
                display.setCursor(60, ypos);
                printFixedPointVal(Model.gnssHomeLatitude, 5);
                display.write(0xF8);
              }
              break;
            
            case ITEM_HOME_LONGITUDE:
              {
                display.print(F("Longitude:"));
                display.setCursor(60, ypos);
                printFixedPointVal(Model.gnssHomeLongitude, 5);
                display.write(0xF8);
              }
              break;

            case ITEM_MSL_ALTITUDE:
              {
                display.print(F("Altitude:"));
                display.setCursor(60, ypos);
                if(GNSSTelemetryData.positionFix != 0)
                  display.print(GNSSTelemetryData.altitude);
                else
                  display.print(Model.gnssLastKnownAltitude);
                display.setCursor(display.getCursorX() + 3, ypos);
                display.print(F("m"));
                display.setCursor(108, ypos);
                display.print(F("MSL"));
              }
              break;

            case ITEM_AGL_ALTITUDE:
              {
                display.print(F("Altitude:"));
                display.setCursor(60, ypos);
                if(GNSSTelemetryData.positionFix != 0)
                  display.print(GNSSTelemetryData.altitude - Model.gnssAltitudeOffset);
                else
                  display.print(Model.gnssLastKnownAltitude - Model.gnssAltitudeOffset);
                display.setCursor(display.getCursorX() + 3, ypos);
                display.print(F("m"));
                display.setCursor(108, ypos);
                display.print(F("AGL"));
              }
              break;

            case ITEM_DISTANCE:
              {
                display.print(F("Distance:"));
                display.setCursor(60, ypos);
                int32_t distance = Model.gnssLastKnownDistanceFromHome;
                if(GNSSTelemetryData.positionFix != 0)
                  distance = gnssDistanceFromHome;
                if(distance < 1000)
                {
                  display.print(distance);
                  display.setCursor(display.getCursorX() + 3, ypos);
                  display.print(F("m"));
                }
                else
                {
                  if(distance < 100000)
                    printFixedPointVal(distance, 3);
                  else
                    printFixedPointVal(distance / 100, 1);
                  display.setCursor(display.getCursorX() + 3, ypos);
                  display.print(F("km"));
                }
              }
              break;
            
            case ITEM_SPEED:
              {
                display.print(F("Speed:"));
                display.setCursor(60, ypos);
                display.print(GNSSTelemetryData.speed / 10);
                display.print(F("."));
                display.print(GNSSTelemetryData.speed % 10);
                display.setCursor(display.getCursorX() + 3, ypos);
                display.print(F("m/s"));
              }
              break;

            case ITEM_COURSE:
              {
                display.print(F("Course:"));
                display.setCursor(60, ypos);
                display.print(GNSSTelemetryData.course / 10);
                display.print(F("."));
                display.print(GNSSTelemetryData.course % 10);
                display.write(0xF8);
              }
              break;
          }
        }

        //Draw scroll bar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_TELEMETRY);
          viewInitialised = false;
        }

      }
      break;
      
    ////////////////////////////////// SYSTEM //////////////////////////////////////////////////////
      
    case SCREEN_SYSTEM_MENU:
      {
        drawHeader_Menu(mainMenu[MAIN_MENU_SYSTEM]);
        
        static uint8_t topItem = 1, highlightedItem = 1;

        menuInitialise();
        menuAddItem(systemMenu[SYSTEM_MENU_RF], SYSTEM_MENU_RF, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_SOUND], SYSTEM_MENU_SOUND, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_BACKLIGHT], SYSTEM_MENU_BACKLIGHT, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_APPEARANCE], SYSTEM_MENU_APPEARANCE, NULL);
        menuAddItem(systemMenu[SYSTEM_MENU_ADVANCED], SYSTEM_MENU_ADVANCED, NULL);
        menuDraw(&topItem, &highlightedItem);

        if(menuSelectedItemID == SYSTEM_MENU_RF) changeToScreen(SCREEN_RF);
        else if(menuSelectedItemID == SYSTEM_MENU_SOUND) changeToScreen(SCREEN_SOUND);
        else if(menuSelectedItemID == SYSTEM_MENU_BACKLIGHT) changeToScreen(SCREEN_BACKLIGHT);
        else if(menuSelectedItemID == SYSTEM_MENU_APPEARANCE) changeToScreen(SCREEN_APPEARANCE);
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
          ITEM_INACTIVITY_MINUTES,
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
        
        //initialise
        static uint8_t topItem;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }
        
        //handle navigation
        changeFocusOnUpDown(listItemCount);
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
              
            case ITEM_INACTIVITY_MINUTES:
              {
                display.print(F("Inactvty:"));
                display.setCursor(72, ypos);
                if(Sys.inactivityMinutes == 0)
                  display.print(F("Off"));
                else
                {
                  display.print(Sys.inactivityMinutes);
                  display.setCursor(display.getCursorX() + 3, display.getCursorY());
                  display.print(F("min"));
                }
                if(edit)
                  Sys.inactivityMinutes = incDec(Sys.inactivityMinutes, 0, 240, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
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
        {
          changeToScreen(SCREEN_SYSTEM_MENU);
          viewInitialised = false;
        }
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
          display.print(Sys.backlightLevel);
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
          Sys.backlightLevel = incDec(Sys.backlightLevel, 10, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
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
        
        //initialise
        static uint8_t topItem;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }
        
        //handle navigation
        changeFocusOnUpDown(ITEM_COUNT);
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 6)
          topItem++;
        
        toggleEditModeOnSelectClicked();
        
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
        
        //Draw scroll bar
        drawScrollBar(127, 9, ITEM_COUNT, topItem, 6, 6 * 9);
        
        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_SYSTEM_MENU);
          viewInitialised = false;
        }
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
        menuAddItem(advancedMenu[ADVANCED_MENU_MISCELLANEOUS], ADVANCED_MENU_MISCELLANEOUS, NULL);
        menuAddItem(advancedMenu[ADVANCED_MENU_DEBUG], ADVANCED_MENU_DEBUG, NULL);
        menuAddItem(advancedMenu[ADVANCED_MENU_ABOUT], ADVANCED_MENU_ABOUT, NULL);
        menuDraw(&topItem, &highlightedItem);

        if(menuSelectedItemID == ADVANCED_MENU_STICKS) changeToScreen(SCREEN_STICKS);
        else if(menuSelectedItemID == ADVANCED_MENU_KNOBS) changeToScreen(SCREEN_KNOBS);
        else if(menuSelectedItemID == ADVANCED_MENU_SWITCHES) changeToScreen(SCREEN_SWITCHES); 
        else if(menuSelectedItemID == ADVANCED_MENU_BATTERY) changeToScreen(SCREEN_BATTERY);
        else if(menuSelectedItemID == ADVANCED_MENU_DEBUG) changeToScreen(SCREEN_DEBUG);
        else if(menuSelectedItemID == ADVANCED_MENU_SECURITY) changeToScreen(SCREEN_SECURITY);
        else if(menuSelectedItemID == ADVANCED_MENU_MISCELLANEOUS) changeToScreen(SCREEN_MISCELLANEOUS);
        else if(menuSelectedItemID == ADVANCED_MENU_ABOUT) changeToScreen(SCREEN_ABOUT);

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
              
              //edit params
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
                printFullScreenMessage(PSTR("Move sticks fully,\nthen center them.\n"));
              else
                printFullScreenMessage(PSTR("Move sticks fully\n"));
                
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
              
              //edit params
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
              
              //edit params
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
                printFullScreenMessage(PSTR("Move knobs fully,\nthen center them.\n"));
              else
                printFullScreenMessage(PSTR("Move knobs fully\n"));
              
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
              
              //edit params
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
            drawCursor(58, ypos);
          display.setCursor(0, ypos);
          uint8_t idx = line + topItem - 1;
          display.print(F("Switch "));
          display.write(65 + idx);
          display.print(F(":"));
          display.setCursor(66, ypos);
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
        printVoltage(Sys.battVoltsMin);

        display.setCursor(0, 18);
        display.print(F("Gauge max:"));
        display.setCursor(72, 18);
        printVoltage(Sys.battVoltsMax);
        
        display.setCursor(0, 27);
        display.print(F("Multplr:"));
        display.setCursor(72, 27);
        display.print(Sys.battVfactor);
        
        display.setCursor(0, hasNextButton ? 45 : 56);
        display.print(F("("));
        printVoltage(battVoltsNow);
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
          int16_t val = Sys.battVoltsMin / 10;
          int16_t min = 300;
          int16_t max = (Sys.battVoltsMax - 100) / 10;
          //inc dec
          val = incDec(val, min, max, INCDEC_NOWRAP, INCDEC_NORMAL);
          //scale back
          Sys.battVoltsMin = val * 10;
        }
        else if(focusedItem == 2)
        {
          //scale down by 1/10 since we display only 2 decimals
          int16_t val = Sys.battVoltsMax / 10;
          int16_t min = (Sys.battVoltsMin + 100) / 10;
          int16_t max = 1500;
          //inc dec
          val = incDec(val, min, max, INCDEC_NOWRAP, INCDEC_NORMAL);
          //scale back
          Sys.battVoltsMax = val * 10;
        }
        else if(focusedItem == 3)
          Sys.battVfactor = incDec(Sys.battVfactor, 0, 2000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);

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
        
        drawCheckbox(8, 9, Sys.lockStartup);
        display.setCursor(18, 9);
        display.print(F("Lock startup"));
        
        drawCheckbox(8, 18, Sys.lockModels);
        display.setCursor(18, 18);
        display.print(F("Lock models"));
        
        display.setCursor(8, 27);
        if(isEmptyStr(Sys.password, sizeof(Sys.password)))
          display.print(F("[Set password]"));
        else
          display.print(F("[Change password]"));

        changeFocusOnUpDown(3);
        toggleEditModeOnSelectClicked();
        drawCursor(0, focusedItem * 9);
        
        if(focusedItem == 1)
          Sys.lockStartup = incDec(Sys.lockStartup, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 2)
          Sys.lockModels = incDec(Sys.lockModels, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 3 && isEditMode)
          isEditTextDialog = true;

        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_ADVANCED_MENU);
        }
      }
      break;
      
    case SCREEN_MISCELLANEOUS:
      {
        drawHeader(advancedMenu[ADVANCED_MENU_MISCELLANEOUS]);
        
        display.setCursor(0, 9);
        display.print(F("Autoslct input:"));
        drawCheckbox(102, 9, Sys.autoSelectMovedControl);

        display.setCursor(0, 18);
        display.print(F("On-screen trim:"));
        drawCheckbox(102, 18, Sys.onscreenTrimEnabled);
        
        display.setCursor(0, 27);
        display.print(F("Mixer templts:"));
        drawCheckbox(102, 27, Sys.mixerTemplatesEnabled);
        
        display.setCursor(0, 36);
        display.print(F("Dflt Ch order:"));
        display.setCursor(102, 36);
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

        changeFocusOnUpDown(4);
        toggleEditModeOnSelectClicked();
        drawCursor(94, focusedItem * 9);
        
        if(focusedItem == 1)
          Sys.autoSelectMovedControl = incDec(Sys.autoSelectMovedControl, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 2)
          Sys.onscreenTrimEnabled = incDec(Sys.onscreenTrimEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 3)
          Sys.mixerTemplatesEnabled = incDec(Sys.mixerTemplatesEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 4 && Sys.mixerTemplatesEnabled) 
        { 
          //there are 4P4 = 4! = 24 possible arrangements. So our range is 0 to 23.  
          Sys.defaultChannelOrder = incDec(Sys.defaultChannelOrder, 0, 23, INCDEC_WRAP, INCDEC_SLOW);
        }

        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_ADVANCED_MENU);
        }
      }
      break;
      
    case SCREEN_DEBUG:
      {
        drawHeader(advancedMenu[ADVANCED_MENU_DEBUG]);
        
        telemetryForceRequest = true;

        enum {
          ITEM_PACKET_RATE,
          ITEM_UP_TIME,
          ITEM_LOOP_NUM,
          ITEM_FIXED_LOOP_TIME,
          ITEM_INACTIVITY_TIME,
          ITEM_FREE_RAM,
          ITEM_SEPARATOR,
          ITEM_SHOW_LOOP_TIME,
          ITEM_DISABLE_INTERLACING,
          ITEM_SIMULATE_TELEMETRY,
          ITEM_SHOW_CHARACTER_SET,
          ITEM_DUMP_INTERNAL_EEPROM,
          ITEM_DUMP_EXTERNAL_EEPROM,
          ITEM_CONFIGURE_SCREENSHOTS,
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
        do {
          changeFocusOnUpDown(listItemCount);
        } while(listItemIDs[focusedItem - 1] == ITEM_SEPARATOR);
        
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
            case ITEM_SHOW_LOOP_TIME:
              {
                drawCheckbox(display.getCursorX(), ypos, Sys.DBG_showLoopTime);
                display.setCursor(display.getCursorX() + 10, ypos);
                display.print(F("Show loop time"));
                if(isFocused)
                {
                  toggleEditModeOnSelectClicked();
                  Sys.DBG_showLoopTime = incDec(Sys.DBG_showLoopTime, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                }
              }
              break;
              
            case ITEM_SIMULATE_TELEMETRY:
              {
                drawCheckbox(display.getCursorX(), ypos, Sys.DBG_simulateTelemetry);
                display.setCursor(display.getCursorX() + 10, ypos);
                display.print(F("Simulate telemetry"));
                if(isFocused)
                {
                  toggleEditModeOnSelectClicked();
                  bool lastState = Sys.DBG_simulateTelemetry;
                  Sys.DBG_simulateTelemetry = incDec(Sys.DBG_simulateTelemetry, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                  if(!lastState && Sys.DBG_simulateTelemetry)
                    makeToast(PSTR("ID:0x30, Src:Virt1"), 2000, 300);
                }
              }
              break;
              
            case ITEM_DISABLE_INTERLACING:
              {
                drawCheckbox(display.getCursorX(), ypos, Sys.DBG_disableInterlacing);
                display.setCursor(display.getCursorX() + 10, ypos);
                display.print(F("No LCD interlacing"));
                if(isFocused)
                {
                  toggleEditModeOnSelectClicked();
                  bool lastState = Sys.DBG_disableInterlacing;
                  Sys.DBG_disableInterlacing = incDec(Sys.DBG_disableInterlacing, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                  if(!lastState && Sys.DBG_disableInterlacing)
                    makeToast(PSTR("May reduce performnc"), 2000, 300);
                }
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
                  changeToScreen(SCREEN_SCREENSHOT_CONFIG);
              }
              break;
              
            case ITEM_FACTORY_RESET:
              {
                display.print(F("[Factory Reset]")); 
                if(isFocused && clickedButton == KEY_SELECT)
                  changeToScreen(CONFIRMATION_FACTORY_RESET);
              }
              break;
              
            case ITEM_SEPARATOR:
              {
                drawDottedHLine(8, ypos + 3, 112, BLACK, WHITE);
              }
              break;
            
            case ITEM_PACKET_RATE:
              {
                display.print(F("Pkt rate:"));
                display.setCursor(72, ypos);
                display.print(transmitterPacketRate);
                display.print(F(", "));
                display.print(receiverPacketRate);
              }
              break;
            
            case ITEM_UP_TIME:
              {
                display.print(F("Up time:"));
                display.setCursor(72, ypos);
                printHHMMSS(millis());
              }
              break;
            
            case ITEM_INACTIVITY_TIME:
              {
                display.print(F("Inactvty:"));
                display.setCursor(72, ypos);
                printHHMMSS(millis() - inputsLastMovedTime);
              }
              break;
            
            case ITEM_FREE_RAM:
              {
                display.print(F("Free ram:"));
                display.setCursor(72, ypos);
                display.print(getFreeRam());
                display.setCursor(display.getCursorX() + 3, ypos);
                display.print(F("bytes"));
              }
              break;
            
            case ITEM_LOOP_NUM:
              {
                display.print(F("Loop #:"));
                display.setCursor(72, ypos);
                display.print(thisLoopNum);
              }
              break;
            
            case ITEM_FIXED_LOOP_TIME:
              {
                display.print(F("Fxd loop:"));
                display.setCursor(72, ypos);
                display.print(fixedLoopTime);
                display.setCursor(display.getCursorX() + 3, ypos);
                display.print(F("ms"));
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

        if(!sdHasCard())
        {
          printFullScreenMessage(PSTR("SD card not found"));
          if(heldButton == KEY_SELECT)
            changeToScreen(SCREEN_DEBUG);
          break;
        }
        
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
          tempSwtch = incDecControlSwitch(tempSwtch, INCDEC_FLAG_PHY_SW);
        
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
            handleBatteryWarnUI();
            if(isDisplayingBattWarn)
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
        drawHeader_Menu(advancedMenu[ADVANCED_MENU_ABOUT]);
        
        static uint8_t topItem = 1, highlightedItem = 1;
        
        enum {
          ITEM_VERSION_INFO,
          ITEM_DISCLAIMER,
          ITEM_THIRD_PARTY,
          ITEM_EASTER_EGG,
        };

        menuInitialise();
        menuAddItem(PSTR("Version info"), ITEM_VERSION_INFO, NULL);
        menuAddItem(PSTR("Disclaimer"), ITEM_DISCLAIMER, NULL);
        menuAddItem(PSTR("Third party notices"), ITEM_THIRD_PARTY, NULL);
        menuAddItem(PSTR(" "), ITEM_EASTER_EGG, NULL);
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
        else if (menuSelectedItemID == ITEM_THIRD_PARTY)
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
          changeToScreen(SCREEN_ADVANCED_MENU);
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
              // i.e s = ut + 1/2at^2 and v = u + at
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
              
              //Calc background position
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
              drawAnimatedSprite(game->birdX, game->birdY, animation_bird, BIRD_LENGTH, BIRD_HEIGHT, BLACK, 3, 80, 0);
              
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
      
    ////////////////////////////////// RECEIVER ////////////////////////////////////////////////////

    case SCREEN_RECEIVER:
      {
        drawHeader(mainMenu[MAIN_MENU_RECEIVER]);

        enum { 
          PAGE_MAIN_RECEIVER,
          PAGE_SECONDARY_RECEIVER,
          NUM_PAGES
        };
        
        uint8_t numPages = NUM_PAGES;
        if(MAX_CHANNELS_PER_RECEIVER == NUM_RC_CHANNELS)
          numPages = 1;
        
        static uint8_t page = PAGE_MAIN_RECEIVER;

        isMainReceiver = (page == PAGE_MAIN_RECEIVER);

        display.setCursor(8, 9);
        strlcpy_P(textBuff, isMainReceiver ? PSTR("Main rcvr") : PSTR("Secondary rcvr"), sizeof(textBuff));
        display.print(textBuff);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);

        enum {
          ITEM_SECONDARY_RECEIVER_ENABLED,
          ITEM_BIND,
          ITEM_CONFIGURE,

          ITEM_COUNT
        };

        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(page == PAGE_MAIN_RECEIVER)
          {
            if(i == ITEM_SECONDARY_RECEIVER_ENABLED)
              continue;
          }
          else if(page == PAGE_SECONDARY_RECEIVER)
          {
            if(!Model.secondaryRcvrEnabled && i != ITEM_SECONDARY_RECEIVER_ENABLED)
              continue;
          }

          listItemIDs[listItemCount++] = i;
        }
        
        //initialise
        static uint8_t topItem;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }
        
        //handle navigation
        changeFocusOnUpDown(listItemCount + 1); //+1 for title focus
        toggleEditModeOnSelectClicked();
        if(focusedItem == 1) //title focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }

        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(0, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (focusedItem > 1 && itemID == listItemIDs[focusedItem - 2]);
          
          display.setCursor(8, ypos);
          switch(itemID)
          { 
            case ITEM_BIND:
              {
                display.print(F("[Bind]"));
                if(isFocused && clickedButton == KEY_SELECT)
                {
                  isRequestingBind = true;
                  changeToScreen(SCREEN_RECEIVER_BINDING);
                }
              }
              break;
            
            case ITEM_CONFIGURE:
              {
                display.print(F("[Configure]"));
                if(isFocused && clickedButton == KEY_SELECT)
                {
                  changeToScreen(SCREEN_RECEIVER_CONFIG);
                }
              }
              break;
              
            case ITEM_SECONDARY_RECEIVER_ENABLED:
              {
                drawCheckbox(8, ypos, Model.secondaryRcvrEnabled);
                display.setCursor(18, ypos);
                display.print(F("Enable"));
                if(isFocused && isEditMode)
                {
                  Model.secondaryRcvrEnabled = incDec(Model.secondaryRcvrEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                }
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);

        //change to next receiver
        if(focusedItem == 1)
        {
          drawCursor(0, 9);
          page = incDec(page, 0, numPages - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MAIN_MENU);
      }
      break;
      
    case SCREEN_RECEIVER_BINDING:
      {
        static uint32_t entryTime = 0;
        static bool initialised = false;
        if(!initialised)
        {
          initialised = true;
          entryTime = millis();
          bindStatusCode = 0;
        }

        printFullScreenMessage(PSTR("Binding"));
        if(Sys.animationsEnabled)
          drawLoaderSpinner(60, display.getCursorY() + 12, 2);
        
        if(bindStatusCode == 1)
        {
          makeToast(PSTR("Bind success"), 2000, 0);
          changeToScreen(SCREEN_RECEIVER);
          audioToPlay = AUDIO_BIND_SUCCESS;
          initialised = false;
        }
        else if(bindStatusCode == 2)
        {
          makeToast(PSTR("Bind failed"), 2000, 0);
          changeToScreen(SCREEN_RECEIVER);
          initialised = false;
        }
        else if(millis() - entryTime > 5000)
        {
          makeToast(PSTR("Bind timeout"), 2000, 0);
          changeToScreen(SCREEN_RECEIVER);
          initialised = false;
        }
      }
      break;
      
    case SCREEN_RECEIVER_CONFIG:
      {
        enum {
          QUERYING_CONFIG, 
          SENDING_CONFIG, 
          VIEWING_CONFIG,
          VIEWING_CONFIG_SERVOPWM
        };
        
        static uint8_t state = QUERYING_CONFIG;
        static bool stateInitialised = false;
        static uint32_t entryTime = 0;
        static bool actionStarted = false;
        
        if(!stateInitialised)
        {
          state = QUERYING_CONFIG;
          // state = VIEWING_CONFIG; //DEBUG
          entryTime = millis();
          gotOutputChConfig = false;
          stateInitialised = true;
          receiverConfigStatusCode = 0;
        }
        
        switch(state)
        {
          case QUERYING_CONFIG:
            {
              printFullScreenMessage(PSTR("Reading settings"));
              if(Sys.animationsEnabled)
                drawLoaderSpinner(60, display.getCursorY() + 12, 2);
              
              if(!actionStarted)
              {
                isRequestingOutputChConfig = true;
                actionStarted = true;
              }
              if(gotOutputChConfig)
              {
                actionStarted = false;
                state = VIEWING_CONFIG;
              }
              //Time out
              if(millis() - entryTime > 3000)
              {
                makeToast(PSTR("No response"), 2000, 0);
                stateInitialised = false;
                actionStarted = false;
                changeToScreen(SCREEN_RECEIVER);
              }
            }
            break;
            
          case VIEWING_CONFIG:
            {
              drawHeader(PSTR("Rcvr output config"));

              display.setCursor(0, 9);
              display.print(F("Type of signal"));
            
              //--scrollable list--
              
              static uint8_t topItem;
              static bool viewInitialised = false;
              if(!viewInitialised)
              {
                focusedItem = 1;
                topItem = 1;
                isEditMode = false;
                viewInitialised = true;
              }
            
              uint8_t startIdx = 0; 
              uint8_t endIdx = MAX_CHANNELS_PER_RECEIVER - 1;
              
              //fill list
              uint8_t numItems = (endIdx - startIdx) + 1; 
              for(uint8_t line = 0; line < 4 && line < numItems; line++)
              {
                uint8_t ypos = 18 + line * 9;
                uint8_t item = topItem + line;
                if(focusedItem == item)
                  drawCursor(34, ypos);
                
                display.setCursor(0, ypos);
                uint8_t idx = startIdx + item - 1; 
                if(idx <= endIdx)
                {
                  display.print(F("Ch"));
                  if(isMainReceiver)
                    display.print(idx + 1);
                  else
                    display.print(idx + 1 + MAX_CHANNELS_PER_RECEIVER);
                  display.print(F(":"));
                  display.setCursor(42, ypos);
                  uint8_t val = outputChConfig[idx] & 0x03;
                  display.print(findStringInIdStr(enum_outputChConfig, val));
                }
              }
              
              //Draw scroll bar
              drawScrollBar(127, 9, numItems, topItem, 4, 44);
              
              //show the write button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              display.setCursor(90, 56);
              display.print(F("[Next]"));
              if(focusedItem == numItems + 1)
                drawCursor(82, 56);
              
              //Handle navigation
              changeFocusOnUpDown(numItems + 1); //+1 for button focus
              if(focusedItem < topItem)
                topItem = focusedItem;
              while(focusedItem >= topItem + 4 && focusedItem < numItems + 1)
                topItem++;
              toggleEditModeOnSelectClicked();
            
              //edit params
              uint8_t idx = startIdx + focusedItem - 1;
              if(idx <= endIdx)
              {
                uint8_t signalType = outputChConfig[idx] & 0x03;
                uint8_t maxSignalType = (outputChConfig[idx] >> 2) & 0x03;
                signalType = incDec(signalType, 0, maxSignalType, INCDEC_WRAP, INCDEC_SLOW);
                outputChConfig[idx] &= ~0x03;
                outputChConfig[idx] |= signalType;
              }
              
              //move to next 
              if(focusedItem == numItems + 1 && clickedButton == KEY_SELECT)
              {
                state = VIEWING_CONFIG_SERVOPWM;
                viewInitialised = false;
              }
              
              //exit
              if(heldButton == KEY_SELECT)
              {
                stateInitialised = false;
                actionStarted = false;
                viewInitialised = false;
                changeToScreen(SCREEN_RECEIVER);
              }
            }
            break;
            
          case VIEWING_CONFIG_SERVOPWM:
            {
              drawHeader(PSTR("Rcvr output config"));

              display.setCursor(0, 9);
              display.print(F("Servo PWM range"));
            
              //--scrollable list--
              
              static uint8_t topItem;
              static bool viewInitialised = false;
              if(!viewInitialised)
              {
                focusedItem = 1;
                topItem = 1;
                isEditMode = false;
                viewInitialised = true;
              }
            
              uint8_t startIdx = 0; 
              uint8_t endIdx = MAX_CHANNELS_PER_RECEIVER - 1;
              
              //fill list
              uint8_t numItems = (endIdx - startIdx) + 1; 
              for(uint8_t line = 0; line < 4 && line < numItems; line++)
              {
                uint8_t ypos = 18 + line * 9;
                uint8_t item = topItem + line;
                if(focusedItem == item)
                  drawCursor(32, ypos);
                
                display.setCursor(0, ypos);
                uint8_t idx = startIdx + item - 1; 
                if(idx <= endIdx)
                {
                  display.print(F("Ch"));
                  if(isMainReceiver)
                    display.print(idx + 1);
                  else
                    display.print(idx + 1 + MAX_CHANNELS_PER_RECEIVER);
                  display.print(F(":"));
                  display.setCursor(40, ypos);
                  if((outputChConfig[idx] & 0x03) == SIGNAL_TYPE_SERVOPWM)
                  {
                    uint8_t servoPWMRangeIdx = (outputChConfig[idx] >> 4) & 0x0F;
                    display.print(500 + (servoPWMRangeIdx * 50));
                    display.print(F(" to "));
                    display.print(2500 - (servoPWMRangeIdx * 50));
                    display.setCursor(display.getCursorX() + 3, ypos);
                    display.print(F("\xE6s"));
                  }
                  else
                    display.print(F("N/A"));
                }
              }
              
              //Draw scroll bar
              drawScrollBar(127, 9, numItems, topItem, 4, 44);
              
              //show the write button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              display.setCursor(84, 56);
              display.print(F("[Write]"));
              if(focusedItem == numItems + 1)
                drawCursor(76, 56);
              
              //Handle navigation
              changeFocusOnUpDown(numItems + 1); //+1 for button focus
              if(focusedItem < topItem)
                topItem = focusedItem;
              while(focusedItem >= topItem + 4 && focusedItem < numItems + 1)
                topItem++;
              toggleEditModeOnSelectClicked();
            
              //edit params
              uint8_t idx = startIdx + focusedItem - 1;
              if(idx <= endIdx && ((outputChConfig[idx] & 0x03) == SIGNAL_TYPE_SERVOPWM))
              {
                uint8_t servoPWMRangeIdx = (outputChConfig[idx] >> 4) & 0x0F;
                servoPWMRangeIdx = incDec(servoPWMRangeIdx, 0, 15, INCDEC_NOWRAP, INCDEC_SLOW);
                outputChConfig[idx] &= 0x0F;
                outputChConfig[idx] |= servoPWMRangeIdx << 4;
              }
              
              //write configuration
              if(focusedItem == numItems + 1 && clickedButton == KEY_SELECT)
              {
                state = SENDING_CONFIG;
                entryTime = millis();
                gotOutputChConfig = false;
                actionStarted = false;
                viewInitialised = false;
              }
              
              //exit without writing changes
              if(heldButton == KEY_SELECT)
              {
                stateInitialised = false;
                actionStarted = false;
                viewInitialised = false;
                changeToScreen(SCREEN_RECEIVER);
              }
            }
            break;
            
          case SENDING_CONFIG:
            {
              printFullScreenMessage(PSTR("Writing settings"));
              if(Sys.animationsEnabled)
                drawLoaderSpinner(60, display.getCursorY() + 12, 2);
              
              if(!actionStarted)
              {
                isSendOutputChConfig = true;
                actionStarted = true;
              }
              if(receiverConfigStatusCode > 0)
              {
                if(receiverConfigStatusCode == 1) makeToast(PSTR("Write success"), 2000, 0);
                if(receiverConfigStatusCode == 2) makeToast(PSTR("Write failed"), 2000, 0);
                stateInitialised = false;
                actionStarted = false;
                changeToScreen(SCREEN_RECEIVER);
              }
              //Time out
              if(millis() - entryTime > 3000)
              {
                makeToast(PSTR("No response"), 2000, 0);
                stateInitialised = false;
                actionStarted = false;
                changeToScreen(SCREEN_RECEIVER);
              }
            }
            break;
        }
      }
      break;

    ////////////////////////////////// GENERIC TEXT VIEWER /////////////////////////////////////////
    
    case SCREEN_TEXT_VIEWER:
      {
        static uint16_t startPos; //the offset into the text

        static uint16_t scrollOffsetQQ[6]; //buffer to help with scrolling. Stores offsets into the text.
        static uint8_t idxQQ = 0;
        
        static bool initialised = false;
        if(!initialised)
        {
          initialised = true;
          startPos = 0;
          idxQQ = 0;
        }
        
        uint16_t pos = startPos; //position in text
        bool isEnd = false;
        
        //Print the text with both line wrap and word wrap. Word wrap prioritized. 
        //Doesn't cater for all edge cases, but works good enough.
        uint8_t line = 0, i = 0;
        bool isPageBreak = false;
        while(line < 6 && !isEnd)
        {
          while(i < 21)
          {
            char c = pgm_read_byte(textViewerText + pos);
            //Check for end of text
            if(c == '\0')
            {
              isEnd = true;
              break;
            }
            //Skip carriage return, or if we are at start of line and the character is a space
            if(c == '\r' || (i == 0 && c == ' '))
            {
              pos++; //advance position in text
              continue;
            }
            //Check if it is a new line character
            if(c == '\n')
            {
              pos++; //advance position in text
              break;
            }
            //check if it is a form feed character
            if(c == '\f')
            {
              isPageBreak = true;
              pos++; //advance position in text
              break;
            }
            
            //Figure out how long the word is. If it is longer than the space left on the 
            //line, force it onto a new line. If the word is longer than the total space on the line, 
            //just print anyway but starting on a new line. 
            if(pgm_read_byte(textViewerText + pos - 1) == ' ' || pos == 0) //start of word, prev character is a space
            {
              uint8_t wordLen = 0;
              uint8_t j = 0;
              char wordCharacter;
              while((wordCharacter = pgm_read_byte(textViewerText + pos + j)) != ' ')
              {
                if(wordCharacter == '\0' || wordCharacter == '\r' || wordCharacter == '\n' || wordCharacter == '\f')
                  break;
                wordLen++;
                j++;
              }
              uint8_t remainingSpace = 20 - i;
              if(wordLen > remainingSpace && i != 0)
                break;
            }
            //Write the character to the screen
            display.setCursor(1 + i*6, 6 + line*9);
            display.write(c);
            pos++; //advance position in text
            i++; //advance position in line
          }
          
          line++; //advance line
          i = 0; //reset position in line
          
          if(isPageBreak)
            break;
        }
        
        //Show arrow icons
        if(!isEnd)
          display.drawBitmap(61, 61, icon_down_arrow_small, 5, 3, BLACK);
        if(startPos > 0)
          display.drawBitmap(61, 0, icon_up_arrow_small, 5, 3, BLACK);
        
        //Handle scrolling down
        if(pressedButton == KEY_DOWN && !isEnd)
        {
          scrollOffsetQQ[idxQQ] = startPos; //store the current offset
          idxQQ++;
          if(idxQQ >= sizeof(scrollOffsetQQ)/scrollOffsetQQ[0])
          {
            idxQQ = sizeof(scrollOffsetQQ)/scrollOffsetQQ[0] - 1;
            //In this case, we have reached the buffer's limit, therefore 
            //remove the oldest offset by shifting elements in the array to the left.
            for(uint8_t i = 0; i < sizeof(scrollOffsetQQ)/scrollOffsetQQ[0] - 1; i++)
            {
              scrollOffsetQQ[i] = scrollOffsetQQ[i + 1];
            }
          }
          //update the start position
          startPos = pos;
        }
        //Handle scrolling up
        if(pressedButton == KEY_UP && startPos > 0)
        {
          if(idxQQ > 0)
          {
            startPos = scrollOffsetQQ[idxQQ - 1];
            idxQQ--;
          }
          else
          {
            //Here, we have reached the buffer's limit and the workaround is to recalculate the startPos
            //by simply subtracting off the number of characters that can be fitted on a single screen.
            if(startPos >= 126) // 21*6
              startPos -= 126;
            else
              startPos = 0;
          }
        }
        
        //Exit
        if(heldButton == KEY_SELECT)
        {
          initialised = false;
          changeToScreen(lastScreen);
        }
      }
      break;
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    
    default:
      changeToScreen(SCREEN_HOME);
      break;
  }

  //-------------- Toasts -------------------------------
  drawToast();
  
  //-------------- Custom notifications -----------------
  notificationHandler();
  
  //-------------- Debug print --------------------------
  if(Sys.DBG_showLoopTime)
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
  
  //-------------- Show on physical lcd -----------------
  if(Sys.DBG_disableInterlacing) //override
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
      printHHMMSS(ttqq + 999); //add 999ms so the displayed time doesnt 
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
  uint8_t headingXOffset = (display.width() - textWidthPix) / 2; //middle align heading
  display.setCursor(headingXOffset, 0);
  display.print(textBuff);
  display.drawHLine(0, 3, headingXOffset - 2, BLACK);
  display.drawHLine(headingXOffset + textWidthPix + 1, 3, 128 - (headingXOffset + textWidthPix + 1), BLACK);
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
                        uint8_t frameCount, uint16_t frameTime, uint32_t timeOffset)
{
  uint8_t frameIdx = ((millis() - timeOffset) % (frameCount * frameTime)) / frameTime; 
  if(!Sys.animationsEnabled)
    frameIdx = 0;
  display.drawBitmap(x, y, (uint8_t *)pgm_read_word(&bitmapTable[frameIdx]), w, h, color);
}

//--------------------------------------------------------------------------------------------------

void drawScrollBar(uint8_t xpos, uint8_t ypos, uint16_t numItems, uint16_t topItem, uint16_t numVisible, uint16_t viewportHeight)
{ 
  if(numItems > numVisible)
  {
    uint8_t barHeight = divRoundClosest((int32_t)viewportHeight * numVisible, numItems);
    //limit barHeight to 6px minimum
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

void drawHorizontalBarChart(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, int16_t val, int16_t valMin, int16_t valMax)
{
  display.drawRect(x, y, w, h, color);
  if(val < valMin) val = valMin;
  if(val > valMax) val = valMax;
  if(valMin > valMax) return;
  if(valMax - valMin == 0) return;
  w = divRoundClosest(((int32_t)w - 2)* (val - valMin), valMax - valMin);
  display.fillRect(x+1, y, w, h, color);
}

//--------------------------------------------------------------------------------------------------

void drawHorizontalBarChartZeroCentered(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, int16_t val, int16_t range)
{
  int8_t xCenter = x + w/2;
  if(range != 0)
  {
    if(range < 0) range = -range;
    if(val > 0)
    {
      if(val > range/2) val = range/2;
      int8_t xLen = (val*w)/range;
      display.fillRect(xCenter, y, xLen + 1, h, color);
    }
    else if(val < 0)
    {
      val = -val;
      if(val > range/2) val = range/2;
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
        graphYCoord[i] = cubicHermiteInterpolate(xQQ, yQQ, crv->numPoints, xCoord * 20) / 20; 
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
            graphYCoord[i] = yStart;
          if(xCoord == xEnd)
            graphYCoord[i] = yEnd;
          if(xCoord > xStart && (abs(graphYCoord[i] - graphYCoord[i-1]) > 1))
            display.drawLine(100 + xCoord - 1, 36 - graphYCoord[i-1], 100 + xCoord, 36 - graphYCoord[i], BLACK); 
          else
            display.drawPixel(100 + xCoord, 36 - graphYCoord[i], BLACK);
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
  // Z to A  (90 ... 65)   -->  0 ... 25
  // space   (32)          -->  26
  // a to z  (97 ... 122)  -->  27 ... 52
  // Numbers (48 ... 57)   -->  53 ... 62
  // - . /   (45 ... 47)   -->  63 ... 65
  // % symbol  (37)        -->  66
  // Degree symbol (248)   -->  67
  // Caret ^ (94)          -->  68
  // Square symbol (253)   -->  69
  // Greek letter mu (230) -->  70

  if(thisChar == 32) thisChar = 26;
  else if(thisChar == 37)  thisChar = 66;
  else if(thisChar <= 47)  thisChar += 18;
  else if(thisChar <= 57)  thisChar += 5;
  else if(thisChar <= 90)  thisChar = 90 - thisChar ;
  else if(thisChar == 94)  thisChar = 68;
  else if(thisChar <= 122) thisChar -= 70;
  else if(thisChar == 230) thisChar = 70;
  else if(thisChar == 248) thisChar = 67;
  else if(thisChar == 253) thisChar = 69;
  
  //adjust
  isEditMode = true;
  thisChar = incDec(thisChar, 70, 0, INCDEC_NOWRAP, INCDEC_SLOW);

  //map back
  if(thisChar <= 25) thisChar = 90 - thisChar;
  else if(thisChar == 26) thisChar = 32; 
  else if(thisChar <= 52) thisChar += 70;
  else if(thisChar <= 62) thisChar -= 5;
  else if(thisChar <= 65) thisChar -= 18;
  else if(thisChar == 66) thisChar = 37;
  else if(thisChar == 67) thisChar = 248;
  else if(thisChar == 68) thisChar = 94;
  else if(thisChar == 69) thisChar = 253;
  else if(thisChar == 70) thisChar = 230;

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
      makeToast(PSTR("Can't be empty"), 2000, 0);
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
  
  uint8_t numVisible = 4;
  uint8_t lineHeight = 13;
  uint8_t y0 = 14;
  if(Sys.useDenserMenus)
  {
    numVisible = 5;
    lineHeight = 11;
    y0 = 11;
  }
  
  //handle scenario of being called with invalid highlightedItem
  if(*highlightedItem > _menuItemCount) 
    *highlightedItem = 1;
  
  //handle navigation
  isEditMode = true;
  *highlightedItem = incDec(*highlightedItem, _menuItemCount, 1, INCDEC_WRAP, INCDEC_SLOW);
  isEditMode = false;
  if(*highlightedItem < *topItem)
    *topItem = *highlightedItem;
  while(*highlightedItem >= *topItem + numVisible)
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
  
  //fill menu slots
  for(uint8_t line = 0; line < numVisible && line < _menuItemCount; line++)
  {
    //prevent showing garbage entries when we've changed to using denser menus
    if(*topItem + line > _menuItemCount)
      break;
    
    uint8_t item = *topItem + line;
    uint8_t ypos = y0 + line * lineHeight;
    //highlight selection
    if(*highlightedItem == item)
    {
      display.fillRoundRect(2, Sys.useDenserMenus ? ypos - 2 : ypos - 3, _menuItemCount <= numVisible ? 124 : 123, 
                            lineHeight, Sys.useRoundRect ? 4 : 0, BLACK);
      display.setTextColor(WHITE);
    }
    //show icon
    if(_menuItemIcons[item - 1] != NULL && Sys.showMenuIcons)
      display.drawBitmap(7, ypos - 2 , _menuItemIcons[item - 1], 15, 11, *highlightedItem == item ? WHITE : BLACK);
    
    //show text
    display.setCursor((hasMenuIcons && Sys.showMenuIcons) ? 26 : 10, ypos);
    strlcpy_P(textBuff, _menuItems[item - 1], sizeof(textBuff));
    display.print(textBuff);
    display.setTextColor(BLACK);
  }
  
  //scroll bar
  drawScrollBar(127, Sys.useDenserMenus ? 9 : 11, _menuItemCount, *topItem, numVisible, numVisible * lineHeight);

  //get the id of the item selected
  menuSelectedItemID = 0xff;
  if(clickedButton == KEY_SELECT)
    menuSelectedItemID = _menuItemIDs[*highlightedItem - 1];
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

  //Calculate y coord for text item 0. Items are center aligned
  uint8_t numVisible = _menuItemCount <= maxVisible ? _menuItemCount : maxVisible;
  uint8_t y0 = ((display.height() - (numVisible * 10)) / 2) + 1;  //10 is line height
  
  //draw bounding box
  drawBoundingBox(5, y0 - 4, 117, numVisible * 10 + 5, BLACK);  
  
  //fill list
  for(uint8_t line = 0; line < numVisible; line++)
  {
    uint8_t ypos = y0 + line * 10;
    uint8_t item = contextMenuTopItem + line;
    strlcpy_P(textBuff, _menuItems[item-1], sizeof(textBuff));
    if(item == contextMenuFocusedItem)
    {
      display.fillRoundRect(7, ypos - 2, _menuItemCount <= maxVisible ? 113 : 109, 11, Sys.useRoundRect ? 4 : 0, BLACK);
      display.setTextColor(WHITE);
    }
    display.setCursor(11, ypos);
    display.print(textBuff);
    display.setTextColor(BLACK);
  }
  
  //scroll bar
  uint8_t  y = Sys.useRoundRect ? y0 - 1 : y0 - 2;
  uint16_t h = Sys.useRoundRect ? numVisible * 10 - 1 : numVisible * 10 + 1;
  drawScrollBar(118, y, _menuItemCount, contextMenuTopItem, numVisible, h);
  
  //get the id of the item selected
  contextMenuSelectedItemID = 0xff;
  if(clickedButton == KEY_SELECT)
    contextMenuSelectedItemID = _menuItemIDs[contextMenuFocusedItem - 1];
}

#endif //UI_128X64

