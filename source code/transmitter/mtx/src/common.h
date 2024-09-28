#ifndef _COMMON_H_
#define _COMMON_H_

void resetSystemParams();

void resetModelName();
void resetModelParams();

void resetMixerParams();
void resetMixerParams(uint8_t idx);

void resetChannelParams();
void resetChannelParams(uint8_t idx);

void resetTelemParams();
void resetTelemParams(uint8_t idx);

void resetCustomCurveParams();
void resetCustomCurveParams(uint8_t idx);

void resetLogicalSwitchParams();
void resetLogicalSwitchParams(uint8_t idx);

void resetTimerParams();
void resetTimerParams(uint8_t idx);
void resetTimerRegisters();
void resetTimerRegister(uint8_t idx);
void restoreTimerRegisters();

void resetCounterParams();
void resetCounterParams(uint8_t idx);
void resetCounterRegisters();
void resetCounterRegister(uint8_t idx);
void restoreCounterRegisters();

void resetFuncgenParams();
void resetFuncgenParams(uint8_t idx);

void resetNotificationParams();
void resetNotificationParams(uint8_t idx);

void resetWidgetParams();
void resetWidgetParams(uint8_t idx);

bool changeLSReference(uint8_t newRef, uint8_t oldRef);

bool verifyModelData();

uint16_t joinBytes(uint8_t highByte, uint8_t lowByte);

bool isEmptyStr(char* buff, uint8_t lenBuff);
void trimWhiteSpace(char* buff, uint8_t lenBuff);

//====================== MISC =====================================================================

//---- Output channels -------------------

/* NOTE: Changing these will need updating the stx and receiver code,
as well as the schematics and all related stuff */
#define NUM_RC_CHANNELS  20
#define MAX_CHANNELS_PER_RECEIVER 10

extern int16_t  channelOut[NUM_RC_CHANNELS];  //Range is -500 to 500

//---- Sticks and pots -------------------

// Leave as is unless you've physically added or removed the corresponding pins in the schematics
// If you must change this, you also need to add the axis names manually
#define NUM_STICK_AXES 10
#define NUM_KNOBS 2

extern int16_t stickAxisIn[NUM_STICK_AXES];
extern int16_t knobIn[NUM_KNOBS];

extern bool isCalibratingControls;
enum {
  //calibration stages
  CALIBRATE_INIT,
  CALIBRATE_MOVE,
  CALIBRATE_DEADBAND
};

//---- Physical Switches -----------------

/* Leave as is unless you've physically added or removed the corresponding pins in the schematics */
#define NUM_PHYSICAL_SWITCHES  8 

extern uint8_t swState[NUM_PHYSICAL_SWITCHES];
enum {  
  //Switch states
  SWUPPERPOS = 0, 
  SWLOWERPOS = 1, 
  SWMIDPOS   = 2
};

//---- Buttons and button events ----------

enum {
  KEY_SELECT = 1, 
  KEY_UP, 
  KEY_DOWN,
  
  KEY_X1_TRIM_UP,
  KEY_X1_TRIM_DOWN,
  KEY_Y1_TRIM_UP,
  KEY_Y1_TRIM_DOWN,
  KEY_X2_TRIM_UP,
  KEY_X2_TRIM_DOWN,
  KEY_Y2_TRIM_UP,
  KEY_Y2_TRIM_DOWN,
  KEY_TRIM_FIRST = KEY_X1_TRIM_UP,
  KEY_TRIM_LAST = KEY_Y2_TRIM_DOWN
};

extern uint8_t  buttonCode; 

extern uint32_t buttonStartTime;
extern uint32_t buttonReleaseTime;

extern uint32_t heldButtonEntryLoopNum;

//Button events
#define LONGPRESSDELAY 350
extern uint8_t  pressedButton; //triggered once when the button goes down
extern uint8_t  clickedButton; //triggered when the button is released before heldButton event
extern uint8_t  heldButton;    //triggered when button is held down long enough

//---- Transmitter Battery -----------------

extern int16_t  battVoltsNow; //millivolts
extern uint8_t  battState;

