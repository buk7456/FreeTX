#ifndef _UI_128x64_H_
#define _UI_128x64_H_

// The #include are put here for convenience, such that we can avoid re-specifying them in every ui file.
#include "Arduino.h"
#include "../../../config.h"
#include "../../common.h"
#include "../../stringDefs.h"
#include "../../crc.h"
#include "../../inputs.h"
#include "../../mathHelpers.h"
#include "../../mixer.h"
#include "../../mtx.h"
#include "../../templates.h"
#include "../../tonePlayer.h"
#include "../../ee/eestore.h"
#include "../../sd/sdStore.h"
#include "../bitmaps.h"
#include "../about.h"
#include "../ui.h"
#include "../uiCommon.h"

//---------------------------- Main UI states ------------------------------------------------------

enum {
  //--- Home screen and related ---
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
  DIALOG_COUNTER_TYPE,
  DIALOG_COPY_COUNTER,
  CONFIRMATION_CLEAR_ALL_COUNTERS,
  SCREEN_COUNTER_OUTPUTS,
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
  SCREEN_TELEMETRY,
  CONTEXT_MENU_ACTIVE_SENSOR,
  CONTEXT_MENU_FREE_SENSOR,
  SCREEN_CREATE_SENSOR,
  CONTEXT_MENU_SENSOR_TEMPLATES,
  SCREEN_SENSOR_STATISTICS,
  SCREEN_EDIT_SENSOR,
  CONFIRMATION_DELETE_SENSOR,
  SCREEN_TELEMETRY_GNSS,
  SCREEN_SELECT_GNSS_UNITS,
  
  //---- System settings ----
  SCREEN_SYSTEM_MENU,
  SCREEN_RF,
  SCREEN_SOUND,
  SCREEN_BACKLIGHT,
  SCREEN_APPEARANCE,
  SCREEN_MISCELLANEOUS,
  SCREEN_UNLOCK_ADVANCED_MENU,
  SCREEN_ADVANCED_MENU,
  SCREEN_STICKS,
  SCREEN_KNOBS,
  SCREEN_SWITCHES,
  SCREEN_BATTERY,
  SCREEN_SECURITY,
  SCREEN_DEBUG,
  SCREEN_DEBUG_STATISTICS,
  SCREEN_INTERNAL_EEPROM_DUMP,
  SCREEN_EXTERNAL_EEPROM_DUMP,
  SCREEN_CHARACTER_SET,
  SCREEN_SCREENSHOT_CONFIG,
  DIALOG_ADJUST_LONG_PRESS_DELAY,
  DIALOG_ADJUST_KEY_REPEAT_INTERVAL,
  CONFIRMATION_BACKUP_SYSTEM_SETTINGS,
  CONFIRMATION_RESTORE_SYSTEM_SETTINGS,
  CONFIRMATION_FACTORY_RESET,
  SCREEN_ABOUT,
  SCREEN_EASTER_EGG,
  
  //---- Receiver ----
  SCREEN_RECEIVER,
  SCREEN_RECEIVER_CONFIG,
  SCREEN_RECEIVER_BINDING,
  
  //---- Generic text viewer ----
  SCREEN_TEXT_VIEWER,

  TOTAL_SCREEN_COUNT
};

//---------------------------- Function declarations -----------------------------------------------

//Main UI handlers
void ui_handler_home();
void ui_handler_main_menu();
void ui_handler_model_manager();
void ui_handler_inputs();
void ui_handler_mixer();
void ui_handler_outputs();
void ui_handler_extras_menu();
void ui_handler_custom_curves();
void ui_handler_function_generators();
void ui_handler_logical_switches();
void ui_handler_timers();
void ui_handler_counters();
void ui_handler_custom_notifications();
void ui_handler_safety_checks();
void ui_handler_trim_setup();
void ui_handler_flight_modes();
void ui_handler_telemetry();
void ui_handler_system_settings();
void ui_handler_receiver();
void ui_handler_text_viewer();

void handleBatteryWarningUI();

void changeToScreen(uint8_t scrn);
void changeFocusOnUpDown(uint8_t numItems);
void toggleEditModeOnSelectClicked();
void drawCursor(uint8_t xpos, uint8_t ypos);
void drawHeader(const char* str);
void drawHeader_Menu(const char* str);
void drawSubheader(const char* str, uint8_t ypos);
void drawDottedVLine(uint8_t x, uint8_t y, uint8_t len, uint8_t fgColor, uint8_t bgColor);
void drawDottedHLine(uint8_t x, uint8_t y, uint8_t len, uint8_t fgColor, uint8_t bgColor);
void drawBoundingBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void drawScrollBar(uint8_t xpos, uint8_t ypos, uint16_t numItems, uint16_t topItem, uint16_t numVisible, uint16_t viewportHeight);
void drawCheckbox(uint8_t xpos, uint8_t ypos, bool val);
void drawLoaderSpinner(uint8_t xpos, uint8_t ypos, uint8_t size);
void drawAnimatedSprite(uint8_t x, uint8_t y, const uint8_t* const bitmapTable[], uint8_t w, uint8_t h, uint8_t color, 
                        uint8_t frameCount, uint16_t frameTime, uint32_t timeOffset, bool forceAnimation);
