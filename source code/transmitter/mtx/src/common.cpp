#include "Arduino.h"
#include "common.h"

#if !defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__)
  #error "Oops!  Make sure you have 'Arduino Mega' selected as board."
#endif

#if NUM_RC_CHANNELS < 8
  #error At least 8 channels
#elif NUM_RC_CHANNELS > 20
  #error At most 20 channels, else features might break.
#endif

#if MAX_CHANNELS_PER_RECEIVER < 8
  #error At least 8 receiver channels
#elif MAX_CHANNELS_PER_RECEIVER > NUM_RC_CHANNELS
  #error Receiver channels cannot exceed RC channels.
#endif

#if NUM_FLIGHT_MODES < 2
  #error At least 2 flight modes
#elif NUM_FLIGHT_MODES > 5
  #error At most 5 flight modes
#endif

#if NUM_FUNCGEN < 2
  #error At least 2 function generators
#elif NUM_FUNCGEN > 5
  #error At most 5 function generators
#endif

#if NUM_LOGICAL_SWITCHES < 4
  #error At least 4 logical switch
#elif NUM_LOGICAL_SWITCHES > 20
  #error At most 20 logical switches
#endif

#if NUM_CUSTOM_CURVES < 4
  #error At least 4 custom curve
#elif NUM_CUSTOM_CURVES > 20
  #error At most 20 custom curves
#endif

#if MAX_NUM_POINTS_CUSTOM_CURVE < 8
  #error At least 8 curve points
#elif MAX_NUM_POINTS_CUSTOM_CURVE > 20
  #error At most 20 curves points
#endif

#if MIN_NUM_POINTS_CUSTOM_CURVE < 2
  #error At least 2 curve points
#elif MIN_NUM_POINTS_CUSTOM_CURVE > MAX_NUM_POINTS_CUSTOM_CURVE
  #error Min num points cannot exceed max num points
#endif

#if DEFAULT_NUM_POINTS_CUSTOM_CURVE < MIN_NUM_POINTS_CUSTOM_CURVE
  #error Default num points cannot be less than min num points 
#elif DEFAULT_NUM_POINTS_CUSTOM_CURVE > MAX_NUM_POINTS_CUSTOM_CURVE
  #error Default num points cannot exceed max num points
#endif

#if NUM_COUNTERS < 2
  #error At least 2 counters
#elif NUM_COUNTERS > 5
  #error At most 5 counters
#endif

#if NUM_MIX_SLOTS < 4
  #error At least 4 mix slots 
#elif NUM_MIX_SLOTS > 40
  #error At most 40 mix slots
#endif

#if NUM_VIRTUAL_CHANNELS < 2
  #error At least 2 virtual channels 
#elif NUM_VIRTUAL_CHANNELS > 5
  #error At most 5 virtual channels
#endif

#if NUM_CUSTOM_TELEMETRY < 2
  #error At least 2 telemetry 
#elif NUM_CUSTOM_TELEMETRY > 10
  #error At most 10 telemetry
#endif
  
#if NUM_WIDGETS < 2
  #error At least 2 widgets 
#elif NUM_WIDGETS > 5
  #error At most 5 widgets
#endif

#if NUM_TIMERS < 1
  #error At least 1 timer
#elif NUM_TIMERS > 3
  #error At most 3 timers
#endif

sys_params_t   Sys;
model_params_t Model; 

int16_t  channelOut[NUM_RC_CHANNELS];  

int16_t stickAxisIn[NUM_STICK_AXES];
 
int16_t  knobIn[NUM_KNOBS];

bool     isCalibratingControls = false;

uint8_t  swState[NUM_PHYSICAL_SWITCHES];

uint8_t  activeFmdIdx = 0;

uint8_t  buttonCode = 0; 
uint32_t buttonStartTime = 0;
uint32_t buttonReleaseTime = 0;

uint32_t heldButtonEntryLoopNum;

uint8_t  pressedButton = 0; 
uint8_t  clickedButton = 0; 
uint8_t  heldButton = 0;

int16_t  battVoltsNow; 
uint8_t  battState = BATTHEALTY;

uint8_t  audioToPlay = AUDIO_NONE;

int16_t   audioTrimVal;

bool     backlightIsOn = false;

uint32_t inputsLastMovedTime = 0; 

uint8_t  transmitterPacketRate = 0;
uint8_t  receiverPacketRate = 0;

bool     isRequestingBind = false;
uint8_t  bindStatusCode = 0;  
bool     isMainReceiver = true;

uint8_t  outputChConfig[MAX_CHANNELS_PER_RECEIVER];
bool     gotOutputChConfig = false;
bool     isRequestingOutputChConfig = false;
bool     isSendOutputChConfig = false;
uint8_t  receiverConfigStatusCode = 0; 