enum {
  BATTLOW, 
  BATTHEALTY
};

//---- Audio ------------------------------

#define NOTIFICATION_TONE_COUNT 5

enum {  
  AUDIO_NONE, 
  AUDIO_BATTERY_WARN, 
  AUDIO_SAFETY_WARN, 
  AUDIO_TELEM_WARN,
  AUDIO_TELEM_MUTE_CHANGED,
  AUDIO_INACTIVITY, 
  AUDIO_TIMER_ELAPSED,
  AUDIO_BIND_SUCCESS,
  AUDIO_SCREENSHOT_CAPTURED,
  AUDIO_SWITCH_MOVED,
  AUDIO_KEY_PRESSED,

  AUDIO_TRIM_MOVED,
  AUDIO_TRIM_CENTER,
  AUDIO_TRIM_MODE_ENTERED,
  AUDIO_TRIM_MODE_EXITED,
  AUDIO_TRIM_MODE_X1,
  AUDIO_TRIM_MODE_Y1,
  AUDIO_TRIM_MODE_X2,
  AUDIO_TRIM_MODE_Y2,

  AUDIO_NOTIFICATION_TONE_FIRST,
  AUDIO_NOTIFICATION_TONE_LAST = AUDIO_NOTIFICATION_TONE_FIRST + NOTIFICATION_TONE_COUNT - 1,
};

extern uint8_t  audioToPlay;

extern int16_t  audioTrimVal; //fixed point representation with scaling factor 1/10

//---- Misc -------------------------------

extern uint8_t  maxNumOfModels;

extern uint32_t inputsLastMoved; //inactivity detection
extern bool     backlightIsOn;

extern uint8_t  activeFmdIdx; 

//---- Receiver related -----

extern uint8_t  transmitterPacketRate;
extern uint8_t  receiverPacketRate;

extern bool     isRequestingBind;
extern uint8_t  bindStatusCode;  //1 on success, 2 on fail
extern bool     isMainReceiver;

extern uint8_t  outputChConfig[NUM_RC_CHANNELS];
extern bool     gotOutputChConfig;
extern bool     isRequestingOutputChConfig;
extern bool     isSendOutputChConfig;
extern uint8_t  receiverConfigStatusCode; //1 on success, 2 on fail

enum {
  SIGNAL_TYPE_DIGITAL = 0,
  SIGNAL_TYPE_SERVOPWM = 1,
  SIGNAL_TYPE_PWM = 2
};

//---- Telemetry --------------------------

#define NUM_CUSTOM_TELEMETRY  6

extern int16_t  telemetryReceivedValue[NUM_CUSTOM_TELEMETRY]; //stores the raw received telemetry
extern int16_t  telemetryMaxReceivedValue[NUM_CUSTOM_TELEMETRY]; //for stats
extern int16_t  telemetryMinReceivedValue[NUM_CUSTOM_TELEMETRY]; //for stats
extern int16_t  telemetryLastReceivedValue[NUM_CUSTOM_TELEMETRY];
extern uint32_t telemetryLastReceivedTime[NUM_CUSTOM_TELEMETRY]; 
extern uint8_t  telemetryAlarmState[NUM_CUSTOM_TELEMETRY];
extern bool     telemetryMuteAlarms;
extern bool     telemetryForceRequest;

#define TELEMETRY_NO_DATA  0x7FFF

//---- Main loop control -------------------

#define fixedLoopTime  20
/* in milliseconds. Should be greater than the time taken by radio module to transmit the 
entire packet or else the window is missed resulting in much less throughput. 
It should also be close to the average worst case time to do the main loop i.e when all features 
are active (mixers, logical switches, ui, etc).
The main loop time can be displayed by holding the select key while powering on.
*/

extern uint32_t thisLoopNum; //main loop counter

//---- Debug -------------------------------
extern uint32_t DBG_loopTime;

extern uint8_t screenshotSwtch;

//====================== SYSTEM PARAMETERS =========================================================

/* 
  NOTES & WARNINGS
  - The odering in the enumerations should match the ordering in the UI.
  - Adding new fields or parameters can corrupt the system data. This is no big deal however.
*/