void drawHorizontalBarChart(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, int32_t val, int32_t valMin, int32_t valMax);
void drawHorizontalBarChartZeroCentered(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, int32_t val, int32_t range);
void drawTrimSliders();
void drawDialogCopyMove(const char* str, uint8_t srcIdx, uint8_t destIdx, bool isCopy);
void drawCustomCurve(custom_curve_t *crv, uint8_t selectPt, uint8_t src);
void drawMixerCurvePreview(mixer_params_t* mxr, bool showCrossHairs);
void drawToast();
void drawTooltip(uint8_t x, uint8_t y, char* str);
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

//---------------------------- Variable declarations -----------------------------------------------

//menu strings
extern char const mainMenu[][20] PROGMEM;
extern char const extrasMenu[][20] PROGMEM;
extern char const systemMenu[][20] PROGMEM;
extern char const advancedMenu[][20] PROGMEM;

enum {
  MAIN_MENU_MODEL, MAIN_MENU_INPUTS, MAIN_MENU_MIXER, MAIN_MENU_OUTPUTS, MAIN_MENU_EXTRAS,
  MAIN_MENU_TELEMETRY, MAIN_MENU_SYSTEM, MAIN_MENU_RECEIVER
};

enum {
  EXTRAS_MENU_CUSTOM_CURVES, EXTRAS_MENU_FUNCTION_GENERATORS, EXTRAS_MENU_LOGICAL_SWITCHES,
  EXTRAS_MENU_TIMERS, EXTRAS_MENU_COUNTERS, EXTRAS_MENU_NOTIFICATION_SETUP, EXTRAS_MENU_SAFETY_CHECKS,
  EXTRAS_MENU_TRIM_SETUP, EXTRAS_MENU_FLIGHT_MODES
};

enum {
  SYSTEM_MENU_RF, SYSTEM_MENU_SOUND, SYSTEM_MENU_BACKLIGHT, SYSTEM_MENU_APPEARANCE, 
  SYSTEM_MENU_MISCELLANEOUS, SYSTEM_MENU_ADVANCED, SYSTEM_MENU_ABOUT,
};

enum {
  ADVANCED_MENU_STICKS, ADVANCED_MENU_KNOBS, ADVANCED_MENU_SWITCHES, 
  ADVANCED_MENU_BATTERY, ADVANCED_MENU_SECURITY, ADVANCED_MENU_DEBUG
};

extern char textBuff[32]; //generic buffer for working with strings. Leave at 32 as it's also reused to dump EEPROM.

extern uint8_t theScreen;
extern uint8_t lastScreen;

extern uint8_t focusedItem; 

extern bool isEditTextDialog;
extern bool isDisplayingBatteryWarning;

#define VISIBLE_MODELS_IN_MODEL_SCREEN 6

union SharedData {
  //string table to hold model names, for use in the model screen.
  char modelNameStr[VISIBLE_MODELS_IN_MODEL_SCREEN][sizeof(Model.name)];
  
  //buffer to hold name, for use with SD card model operations. Only 8.3 names supported. 
  char nameStr[13];
  
  //cache the graph y coordinates to avoid unnecessary computation.
  int8_t graphYCoord[51]; //51 points total to plot. 
};

extern union SharedData mySharedUnion; //TODO: Better name

extern bool graphYCoordinatesInvalid;

//menu
extern uint8_t menuSelectedItemID;

//context menu
extern uint8_t contextMenuTopItem;
extern uint8_t contextMenuFocusedItem;
extern uint8_t contextMenuSelectedItemID;

//Model
extern uint8_t thisModelIdx;

//mixer
extern uint8_t thisMixIdx;
extern uint8_t destMixIdx; 

//channel outputs
extern uint8_t thisChIdx;

//custom curves
extern uint8_t thisCrvIdx;
extern uint8_t destCrvIdx;
extern uint8_t thisCrvPt;

//function generators
extern uint8_t thisFgenIdx;
extern uint8_t destFgenIdx;

//logical switch
extern uint8_t thisLsIdx;
extern uint8_t destLsIdx;

//timers
extern uint8_t thisTimerIdx;

//counters
extern uint8_t thisCounterIdx;
extern uint8_t destCounterIdx;

//notifications
extern uint8_t thisNotificationIdx;
extern uint8_t destNotificationIdx;

//flight modes
extern uint8_t thisFmdIdx;

//telemetry 
extern uint8_t thisTelemIdx;

//widgets
extern uint8_t thisWidgetIdx;
extern uint8_t destWidgetIdx;

//others 
extern bool isRequestingSettingsRestore;
extern bool isRequestingStickCalibration;
extern bool isRequestingKnobCalibration;
extern bool isRequestingSwitchesSetup;
extern bool batteryGaugeCalibrated;
extern bool mainMenuLocked;
extern const char* textViewerText;

//onscreen trim
extern bool isOnscreenTrimMode;
extern uint8_t trimIdx;


#endif