int16_t  telemetryReceivedValue[NUM_CUSTOM_TELEMETRY];
int16_t  telemetryMaxReceivedValue[NUM_CUSTOM_TELEMETRY];
int16_t  telemetryMinReceivedValue[NUM_CUSTOM_TELEMETRY];
uint32_t telemetryLastReceivedTime[NUM_CUSTOM_TELEMETRY];
int16_t  telemetryLastReceivedValue[NUM_CUSTOM_TELEMETRY];
uint8_t  telemetryAlarmState[NUM_CUSTOM_TELEMETRY];
bool     telemetryMuteAlarms = false;
bool     telemetryForceRequest = false;

uint8_t telemetryType;

gnss_telemetry_data_t GNSSTelemetryData;

uint32_t gnssTelemetrylastReceivedTime;
int32_t  gnssDistanceFromHome;

int16_t  counterOut[NUM_COUNTERS];

uint32_t timerElapsedTime[NUM_TIMERS];
uint32_t timerLastElapsedTime[NUM_TIMERS];
uint32_t timerLastPaused[NUM_TIMERS];
bool     timerIsRunning[NUM_TIMERS];
bool     timerForceRun[NUM_TIMERS];

uint8_t  maxNumOfModels;

uint32_t thisLoopNum = 0; 

uint32_t DBG_loopTime;

uint8_t screenshotSwtch = CTRL_SW_NONE;

//--------------------------------------------------------------------------------------------------

void resetSystemParams()
{
  Sys.activeModelIdx = 0;

  for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
  {
    if(SRC_STICK_AXIS_FIRST + i == SRC_Y1_AXIS)
      Sys.StickAxis[i].type = STICK_AXIS_NON_CENTERING;
    else if(SRC_STICK_AXIS_FIRST + i == SRC_Z1_AXIS)
      Sys.StickAxis[i].type = STICK_AXIS_ABSENT;
    else if(SRC_STICK_AXIS_FIRST + i == SRC_Z2_AXIS)
      Sys.StickAxis[i].type = STICK_AXIS_ABSENT;
    else
      Sys.StickAxis[i].type = STICK_AXIS_SELF_CENTERING;

    Sys.StickAxis[i].minVal = 0;
    Sys.StickAxis[i].centerVal = 512;
    Sys.StickAxis[i].maxVal = 1023;
    Sys.StickAxis[i].deadzone = 3;
  }

  for(uint8_t i = 0; i < NUM_KNOBS; i++)
  {
    Sys.Knob[i].minVal = 0;
    Sys.Knob[i].centerVal = 512;
    Sys.Knob[i].maxVal = 1023;
    Sys.Knob[i].deadzone = 3;
    Sys.Knob[i].type = KNOB_NO_CENTER_DETENT;
  }

  Sys.defaultStickMode = STICK_MODE_RTAE;

  Sys.battVoltsMin = 7200; 
  Sys.battVoltsMax = 8000; 
  Sys.battVfactor  = 1000; 
  
  for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++) 
  {
    Sys.swType[i] = (i % 4 == 2) ? SW_3POS : SW_2POS;
  }

  Sys.rfEnabled = false;
  Sys.rfPower = RF_POWER_MEDIUM;

  Sys.soundEnabled = true;
  Sys.soundOnInactivity = true;
  Sys.soundSwitches = true;
  Sys.soundKnobCenter = true;
  Sys.soundKeys = true;
  Sys.soundTrims = true;
  Sys.trimToneFreqMode = TRIM_TONE_FREQ_VARIABLE;
  
  Sys.backlightEnabled = true;
  Sys.backlightLevel = 50;
  Sys.backlightTimeout = BACKLIGHT_TIMEOUT_1MIN;
  Sys.backlightWakeup = BACKLIGHT_WAKEUP_KEYS;
  Sys.backlightSuppressFirstKey = true;
  Sys.contrast = 50;

  Sys.lockStartup = false;
  Sys.lockModels = false;
  Sys.lockOnInactivity = false;
  Sys.password[0] = '\0';
  
  Sys.showMenuIcons = true;
  Sys.rememberMenuPosition = false;
  Sys.useRoundRect = true;
  Sys.useDenserMenus = false;
  Sys.animationsEnabled = true;
  Sys.autohideTrims = false;
  Sys.showSplashScreen = true;
  Sys.showWelcomeMessage = true;
  Sys.useNumericalBatteryIndicator = false;
  Sys.alwaysShowHours = true;
  
  Sys.autoSelectMovedControl = true;
  Sys.mixerTemplatesEnabled = true;
  Sys.defaultChannelOrder = 0;
  Sys.onscreenTrimEnabled = true;
  Sys.inactivityMinutes = 10;
  
  Sys.DBG_showLoopTime = false;
  Sys.DBG_simulateTelemetry = false;
  Sys.DBG_disableInterlacing = false;

  Sys.screenshotSeqNo = 0;
}