//------------------------------------------------
// structure for stick axis data
//------------------------------------------------

typedef struct {
  int16_t minVal; 
  int16_t centerVal; 
  int16_t maxVal;
  uint8_t deadzone;
  uint8_t type;
} stick_axis_params_t;

enum {
  //stick axis types
  STICK_AXIS_SELF_CENTERING,
  STICK_AXIS_NON_CENTERING,
  STICK_AXIS_ABSENT,
  
  STICK_AXIS_TYPE_COUNT
};

typedef stick_axis_params_t knob_params_t;

enum {
  //type of knob
  KNOB_CENTER_DETENT,
  KNOB_NO_CENTER_DETENT,
  KNOB_ABSENT,

  KNOB_TYPE_COUNT
};

//================================================
// Structure for the entire system data. This is
// also the data we store to the eeprom.
//================================================

typedef struct {
  uint8_t  activeModelIdx;

  //--- sticks axes
  stick_axis_params_t StickAxis[NUM_STICK_AXES];

  //---- knobs
  knob_params_t Knob[NUM_KNOBS];

  //--- stick mode
  //used for default assignment when creating new models
  uint8_t  defaultStickMode;

  //--- switches
  uint8_t  swType[NUM_PHYSICAL_SWITCHES];
  
  //--- battery
  int16_t  battVoltsMin; //millivolts
  int16_t  battVoltsMax; //millivolts
  int16_t  battVfactor;  //ADC scaling factor
  
  //--- security
  bool     lockStartup;
  bool     lockModels;
  char     password[5]; //4 chars + null. Just enough

  //--- rf
  bool     rfEnabled;
  uint8_t  rfPower;   //3 levels. Low, Medium, Max

  //--- sound
  bool     soundEnabled; 
  uint8_t  inactivityMinutes;
  bool     soundSwitches;
  bool     soundKnobCenter;
  bool     soundKeys;
  bool     soundTrims;
  uint8_t  trimToneFreqMode;

  //--- backlight
  bool     backlightEnabled; 
  uint8_t  backlightLevel;    //in percentage
  uint8_t  backlightTimeout;
  uint8_t  backlightWakeup;
  bool     backlightSuppressFirstKey; //better name
  uint8_t  contrast;
  
  //--- appearance
  bool     showMenuIcons;
  bool     rememberMenuPosition;
  bool     useRoundRect;
  bool     useDenserMenus;
  bool     animationsEnabled;
  bool     autohideTrims;
  bool     useNumericalBatteryIndicator;
  bool     showSplashScreen;
  bool     showWelcomeMsg;
  uint8_t  scrollBarStyle;
  
  //--- misc
  bool     autoSelectMovedControl;
  bool     mixerTemplatesEnabled;
  uint8_t  defaultChannelOrder;
  bool     onscreenTrimEnabled;
  
  //--- debug
  bool     DBG_showLoopTime;
  bool     DBG_simulateTelemetry; //fakes telemetry value on sensor ID 0x30
  bool     DBG_disableInterlacing; 

  //--- screenshots
  uint16_t screenshotSeqNo;
  
} sys_params_t;

extern sys_params_t Sys;

enum {
  SW_ABSENT,
  SW_2POS,
  SW_3POS,
  
  SW_TYPE_COUNT
};

enum {
  RF_POWER_LOW,
  RF_POWER_MEDIUM,
  RF_POWER_MAX,
  
  RF_POWER_COUNT
};

enum {
  TRIM_TONE_FREQ_FIXED,
  TRIM_TONE_FREQ_VARIABLE,

  TRIM_TONE_FREQ_MODE_COUNT
};

enum { 
  BACKLIGHT_WAKEUP_KEYS, 
  BACKLIGHT_WAKEUP_ACTIVITY, 
  
  BACKLIGHT_WAKEUP_COUNT
};

enum {
  BACKLIGHT_TIMEOUT_5SEC,
  BACKLIGHT_TIMEOUT_15SEC,
  BACKLIGHT_TIMEOUT_1MIN,
  BACKLIGHT_TIMEOUT_2MIN,
  BACKLIGHT_TIMEOUT_5MIN,
  BACKLIGHT_TIMEOUT_10MIN,
  BACKLIGHT_TIMEOUT_30MIN,
  BACKLIGHT_TIMEOUT_NEVER,
  
  BACKLIGHT_TIMEOUT_COUNT
};

enum {
  //ordered x1y1x2y2
  STICK_MODE_RTAE,
  STICK_MODE_AERT,
  STICK_MODE_REAT,
  STICK_MODE_ATRE,
  
  STICK_MODE_COUNT
};

//====================== MODEL PARAMETERS ==========================================================
/* 
  NOTES & WARNINGS
  - The odering in the enumerations should match the ordering in the UI.
  - Adding new fields/parameters or reordering the structures can corrupt model data. Always make 
    a backup of the models you care about to an SD card before updating the device's firmware.
  - If you have added new fields, remember to update the model import and export code for the SD card,
    as well as the changeLSReference() function.

*/

//------------------------------------------------
// structure for channel params
//------------------------------------------------

typedef struct {
  char     name[7];       //6 chars + null
  int8_t   curve;         //-1 means none, 0 to .. are custom curves
  bool     reverse; 
  int16_t  subtrim;       //fixed point representation with scaling factor 1/10
  uint8_t  overrideSwitch;
  int8_t   overrideVal; 
  int8_t   failsafe;      //-102 to 100. -102 means hold, -101 No pulse
  int8_t   endpointL;     //left endpoint, -100 to 0
  int8_t   endpointR;     //right endpoint, 0 to 100
} channel_params_t;

//------------------------------------------------
// structure for custom telemetry
//------------------------------------------------

typedef struct {
  char     name[9];        //8 chars + Null. If no name, the telemetry doesn't then exist
  char     unitsName[6];   //5 chars + null
  uint8_t  identifier;     //ID 00 to FE. FF is unused
  int16_t  multiplier;     //1 to 1000, scales to 0.01 to 10.00
  int8_t   factor10;       //-2, 1, 0, 1, 2. Means x10^
  int16_t  offset;         //-30000 to 30000
  bool     logEnabled;     //
  uint8_t  alarmCondition; //None, >Thresh, <Thresh
  int16_t  alarmThreshold; //-30000 to 30000
  uint8_t  alarmMelody;
  bool     showOnHome; 
  bool     recordMaximum;
  bool     recordMinimum;
} telemetry_params_t;

enum {
  TELEMETRY_ALARM_CONDITION_NONE,
  TELEMETRY_ALARM_CONDITION_GREATER_THAN,
  TELEMETRY_ALARM_CONDITION_LESS_THAN,
  
  TELEMETRY_ALARM_CONDITION_COUNT
};

////----------------------------------------------
// structure for dual rates and expo
//------------------------------------------------

typedef struct {
  int8_t   rate1;  // 0 to 100
  int8_t   rate2;  // 0 to 100
  int8_t   expo1;  // -100 to 100
  int8_t   expo2;  // -100 to 100
  uint8_t  swtch;
} rate_expo_t;

//------------------------------------------------
// structure for function generator data
//------------------------------------------------

typedef struct {
  uint8_t  waveform;
  union {
    uint8_t  periodMode;   //for waveforms sine, square, triangle, sawtooth, and random
    uint8_t  widthMode;    //for pulse waveform
  };
  //time is in tenths of a second
  union {
    uint16_t period1; 
    uint16_t width; //for pulse waveform
  };
  union {
    uint16_t period2; 
    uint16_t period;  //for pulse waveform
  };
  uint8_t  modulatorSrc;
  bool     reverseModulator;
  uint8_t  phaseMode; 
  uint16_t phase;

} funcgen_t;

#define NUM_FUNCGEN 5

enum { 
  FUNCGEN_WAVEFORM_SINE,
  FUNCGEN_WAVEFORM_SQUARE,
  FUNCGEN_WAVEFORM_TRIANGLE,
  FUNCGEN_WAVEFORM_SAWTOOTH,
  FUNCGEN_WAVEFORM_PULSE,
  FUNCGEN_WAVEFORM_RANDOM,
  