//--------------------------------------------------------------------------------------------------

void resetModelName()
{
  Model.name[0] = '\0';
}

//--------------------------------------------------------------------------------------------------

void resetModelParams()
{
  //--- model type ---
  Model.type = MODEL_TYPE_AIRPLANE;

  //---
  Model.secondaryRcvrEnabled = false;
  
  //--- raw sources for rud, thr, ail, ele inputs ---
  if(Sys.defaultStickMode == STICK_MODE_RTAE)
  {
    Model.rudSrcRaw = SRC_X1_AXIS;
    Model.thrSrcRaw = SRC_Y1_AXIS;
    Model.ailSrcRaw = SRC_X2_AXIS;
    Model.eleSrcRaw = SRC_Y2_AXIS;
  }
  if(Sys.defaultStickMode == STICK_MODE_AERT)
  {
    Model.ailSrcRaw = SRC_X1_AXIS;
    Model.eleSrcRaw = SRC_Y1_AXIS;
    Model.rudSrcRaw = SRC_X2_AXIS;
    Model.thrSrcRaw = SRC_Y2_AXIS;
  }
  if(Sys.defaultStickMode == STICK_MODE_REAT)
  {
    Model.rudSrcRaw = SRC_X1_AXIS;
    Model.eleSrcRaw = SRC_Y1_AXIS;
    Model.ailSrcRaw = SRC_X2_AXIS;
    Model.thrSrcRaw = SRC_Y2_AXIS;
  }
  if(Sys.defaultStickMode == STICK_MODE_ATRE)
  {
    Model.ailSrcRaw = SRC_X1_AXIS;
    Model.thrSrcRaw = SRC_Y1_AXIS;
    Model.rudSrcRaw = SRC_X2_AXIS;
    Model.eleSrcRaw = SRC_Y2_AXIS;
  }

  //--- rates and expo ---
  rate_expo_t* rateExpo[3];
  rateExpo[0] = &Model.RudDualRate;
  rateExpo[1] = &Model.AilDualRate;
  rateExpo[2] = &Model.EleDualRate;
  for(uint8_t i = 0; i < 3; i++)
  {
    rateExpo[i]->rate1 = 100;
    rateExpo[i]->rate2 = 100;
    rateExpo[i]->expo1 = 0;
    rateExpo[i]->expo2 = 0;
    rateExpo[i]->swtch = CTRL_SW_NONE;
  }
  
  //--- throttle curve ---
  Model.ThrottleCurve.numPoints = DEFAULT_NUM_POINTS_CUSTOM_CURVE;
  Model.ThrottleCurve.smooth = false;
  for(uint8_t pt = 0; pt < DEFAULT_NUM_POINTS_CUSTOM_CURVE && pt < MAX_NUM_POINTS_CUSTOM_CURVE; pt++)
  {
    Model.ThrottleCurve.yVal[pt] = -100 + ((200 * pt)/(DEFAULT_NUM_POINTS_CUSTOM_CURVE - 1));
    Model.ThrottleCurve.xVal[pt] = -100 + ((200 * pt)/(DEFAULT_NUM_POINTS_CUSTOM_CURVE - 1));
  }
  
  //--- custom curves ---
  resetCustomCurveParams();
  
  //--- mixers ---
  resetMixerParams();
  
  //--- output channels ---
  resetChannelParams();
  
  //--- logical switches ---
  resetLogicalSwitchParams();
  
  //--- function generators ---
  resetFuncgenParams();

  //--- counters ---
  resetCounterParams();
  
  //--- timers ---
  resetTimerParams();

  //--- trim ----
  trim_params_t* qqTrim[4];
  qqTrim[0] = &Model.X1Trim;
  qqTrim[1] = &Model.Y1Trim;
  qqTrim[2] = &Model.X2Trim;
  qqTrim[3] = &Model.Y2Trim;
  for(uint8_t i = 0; i < 4; i++)
  {
    qqTrim[i]->trimState = TRIM_COMMON;
    qqTrim[i]->commonTrim = 0;
  }

  Model.trimStep = TRIM_STEP_MEDIUM;

  //--- flight modes ---
  for(uint8_t i = 0; i < NUM_FLIGHT_MODES; i++)
  {
    Model.FlightMode[i].name[0] = '\0';
    Model.FlightMode[i].swtch = CTRL_SW_NONE;
    Model.FlightMode[i].transitionTime = 10; 
    Model.FlightMode[i].x1Trim = 0;
    Model.FlightMode[i].y1Trim = 0;
    Model.FlightMode[i].x2Trim = 0;
    Model.FlightMode[i].y2Trim = 0;
  }
  
  //--- safety checks ---
  Model.checkThrottle = true;
  for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
  {
    if(Sys.swType[i] == SW_3POS_MOMENTARY)
      Model.switchWarn[i] = SWMIDPOS;
    else
      Model.switchWarn[i] = SWUPPERPOS;
  }
  
  //--- notifications ---
  resetNotificationParams();
  
  //--- telemetry ---
  resetTelemParams();
  
  //--- widgets ---
  resetWidgetParams();

  //--- GNSS telemetry ---
  Model.gnssHomeLongitude = 0;
  Model.gnssHomeLatitude = 0;
  Model.gnssAltitudeOffset = 0;
  Model.gnssLastKnownLatitude = 0;
  Model.gnssLastKnownLongitude = 0;
  Model.gnssLastKnownAltitude = 0;
  Model.gnssLastKnownDistanceFromHome = 0;
}