  FUNCGEN_WAVEFORM_COUNT
};

enum { 
  FUNCGEN_PHASEMODE_AUTO, //automatic phase compensation for smoothly transitioning to new period
  FUNCGEN_PHASEMODE_FIXED, //no compensation
  
  FUNCGEN_PHASEMODE_COUNT
};

enum { 
  FUNCGEN_PERIODMODE_VARIABLE,
  FUNCGEN_PERIODMODE_FIXED, 
  
  FUNCGEN_PERIODMODE_COUNT
};

enum {
  FUNCGEN_PULSE_WIDTH_VARIABLE,
  FUNCGEN_PULSE_WIDTH_FIXED,

  FUNCGEN_PULSE_WIDTH_MODE_COUNT
};

//-----------------------------------------------
// structure for custom curve data
//-----------------------------------------------

#define MIN_NUM_POINTS_CUSTOM_CURVE     2
#define DEFAULT_NUM_POINTS_CUSTOM_CURVE 5
#define MAX_NUM_POINTS_CUSTOM_CURVE     10

typedef struct {
  char    name[6]; //5 chars + Null
  uint8_t numPoints; 
  int8_t  xVal[MAX_NUM_POINTS_CUSTOM_CURVE];
  int8_t  yVal[MAX_NUM_POINTS_CUSTOM_CURVE];
  bool    smooth;
} custom_curve_t;

#define NUM_CUSTOM_CURVES 10

//------------------------------------------------
// structure for logical switch data
//------------------------------------------------

typedef struct {
  uint8_t  func;
  int16_t  val1;
  int16_t  val2;
  int16_t  val3;
  int16_t  val4;
} logical_switch_t;

#define NUM_LOGICAL_SWITCHES 20

enum { 
  LS_FUNC_NONE,
  LS_FUNC_A_GREATER_THAN_X,
  LS_FUNC_A_LESS_THAN_X,
  LS_FUNC_A_EQUAL_X,
  LS_FUNC_A_GREATER_THAN_OR_EQUAL_X,
  LS_FUNC_A_LESS_THAN_OR_EQUAL_X,
  LS_FUNC_GROUP1_LAST = LS_FUNC_A_LESS_THAN_OR_EQUAL_X,

  LS_FUNC_ABS_A_GREATER_THAN_X,
  LS_FUNC_ABS_A_LESS_THAN_X,
  LS_FUNC_ABS_A_EQUAL_X,
  LS_FUNC_ABS_A_GREATER_THAN_OR_EQUAL_X,
  LS_FUNC_ABS_A_LESS_THAN_OR_EQUAL_X,
  LS_FUNC_GROUP2_LAST = LS_FUNC_ABS_A_LESS_THAN_OR_EQUAL_X,

  LS_FUNC_ABS_DELTA_GREATER_THAN_X,
  LS_FUNC_GROUP3_LAST = LS_FUNC_ABS_DELTA_GREATER_THAN_X,
  
  LS_FUNC_A_GREATER_THAN_B,
  LS_FUNC_A_LESS_THAN_B,
  LS_FUNC_A_EQUAL_B,
  LS_FUNC_A_GREATER_THAN_OR_EQUAL_B,
  LS_FUNC_A_LESS_THAN_OR_EQUAL_B,
  LS_FUNC_GROUP4_LAST = LS_FUNC_A_LESS_THAN_OR_EQUAL_B,
  
  LS_FUNC_AND,
  LS_FUNC_OR,
  LS_FUNC_XOR,
  LS_FUNC_LATCH,
  LS_FUNC_GROUP5_LAST = LS_FUNC_LATCH,
  
  LS_FUNC_TOGGLE,
  LS_FUNC_GROUP6_LAST = LS_FUNC_TOGGLE,
  
  LS_FUNC_PULSE,
  LS_FUNC_GROUP7_LAST = LS_FUNC_PULSE,
  
  LS_FUNC_COUNT
};

//-----------------------------------------------
// structure for mixer data
//-----------------------------------------------

typedef struct {
  char     name[7];    //6 chars + Null 
  uint8_t  output;     //index in mix sources array
  uint8_t  operation; 
  uint8_t  swtch; 
  uint8_t  input;      //index in mix sources array
  int8_t   weight;     //-100 to 100
  int8_t   offset;     //-100 to 100
  uint8_t  curveType;
  int8_t   curveVal;
  bool     trimEnabled;
  uint8_t  flightMode; //each bit corresponds to a flight mode
  uint16_t delayUp;
  uint16_t delayDown;
  uint16_t slowUp;
  uint16_t slowDown;
} mixer_params_t;

#define NUM_MIXSLOTS 40

enum { 
  MIX_ADD,
  MIX_MULTIPLY,
  MIX_REPLACE,
  MIX_HOLD,
  
  MIX_OPERATOR_COUNT
};

enum { 
  MIX_CURVE_TYPE_DIFF,
  MIX_CURVE_TYPE_EXPO,
  MIX_CURVE_TYPE_FUNCTION,
  MIX_CURVE_TYPE_CUSTOM,
  
  MIX_CURVE_TYPE_COUNT
};

enum {
  MIX_CURVE_FUNC_NONE,
  MIX_CURVE_FUNC_X_GREATER_THAN_ZERO,
  MIX_CURVE_FUNC_X_LESS_THAN_ZERO,
  MIX_CURVE_FUNC_ABS_X,
  
  MIX_CURVE_FUNC_COUNT
};

//------------------------------------------------
// structure for trim parameters
//------------------------------------------------

typedef struct {
  uint8_t trimState;
  int16_t  commonTrim; //fixed point representation with scaling factor 1/10
} trim_params_t;

enum {
  TRIM_DISABLED,
  TRIM_COMMON,
  TRIM_FLIGHT_MODE,
  
  TRIM_STATE_COUNT
};

//fixed point representation with scaling factor 1/10
#define TRIM_MAX_VAL  200
#define TRIM_MIN_VAL  (-TRIM_MAX_VAL)

//------------------------------------------------
// structure for flight mode data
//------------------------------------------------

typedef struct {
  char     name[7];  //6 chars + Null 
  uint8_t  swtch;
  int16_t   x1Trim; //fixed point representation with scaling factor 1/10
  int16_t   y1Trim; //fixed point representation with scaling factor 1/10
  int16_t   x2Trim; //fixed point representation with scaling factor 1/10
  int16_t   y2Trim; //fixed point representation with scaling factor 1/10
  uint8_t  transitionTime;
} flight_mode_t;

#define NUM_FLIGHT_MODES 5

//------------------------------------------------
// structure for counter data
//------------------------------------------------

typedef struct {
  char     name[7];   //6 chars + Null 
  uint8_t  clock;     //the control switch that clocks the counter
  uint8_t  edge;      //0 rising, 1 falling, 2 dual
  uint8_t  clear;     //the control switch that clears the counter
  int16_t  modulus;   //2 to 10000
  uint8_t  direction; //0 count upwards, 1 count downwards
  bool     isPersistent;
  int16_t  persistVal;
} counter_params_t;

#define NUM_COUNTERS  5

extern int16_t counterOut[NUM_COUNTERS];

//------------------------------------------------
// structure for timer parameters
//------------------------------------------------

typedef struct {
  char     name[7]; //6 chars + null
  uint8_t  swtch;    //switch that starts or stops the timer
  uint8_t  resetSwitch;  //
  uint8_t  initialMinutes;  //if 0, timer will count up, else count down
  bool     isPersistent; 
  uint32_t persistVal;
} timer_params_t;

#define NUM_TIMERS 3

extern uint32_t timerElapsedTime[NUM_TIMERS];
extern uint32_t timerLastElapsedTime[NUM_TIMERS];
extern uint32_t timerLastPaused[NUM_TIMERS];
extern bool     timerIsRunning[NUM_TIMERS];
extern bool     timerForceRun[NUM_TIMERS];

//------------------------------------------------
// structure for home screen widget data
//------------------------------------------------

typedef struct {
  uint8_t type;
  uint8_t src;
  uint8_t disp;
  int16_t gaugeMin;
  int16_t gaugeMax;
} widget_params_t;