//--------------------------------------------------------------------------------------------------

void resetCustomCurveParams()
{
  for(uint8_t i = 0; i < NUM_CUSTOM_CURVES; i++)
    resetCustomCurveParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetCustomCurveParams(uint8_t idx)
{
  if(idx >= NUM_CUSTOM_CURVES)
    return;
  
  Model.CustomCurve[idx].name[0] = '\0';
  Model.CustomCurve[idx].numPoints = DEFAULT_NUM_POINTS_CUSTOM_CURVE;
  Model.CustomCurve[idx].smooth = false;
  for(uint8_t pt = 0; pt < DEFAULT_NUM_POINTS_CUSTOM_CURVE && pt < MAX_NUM_POINTS_CUSTOM_CURVE; pt++)
  {
    Model.CustomCurve[idx].xVal[pt] = -100 + ((200 * pt)/(DEFAULT_NUM_POINTS_CUSTOM_CURVE - 1));
    Model.CustomCurve[idx].yVal[pt] = -100 + ((200 * pt)/(DEFAULT_NUM_POINTS_CUSTOM_CURVE - 1));
  }
}

//--------------------------------------------------------------------------------------------------

void resetLogicalSwitchParams()
{
  for(uint8_t i = 0; i < NUM_LOGICAL_SWITCHES; i++)
    resetLogicalSwitchParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetLogicalSwitchParams(uint8_t idx)
{
  if(idx >= NUM_LOGICAL_SWITCHES)
    return;
  
  Model.LogicalSwitch[idx].func = LS_FUNC_NONE;
  Model.LogicalSwitch[idx].val1 = SRC_NONE;
  Model.LogicalSwitch[idx].val2 = 0;
  Model.LogicalSwitch[idx].val3 = 0; 
  Model.LogicalSwitch[idx].val4 = 0; 
}

//--------------------------------------------------------------------------------------------------

void resetTimerParams()
{
  for(uint8_t i = 0; i < NUM_TIMERS; i++)
    resetTimerParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetTimerParams(uint8_t idx)
{
  if(idx >= NUM_TIMERS)
    return;

  Model.Timer[idx].name[0] = '\0';
  Model.Timer[idx].swtch = CTRL_SW_NONE;
  Model.Timer[idx].resetSwitch = CTRL_SW_NONE;
  Model.Timer[idx].initialSeconds = 0;
  Model.Timer[idx].isPersistent = false;
  Model.Timer[idx].persistVal = 0;
}

//--------------------------------------------------------------------------------------------------

void resetTimerRegisters()
{
  for(uint8_t i = 0; i < NUM_TIMERS; i++)
  {
    resetTimerRegister(i);
    timerForceRun[i] = false;
  }
}

//--------------------------------------------------------------------------------------------------

void resetTimerRegister(uint8_t idx)
{
  timerElapsedTime[idx] = 0;
  timerLastElapsedTime[idx] = 0;
  timerLastPaused[idx] = millis();
  timerIsRunning[idx] = false;
}

//--------------------------------------------------------------------------------------------------

void restoreTimerRegisters()
{
  for(uint8_t i = 0; i < NUM_TIMERS; i++)
  {
    if(Model.Timer[i].isPersistent)
    {
      timerLastElapsedTime[i] = Model.Timer[i].persistVal;
      timerElapsedTime[i] = Model.Timer[i].persistVal;
      timerLastPaused[i] = millis();
    }
  }
}

//--------------------------------------------------------------------------------------------------

void resetCounterParams()
{
  for(uint8_t i = 0; i < NUM_COUNTERS; i++)
    resetCounterParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetCounterParams(uint8_t idx)
{
  if(idx >= NUM_COUNTERS)
    return;
  
  Model.Counter[idx].name[0] = '\0';
  Model.Counter[idx].clock = CTRL_SW_NONE;
  Model.Counter[idx].edge = 0;
  Model.Counter[idx].clear = CTRL_SW_NONE;
  Model.Counter[idx].modulus = 1000;
  Model.Counter[idx].direction = 0; 
  Model.Counter[idx].isPersistent = false;
  Model.Counter[idx].persistVal = 0;
}

//--------------------------------------------------------------------------------------------------

void resetCounterRegisters()
{
  for(uint8_t i = 0; i < NUM_COUNTERS; i++)
    resetCounterRegister(i);
}

//--------------------------------------------------------------------------------------------------

void resetCounterRegister(uint8_t idx)
{
  counterOut[idx] = 0;
}

//--------------------------------------------------------------------------------------------------

void restoreCounterRegisters()
{
  for(uint8_t i = 0; i < NUM_COUNTERS; i++)
  {
    if(Model.Counter[i].isPersistent)
    {
      counterOut[i] = Model.Counter[i].persistVal;
    }
  }
}

//--------------------------------------------------------------------------------------------------

void resetFuncgenParams()
{
  for(uint8_t i = 0; i < NUM_FUNCGEN; i++)
    resetFuncgenParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetFuncgenParams(uint8_t idx)
{
  if(idx >= NUM_FUNCGEN)
    return;
  
  Model.Funcgen[idx].waveform = FUNCGEN_WAVEFORM_SINE;
  Model.Funcgen[idx].periodMode = FUNCGEN_PERIODMODE_FIXED;
  Model.Funcgen[idx].period1 = 10;
  Model.Funcgen[idx].period2 = 20;
  Model.Funcgen[idx].modulatorSrc = SRC_NONE;
  Model.Funcgen[idx].reverseModulator = false;
  Model.Funcgen[idx].phaseMode = FUNCGEN_PHASEMODE_AUTO; 
  Model.Funcgen[idx].phase = 0;
}

//--------------------------------------------------------------------------------------------------

void resetMixerParams()
{
  for(uint8_t i = 0; i < NUM_MIX_SLOTS; i++)
    resetMixerParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetMixerParams(uint8_t idx)
{
  if(idx >= NUM_MIX_SLOTS)
    return;
  
  Model.Mixer[idx].name[0] = '\0';
  Model.Mixer[idx].output = SRC_NONE;
  Model.Mixer[idx].swtch = CTRL_SW_NONE;
  Model.Mixer[idx].operation = MIX_ADD;
  Model.Mixer[idx].input = SRC_NONE;
  Model.Mixer[idx].weight = 100;
  Model.Mixer[idx].offset = 0;
  Model.Mixer[idx].curveType = 0;
  Model.Mixer[idx].curveVal = 0;
  Model.Mixer[idx].trimEnabled = true;
  Model.Mixer[idx].flightMode = 0xFF;
  Model.Mixer[idx].delayUp = 0;
  Model.Mixer[idx].delayDown = 0;
  Model.Mixer[idx].slowUp = 0;
  Model.Mixer[idx].slowDown = 0;
}

//--------------------------------------------------------------------------------------------------

void resetChannelParams()
{
  for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
    resetChannelParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetChannelParams(uint8_t idx)
{
  if(idx >= NUM_RC_CHANNELS)
    return;
  
  Model.Channel[idx].name[0] = '\0';
  Model.Channel[idx].curve = -1;
  Model.Channel[idx].reverse = false;
  Model.Channel[idx].subtrim = 0;
  Model.Channel[idx].overrideSwitch = CTRL_SW_NONE;
  Model.Channel[idx].overrideVal = -100;
  Model.Channel[idx].failsafe  = -102;
  Model.Channel[idx].endpointL = -100;
  Model.Channel[idx].endpointR = 100;
}

//--------------------------------------------------------------------------------------------------

void resetTelemParams()
{
  for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
    resetTelemParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetTelemParams(uint8_t idx)
{
  if(idx >= NUM_CUSTOM_TELEMETRY)
    return;
  
  Model.Telemetry[idx].name[0] = '\0';
  Model.Telemetry[idx].unitsName[0] = '\0';
  Model.Telemetry[idx].type = TELEMETRY_TYPE_GENERAL;
  Model.Telemetry[idx].identifier = 0;
  Model.Telemetry[idx].multiplier = 100;
  Model.Telemetry[idx].factor10 = 0;
  Model.Telemetry[idx].offset = 0;
  Model.Telemetry[idx].alarmCondition = TELEMETRY_ALARM_CONDITION_NONE;
  Model.Telemetry[idx].alarmThreshold = 0;
  Model.Telemetry[idx].showOnHome = true; 
  Model.Telemetry[idx].recordMaximum = true; 
  Model.Telemetry[idx].recordMinimum = true; 
  
  //also reset received telemetry
  telemetryReceivedValue[idx] = TELEMETRY_NO_DATA;
  telemetryLastReceivedValue[idx] = TELEMETRY_NO_DATA;
  telemetryLastReceivedTime[idx] = millis();
  telemetryMaxReceivedValue[idx] = TELEMETRY_NO_DATA;
  telemetryMinReceivedValue[idx] = TELEMETRY_NO_DATA;
}

//--------------------------------------------------------------------------------------------------

void resetNotificationParams()
{
  for(uint8_t i = 0; i < NUM_CUSTOM_NOTIFICATIONS; i++)
    resetNotificationParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetNotificationParams(uint8_t idx)
{
  if(idx >= NUM_CUSTOM_NOTIFICATIONS)
    return;
  
  Model.CustomNotification[idx].enabled = true;
  Model.CustomNotification[idx].swtch = CTRL_SW_NONE;
  Model.CustomNotification[idx].tone = AUDIO_NOTIFICATION_TONE_FIRST;
  Model.CustomNotification[idx].text[0] = '\0';
}

//--------------------------------------------------------------------------------------------------

void resetWidgetParams()
{
  for(uint8_t i = 0; i < NUM_WIDGETS; i++)
    resetWidgetParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetWidgetParams(uint8_t idx)
{
  if(idx >= NUM_WIDGETS)
    return;
  
  if(idx == 0)
  {
    Model.Widget[idx].type = WIDGET_TYPE_TIMERS;
    Model.Widget[idx].src = 0;
  }
  else
  {
    Model.Widget[idx].type = WIDGET_TYPE_TELEMETRY;
    Model.Widget[idx].src  = WIDGET_SRC_AUTO;
    Model.Widget[idx].disp = WIDGET_DISP_NUMERICAL;
    Model.Widget[idx].gaugeMin = 0;
    Model.Widget[idx].gaugeMax = 100;
  }
}

//--------------------------------------------------------------------------------------------------

bool changeLSReference(uint8_t newRef, uint8_t oldRef)
{
  //Searches for and replaces all occurances of a Logical switch old reference with new reference.

  if(newRef >= NUM_LOGICAL_SWITCHES || oldRef >= NUM_LOGICAL_SWITCHES)
    return false;
  
  bool refFound = false;
  
  uint8_t swOld = CTRL_SW_LOGICAL_FIRST + oldRef;
  uint8_t swNew = CTRL_SW_LOGICAL_FIRST + newRef;
  
  uint8_t swInvertOld =CTRL_SW_LOGICAL_FIRST_INVERT + oldRef;
  uint8_t swInvertNew =CTRL_SW_LOGICAL_FIRST_INVERT + newRef;

  uint8_t srcOld = SRC_SW_LOGICAL_FIRST + oldRef;
  uint8_t srcNew = SRC_SW_LOGICAL_FIRST + newRef;

  //widgets
  for(uint8_t i = 0; i < NUM_WIDGETS; i++)
  {
    widget_params_t *widget = &Model.Widget[i];
    if(widget->type == WIDGET_TYPE_MIXSOURCES)
    {
      if(widget->src == srcOld)   {refFound = true; widget->src = srcNew;}
    }
  }

  //dual rate switch
  rate_expo_t *rateExpo[3];
  rateExpo[0] = &Model.RudDualRate;
  rateExpo[1] = &Model.AilDualRate;
  rateExpo[2] = &Model.EleDualRate;
  for(uint8_t i = 0; i < 3; i++)
  {
    if(rateExpo[i]->swtch == swOld)       {refFound = true; rateExpo[i]->swtch = swNew;}
    if(rateExpo[i]->swtch == swInvertOld) {refFound = true; rateExpo[i]->swtch = swInvertNew;}
  }

  //mixer
  for(uint8_t i = 0; i < NUM_MIX_SLOTS; i++)
  {
    mixer_params_t *mxr = &Model.Mixer[i];
    if(mxr->swtch == swOld)       {refFound = true; mxr->swtch = swNew;}
    if(mxr->swtch == swInvertOld) {refFound = true; mxr->swtch = swInvertNew;}
    if(mxr->input == srcOld)      {refFound = true; mxr->input = srcNew;}
  }

  //output channels
  for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
  {
    channel_params_t *ch = &Model.Channel[i];
    if(ch->overrideSwitch == swOld)       {refFound = true; ch->overrideSwitch = swNew;}
    if(ch->overrideSwitch == swInvertOld) {refFound = true; ch->overrideSwitch = swInvertNew;}
  }

  //logical switches
  for(uint8_t i = 0; i < NUM_LOGICAL_SWITCHES; i++)
  {
    logical_switch_t *ls = &Model.LogicalSwitch[i];
    if(ls->func <= LS_FUNC_GROUP3_LAST)
    {
      if(ls->val1 == srcOld)      {refFound = true; ls->val1 = srcNew;}
    }
    else if(ls->func <= LS_FUNC_GROUP4_LAST)
    {
      if(ls->val1 == srcOld)      {refFound = true; ls->val1 = srcNew;}
      if(ls->val2 == srcOld)      {refFound = true; ls->val2 = srcNew;}
    }
    else if(ls->func <= LS_FUNC_GROUP5_LAST)
    {
      if(ls->val1 == swOld)       {refFound = true; ls->val1 = swNew;}
      if(ls->val1 == swInvertOld) {refFound = true; ls->val1 = swInvertNew;}
      if(ls->val2 == swOld)       {refFound = true; ls->val2 = swNew;}
      if(ls->val2 == swInvertOld) {refFound = true; ls->val2 = swInvertNew;}
    }
    else if(ls->func == LS_FUNC_TOGGLE)
    {
      if(ls->val1 == swOld)       {refFound = true; ls->val1 = swNew;}
      if(ls->val1 == swInvertOld) {refFound = true; ls->val1 = swInvertNew;}
      if(ls->val3 == swOld)       {refFound = true; ls->val3 = swNew;}
      if(ls->val3 == swInvertOld) {refFound = true; ls->val3 = swInvertNew;}
    }
  }

  //function generators
  for(uint8_t i = 0; i < NUM_FUNCGEN; i++)
  {
    funcgen_t *fgen = &Model.Funcgen[i];
    if(fgen->modulatorSrc == srcOld)   {refFound = true; fgen->modulatorSrc = srcNew;}
  }
  
  //timers
  for(uint8_t i = 0; i < NUM_TIMERS; i++)
  {
    timer_params_t *tmr = &Model.Timer[i];
    if(tmr->swtch == swOld)            { refFound = true; tmr->swtch = swNew;}
    if(tmr->swtch == swInvertOld)       {refFound = true; tmr->swtch = swInvertNew;}
    if(tmr->resetSwitch == swOld)       {refFound = true; tmr->resetSwitch = swNew;}
    if(tmr->resetSwitch == swInvertOld) {refFound = true; tmr->resetSwitch = swInvertNew;}
  }

  //counters
  for(uint8_t i = 0; i < NUM_COUNTERS; i++)
  {
    counter_params_t *counter = &Model.Counter[i];
    if(counter->clock == swOld)        {refFound = true; counter->clock = swNew;}
    if(counter->clock == swInvertOld)  {refFound = true; counter->clock = swInvertNew;}
    if(counter->clear == swOld)        {refFound = true; counter->clear = swNew;}
    if(counter->clear == swInvertOld)  {refFound = true; counter->clear = swInvertNew;}
  }

  //custom notifications
  for(uint8_t i = 0; i < NUM_CUSTOM_NOTIFICATIONS; i++)
  {
    notification_params_t *notification = &Model.CustomNotification[i];
    if(notification->swtch == swOld)       {refFound = true; notification->swtch = swNew;}
    if(notification->swtch == swInvertOld) {refFound = true; notification->swtch = swInvertNew;}
  }

  //flight modes
  for(uint8_t i = 0; i < NUM_FLIGHT_MODES; i++)
  {
    flight_mode_t *fmd = &Model.FlightMode[i];
    if(fmd->swtch == swOld)            {refFound = true; fmd->swtch = swNew;}
    if(fmd->swtch == swInvertOld)      {refFound = true; fmd->swtch = swInvertNew;}
  }

  return refFound;
}

//--------------------------------------------------------------------------------------------------

bool verifyModelData()
{
  // Verify if the model data is sane. Here, we check for critical
  // parameters that could cause a segfault if incorrect,
  // and a few other indicators.

  bool isSane = true;

  for(uint8_t i = 0; i < NUM_FUNCGEN; i++)
  {
    if(Model.Funcgen[i].waveform >= FUNCGEN_WAVEFORM_COUNT) isSane = false;
    if(Model.Funcgen[i].phaseMode >= FUNCGEN_PHASEMODE_COUNT) isSane = false;
    if(Model.Funcgen[i].modulatorSrc >= MIX_SOURCES_COUNT) isSane = false;
  }

  for(uint8_t i = 0; i < NUM_MIX_SLOTS; i++)
  {
    if(Model.Mixer[i].output >= MIX_SOURCES_COUNT) isSane = false;
    if(Model.Mixer[i].input >= MIX_SOURCES_COUNT) isSane = false;
    if(Model.Mixer[i].curveType >= MIX_CURVE_TYPE_COUNT)  isSane = false;
    if(Model.Mixer[i].operation >= MIX_OPERATOR_COUNT)  isSane = false;
  }

  for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
  {
    if(Model.Channel[i].curve != -1 && Model.Channel[i].curve >= NUM_CUSTOM_CURVES)
      isSane = false;
  }

  for(uint8_t i = 0; i < NUM_CUSTOM_CURVES; i++)
  {
    if(Model.CustomCurve[i].numPoints > MAX_NUM_POINTS_CUSTOM_CURVE) 
      isSane = false;
  }
  
  if(Model.ThrottleCurve.numPoints > MAX_NUM_POINTS_CUSTOM_CURVE) 
    isSane = false;
  
  for(uint8_t i = 0; i < NUM_LOGICAL_SWITCHES; i++)
  {
    if(Model.LogicalSwitch[i].func >= LS_FUNC_COUNT) 
      isSane = false;
  }
  
  for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
  {
    if(Model.Telemetry[i].alarmCondition >= TELEMETRY_ALARM_CONDITION_COUNT) 
      isSane = false;
  }

  return isSane;
}

//--------------------------------------------------------------------------------------------------

bool verifySystemData()
{
  // Verify system settings. 
  // Here we check a few indicators.
  
  bool isSane = true;
  
  for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
  {
    if(Sys.StickAxis[i].type >= STICK_AXIS_TYPE_COUNT)
      isSane = false;
  }
  
  for(uint8_t i = 0; i < NUM_KNOBS; i++)
  {
    if(Sys.Knob[i].type >= KNOB_TYPE_COUNT)
      isSane = false;
  }
  
  for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
  {
    if(Sys.swType[i] >= SW_TYPE_COUNT)
      isSane = false;
  }
  
  if(Sys.backlightWakeup >= BACKLIGHT_WAKEUP_COUNT) 
    isSane = false;
  
  if(Sys.backlightTimeout >= BACKLIGHT_TIMEOUT_COUNT) 
    isSane = false;
  
  if(Sys.rfPower >= RF_POWER_COUNT) 
    isSane = false;
  
  if(Sys.defaultStickMode >= STICK_MODE_COUNT) 
    isSane = false;
  
  if(Sys.trimToneFreqMode >= TRIM_TONE_FREQ_MODE_COUNT) 
    isSane = false;
  
  return isSane;
}

//--------------------------------------------------------------------------------------------------

uint16_t joinBytes(uint8_t highByte, uint8_t lowByte)
{
  uint16_t rslt;
  rslt = (uint16_t) highByte;
  rslt <<= 8;
  rslt |= (uint16_t) lowByte;
  return rslt;
}

//--------------------------------------------------------------------------------------------------

bool isEmptyStr(char* buff, uint8_t lenBuff)
{
  //Returns true if all characters are spaces
  
  for(uint8_t i = 0; i < lenBuff - 1; i++)
  {
    //handle partial string
    if(*(buff + i) == '\0')
      break;
    //check if character isn't a space
    if(*(buff + i) != ' ')
      return false;
  }
  return true;
}

//--------------------------------------------------------------------------------------------------

//Trims in place the whitespace at beginning and end of a the string

void trimWhiteSpace(char* buff, uint8_t lenBuff)
{
  if(buff[0] == '\0')
    return;
  //--- trim leading space
  uint8_t i = 0; 
  while(isspace(buff[i]))
  {
    i++;
    if(i == lenBuff - 1) //all spaces
    {
      buff[0] = '\0';
      return;
    }
  }
  //--- trim trailing space
  if(lenBuff < 2)
    return;
  //find the first occurance of the null character (in case of a partial string)
  uint8_t idxOfNull = 0xff;
  for(uint8_t m = i; m < lenBuff - 1; m++)
  {
    if(buff[m] == '\0')
    {
      idxOfNull = m;
      break;
    }
  }
  uint8_t j = (idxOfNull != 0xff) ? idxOfNull - 1 : lenBuff - 2;
  while(j > i && isspace(buff[j]))
    j--;
  //--- shift the remaining characters to the beginning of the string
  uint8_t k = 0;
  while(i <= j)
    buff[k++] = buff[i++];
  buff[k] = '\0';  //add a null terminator
}