#define NUM_WIDGETS  4

enum {
  WIDGET_TYPE_TELEMETRY,
  WIDGET_TYPE_MIXSOURCES,
  WIDGET_TYPE_OUTPUTS,
  WIDGET_TYPE_TIMERS,
  WIDGET_TYPE_COUNTERS,
  
  WIDGET_TYPE_COUNT
};

enum {
  WIDGET_SRC_AUTO = NUM_CUSTOM_TELEMETRY,
};

enum {
  WIDGET_DISP_NUMERICAL,
  WIDGET_DISP_GAUGE,
  WIDGET_DISP_GAUGE_ZERO_CENTERED,
  
  WIDGET_DISP_COUNT
};

//------------------------------------------------
// structure for custom notifications
//------------------------------------------------

typedef struct {
  bool    enabled;
  uint8_t swtch;
  uint8_t tone;
  char    text[13]; //12 chars + Null
} notification_params_t;

#define NUM_CUSTOM_NOTIFICATIONS  10

///===============================================
/// STRUCTURE FOR ENTIRE MODEL INFORMATION.
/// THIS NESTS THE PREVIOUS STRUCTURES 
/// IT IS ALSO THE DATA WE STORE TO EEPROM.
///===============================================

typedef struct {
  
  //--- Model name 
  //This is the first member in structure
  char name[9]; //8 chars + Null. The SD library uses 8.3 naming so this should be left as is.
  
  //---Model type 
  //This should be the second member in structure
  uint8_t type;
  
  //--- Raw sources for rud, thr, ail, ele inputs
  uint8_t rudSrcRaw;
  uint8_t ailSrcRaw;
  uint8_t eleSrcRaw;
  uint8_t thrSrcRaw;
  
  //--- Timer
  timer_params_t Timer[NUM_TIMERS];

  //--- Trims
  trim_params_t X1Trim; 
  trim_params_t Y1Trim; 
  trim_params_t X2Trim; 
  trim_params_t Y2Trim;

  uint8_t trimStep;
  
  //--- Rates and expo
  rate_expo_t RudDualRate;
  rate_expo_t AilDualRate; 
  rate_expo_t EleDualRate;
  
  //--- Throttle curve
  custom_curve_t ThrottleCurve;

  //--- Function generator
  funcgen_t Funcgen[NUM_FUNCGEN];
  
  //--- Mixers
  mixer_params_t Mixer[NUM_MIXSLOTS];
  
  //--- Custom curves
  custom_curve_t CustomCurve[NUM_CUSTOM_CURVES];
  
  //--- Logical switches
  logical_switch_t LogicalSwitch[NUM_LOGICAL_SWITCHES];
  
  //--- Counters
  counter_params_t Counter[NUM_COUNTERS];
  
  //--- Flight modes
  flight_mode_t FlightMode[NUM_FLIGHT_MODES];
  
  //--- Safety checks
  bool   checkThrottle;
  int8_t switchWarn[NUM_PHYSICAL_SWITCHES]; //-1 means no checks
  
  //--- Notifications
  notification_params_t CustomNotification[NUM_CUSTOM_NOTIFICATIONS];
  
  //--- Output channels
  channel_params_t Channel[NUM_RC_CHANNELS];
  
  //--- Telemetry
  telemetry_params_t Telemetry[NUM_CUSTOM_TELEMETRY];
  
  //--- Widgets
  widget_params_t Widget[NUM_WIDGETS];
  
} model_params_t;

extern model_params_t Model; 

enum {
  MODEL_TYPE_AIRPLANE,
  MODEL_TYPE_MULTICOPTER,
  MODEL_TYPE_OTHER,
  
  MODEL_TYPE_COUNT
};

enum {
  TRIM_STEP_COARSE,
  TRIM_STEP_MEDIUM,
  TRIM_STEP_FINE,

  TRIM_STEP_COUNT
};


#define NUM_VIRTUAL_CHANNELS  5

enum {
  //mix sources

  SRC_NONE,
  
  SRC_RUD,
  SRC_THR, 
  SRC_AIL, 
  SRC_ELE, 
  
  SRC_STICK_AXIS_FIRST,
  SRC_STICK_AXIS_LAST = SRC_STICK_AXIS_FIRST + NUM_STICK_AXES - 1,

  SRC_X1_AXIS = SRC_STICK_AXIS_FIRST,
  SRC_Y1_AXIS,
  SRC_Z1_AXIS,
  SRC_X2_AXIS,
  SRC_Y2_AXIS,
  SRC_Z2_AXIS,
  SRC_X3_AXIS,
  SRC_Y3_AXIS,
  SRC_X4_AXIS,
  SRC_Y4_AXIS,

  SRC_KNOB_FIRST,
  SRC_KNOB_LAST = SRC_KNOB_FIRST + NUM_KNOBS - 1,
  
  SRC_RAW_ANALOG_FIRST = SRC_STICK_AXIS_FIRST,
  SRC_RAW_ANALOG_LAST = SRC_KNOB_LAST,
  
  SRC_100PERC, 
  
  SRC_FUNCGEN_FIRST,
  SRC_FUNCGEN_LAST = SRC_FUNCGEN_FIRST + NUM_FUNCGEN - 1,

  SRC_X1_TRIM,
  SRC_Y1_TRIM,
  SRC_X2_TRIM,
  SRC_Y2_TRIM,
  
  SRC_SW_PHYSICAL_FIRST,
  SRC_SW_PHYSICAL_LAST = SRC_SW_PHYSICAL_FIRST + NUM_PHYSICAL_SWITCHES - 1,
  
  SRC_SW_LOGICAL_FIRST,
  SRC_SW_LOGICAL_LAST = SRC_SW_LOGICAL_FIRST + NUM_LOGICAL_SWITCHES - 1,
  
  SRC_CH1,
  SRC_VIRTUAL_FIRST = SRC_CH1 + NUM_RC_CHANNELS,
  SRC_VIRTUAL_LAST  = SRC_VIRTUAL_FIRST + NUM_VIRTUAL_CHANNELS - 1,  
  
  MIXSOURCES_COUNT,

  //other sources

  SRC_COUNTER_FIRST = MIXSOURCES_COUNT,
  SRC_COUNTER_LAST = SRC_COUNTER_FIRST + NUM_COUNTERS - 1,

  SRC_TIMER_FIRST,
  SRC_TIMER_LAST = SRC_TIMER_FIRST + NUM_TIMERS - 1,

  SRC_TELEMETRY_FIRST,
  SRC_TELEMETRY_LAST = SRC_TELEMETRY_FIRST + NUM_CUSTOM_TELEMETRY - 1,

  SRC_INACTIVITY_TIMER,
  SRC_TX_BATTERY_VOLTAGE,

  TOTAL_SOURCES_COUNT
};


enum { 
  CTRL_SW_NONE,

  //physical switches
  //up, mid, down, !up, !mid, !down
  CTRL_SW_PHYSICAL_FIRST,
  CTRL_SW_PHYSICAL_LAST = CTRL_SW_PHYSICAL_FIRST + (NUM_PHYSICAL_SWITCHES * 6) - 1,
  
  //logical switches
  CTRL_SW_LOGICAL_FIRST,
  CTRL_SW_LOGICAL_LAST = CTRL_SW_LOGICAL_FIRST + NUM_LOGICAL_SWITCHES - 1,
  CTRL_SW_LOGICAL_FIRST_INVERT,
  CTRL_SW_LOGICAL_LAST_INVERT = CTRL_SW_LOGICAL_FIRST_INVERT + NUM_LOGICAL_SWITCHES - 1,
  
  //flight modes as switches
  CTRL_SW_FMD_FIRST,
  CTRL_SW_FMD_LAST = CTRL_SW_FMD_FIRST + NUM_FLIGHT_MODES - 1,
  CTRL_SW_FMD_FIRST_INVERT,
  CTRL_SW_FMD_LAST_INVERT = CTRL_SW_FMD_FIRST_INVERT + NUM_FLIGHT_MODES - 1,
  
  CTRL_SW_COUNT
};


#endif

