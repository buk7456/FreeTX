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

#if NUM_MIXSLOTS < 4
  #error At least 4 mix slots 
#elif NUM_MIXSLOTS > 40
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
#elif NUM_WIDGETS > 4
  #error At most 4 widgets
#endif

#if NUM_TIMERS < 1
  #error At least 1 timer
#elif NUM_TIMERS > 3
  #error At most 3 timers
#endif

sys_params_t   Sys;
model_params_t Model; 

int16_t  channelOut[NUM_RC_CHANNELS];  
 
int16_t  x1AxisIn;
int16_t  y1AxisIn; 
int16_t  x2AxisIn;
int16_t  y2AxisIn;
int16_t  x3AxisIn;
int16_t  y3AxisIn; 
int16_t  x4AxisIn;
int16_t  y4AxisIn;

int16_t  knobAIn;
int16_t  knobBIn;

bool     isCalibratingSticks = false;

uint8_t  swState[MAX_NUM_PHYSICAL_SWITCHES];

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

bool     backlightIsOn = false;

uint32_t inputsLastMoved = 0; 

uint8_t  transmitterPacketRate = 0;
uint8_t  receiverPacketRate = 0;

bool     isRequestingBind = false;
uint8_t  bindStatusCode = 0;  
bool     isMainReceiver = true;

uint8_t  outputChConfig[NUM_RC_CHANNELS]; 
uint8_t  maxOutputChConfig[NUM_RC_CHANNELS];
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

int16_t  counterOut[NUM_COUNTERS];

uint8_t  maxNumOfModels;

uint32_t thisLoopNum = 0; 

uint32_t DBG_loopTime;

//--------------------------------------------------------------------------------------------------

void resetSystemParams()
{
  Sys.activeModelIdx = 0;
  
  Sys.x1AxisMin = 0, Sys.x1AxisCenter = 512, Sys.x1AxisMax = 1023;
  Sys.y1AxisMin = 0, Sys.y1AxisCenter = 512, Sys.y1AxisMax = 1023;
  Sys.x2AxisMin = 0, Sys.x2AxisCenter = 512, Sys.x2AxisMax = 1023;
  Sys.y2AxisMin = 0, Sys.y2AxisCenter = 512, Sys.y2AxisMax = 1023;
  Sys.x3AxisMin = 0, Sys.x3AxisCenter = 512, Sys.x3AxisMax = 1023;
  Sys.y3AxisMin = 0, Sys.y3AxisCenter = 512, Sys.y3AxisMax = 1023;
  Sys.x4AxisMin = 0, Sys.x4AxisCenter = 512, Sys.x4AxisMax = 1023;
  Sys.y4AxisMin = 0, Sys.y4AxisCenter = 512, Sys.y4AxisMax = 1023;
  
  Sys.x1AxisType = STICK_AXIS_SELF_CENTERING;
  Sys.y1AxisType = STICK_AXIS_NON_CENTERING;
  Sys.x2AxisType = STICK_AXIS_SELF_CENTERING;
  Sys.y2AxisType = STICK_AXIS_SELF_CENTERING;
  Sys.x3AxisType = STICK_AXIS_SELF_CENTERING;
  Sys.y3AxisType = STICK_AXIS_SELF_CENTERING;
  Sys.x4AxisType = STICK_AXIS_SELF_CENTERING;
  Sys.y4AxisType = STICK_AXIS_SELF_CENTERING;

  Sys.x1AxisDeadzone = 3;
  Sys.y1AxisDeadzone = 3;
  Sys.x2AxisDeadzone = 3;
  Sys.y2AxisDeadzone = 3;
  Sys.x3AxisDeadzone = 3;
  Sys.y3AxisDeadzone = 3;
  Sys.x4AxisDeadzone = 3;
  Sys.y4AxisDeadzone = 3;
  
  Sys.defaultStickMode = STICK_MODE_RTAE;

  Sys.battVoltsMin = 7200; 
  Sys.battVoltsMax = 8000; 
  Sys.battVfactor  = 1000; 
  
  for(uint8_t i = 0; i < MAX_NUM_PHYSICAL_SWITCHES; i++) 
  {
    Sys.swType[i] = (i%4 == 2) ? SW_3POS : SW_2POS;
  }

  Sys.rfEnabled = false;
  Sys.rfPower = RF_POWER_MEDIUM;

  Sys.soundEnabled = true; 
  Sys.soundTrims = true;
  Sys.soundSwitches = true;
  Sys.soundKnobCenter = true;
  Sys.soundKeys = true;
  Sys.inactivityMinutes = 10;
  
  Sys.backlightEnabled = true;
  Sys.backlightLevel = 50;
  Sys.backlightTimeout = BACKLIGHT_TIMEOUT_1MIN;
  Sys.backlightWakeup = BACKLIGHT_WAKEUP_KEYS;
  Sys.backlightSuppressFirstKey = true;
  Sys.contrast = 50;

  Sys.lockStartup = false;
  Sys.lockModels = false;
  Sys.password[0] = '\0';
  
  Sys.showMenuIcons = true;
  Sys.rememberMenuPosition = true;
  Sys.useRoundRect = false;
  Sys.animationsEnabled = true;
  Sys.autohideTrims = false;
  Sys.showSplashScreen = true;
  Sys.showWelcomeMsg = true;
  Sys.useNumericalBatteryIndicator = false;
  
  Sys.autoSelectMovedControl = true;
  Sys.mixerTemplatesEnabled = true;
  Sys.defaultChannelOrder = 0;
  
  Sys.DBG_showLoopTime = false;
  Sys.DBG_simulateTelemetry = false;
  Sys.DBG_disableInterlacing = false;
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
  rate_expo_t* rateExpo[3] = {&Model.RudDualRate, &Model.AilDualRate, &Model.EleDualRate};
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
  
  //--- logical switches ---
  resetLogicalSwitchParams();
  
  //--- function generators ---
  resetFuncgenParams();

  //--- counters ---
  resetCounterParams();
  
  //--- timers ---
  resetTimerParams();

  //--- output channels ---
  for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
  {
    Model.Channel[i].name[0] = '\0';
    Model.Channel[i].curve = -1;
    Model.Channel[i].reverse = false;
    Model.Channel[i].subtrim = 0;
    Model.Channel[i].overrideSwitch = CTRL_SW_NONE;
    Model.Channel[i].overrideVal = -100;
    Model.Channel[i].failsafe  = -102;
    Model.Channel[i].endpointL = -100;
    Model.Channel[i].endpointR = 100;
  }
  
  //--- trim ----
  trim_params_t* qqTrim[4] = {&Model.X1Trim, &Model.Y1Trim, &Model.X2Trim, &Model.Y2Trim};
  for(uint8_t i = 0; i < 4; i++)
  {
    qqTrim[i]->trimState = TRIM_COMMON;
    qqTrim[i]->commonTrim = 0;
  }

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
  for(uint8_t i = 0; i < MAX_NUM_PHYSICAL_SWITCHES; i++)
    Model.switchWarn[i] = 0; //Up position
  
  //--- telemetry ---
  resetTelemParams();
  
  //--- widgets ---
  resetWidgetParams();
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
  Model.Timer[idx].initialMinutes = 0;
  Model.Timer[idx].isPersistent = false;
  Model.Timer[idx].persistVal = 0;
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
  
  Model.Counter[idx].name[0]   = '\0';
  Model.Counter[idx].clock     = CTRL_SW_NONE;
  Model.Counter[idx].edge      = 0;
  Model.Counter[idx].clear     = CTRL_SW_NONE;
  Model.Counter[idx].modulus   = 1000;
  Model.Counter[idx].direction = 0; 
  Model.Counter[idx].isPersistent = false;
  Model.Counter[idx].persistVal = 0;
  
  //also clear the output value of the counter to 0
  counterOut[idx] = 0;
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
  Model.Funcgen[idx].phaseAngle = 0;
}

//--------------------------------------------------------------------------------------------------

void resetMixerParams()
{
  for(uint8_t i = 0; i < NUM_MIXSLOTS; i++)
    resetMixerParams(i);
}

//--------------------------------------------------------------------------------------------------

void resetMixerParams(uint8_t idx)
{
  if(idx >= NUM_MIXSLOTS)
    return;
  
  Model.Mixer[idx].name[0] = '\0';
  Model.Mixer[idx].output      = SRC_NONE;
  Model.Mixer[idx].swtch       = CTRL_SW_NONE;
  Model.Mixer[idx].operation   = MIX_ADD;
  Model.Mixer[idx].input       = SRC_NONE;
  Model.Mixer[idx].weight      = 100;
  Model.Mixer[idx].offset      = 0;
  Model.Mixer[idx].curveType   = 0;
  Model.Mixer[idx].curveVal    = 0;
  Model.Mixer[idx].trimEnabled = true;
  Model.Mixer[idx].flightMode  = 0xFF;
  Model.Mixer[idx].delayUp     = 0;
  Model.Mixer[idx].delayDown   = 0;
  Model.Mixer[idx].slowUp      = 0;
  Model.Mixer[idx].slowDown    = 0;
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
  
  Model.Telemetry[idx].name[0]        = '\0';
  Model.Telemetry[idx].unitsName[0]   = '\0';
  Model.Telemetry[idx].identifier     = 0;
  Model.Telemetry[idx].multiplier     = 100;
  Model.Telemetry[idx].factor10       = 0;
  Model.Telemetry[idx].offset         = 0;
  Model.Telemetry[idx].logEnabled     = false;
  Model.Telemetry[idx].alarmCondition = TELEMETRY_ALARM_CONDITION_NONE;
  Model.Telemetry[idx].alarmThreshold = 0;
  Model.Telemetry[idx].alarmMelody    = 0; 
  Model.Telemetry[idx].showOnHome     = true; 
  Model.Telemetry[idx].recordMaximum  = true; 
  Model.Telemetry[idx].recordMinimum  = true; 
  
  //also reset received telemetry
  telemetryReceivedValue[idx] = TELEMETRY_NO_DATA;
  telemetryLastReceivedValue[idx] = TELEMETRY_NO_DATA;
  telemetryLastReceivedTime[idx] = millis();
  telemetryMaxReceivedValue[idx] = TELEMETRY_NO_DATA;
  telemetryMinReceivedValue[idx] = TELEMETRY_NO_DATA;
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

bool verifyModelData()
{
  // Verify if the model data is sane. Here, we check for critical
  // parameters that could cause a segfault if incorrect.

  bool isSane = true;

  for(uint8_t i = 0; i < NUM_FUNCGEN; i++)
  {
    if(Model.Funcgen[i].waveform >= FUNCGEN_WAVEFORM_COUNT) isSane = false;
    if(Model.Funcgen[i].phaseMode >= FUNCGEN_PHASEMODE_COUNT) isSane = false;
  }

  for(uint8_t i = 0; i < NUM_MIXSLOTS; i++)
  {
    if(Model.Mixer[i].output >= MIXSOURCES_COUNT) isSane = false;
    if(Model.Mixer[i].input >= MIXSOURCES_COUNT) isSane = false;
    if(Model.Mixer[i].swtch >= CTRL_SW_COUNT)  isSane = false;
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

//Trims in place the whitespace at begining and end of a the string
  
void trimWhiteSpace(char* buff, uint8_t lenBuff)
{
  if(buff[0] == '\0')
    return;
  
  //trim leading space
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
  
  //trim trailing space
  if(lenBuff < 2)
    return;
  uint8_t j = lenBuff - 2;
  while(j > i && isspace(buff[j]))
    j--;
  
  //shift the remaining characters to the beginning of the string
  uint8_t k = 0;
  while(i <= j)
    buff[k++] = buff[i++];
  buff[k] = '\0';  //add a null terminator
}

//--------------------------------------------------------------------------------------------------

void getSrcName(char* buff, uint8_t idx, uint8_t lenBuff)
{
  switch(idx)
  {
    case SRC_NONE:    strlcpy_P(buff, PSTR("None"), lenBuff); break;
    case SRC_X1_AXIS: strlcpy_P(buff, PSTR("X1"), lenBuff); break;
    case SRC_Y1_AXIS: strlcpy_P(buff, PSTR("Y1"), lenBuff); break;
    case SRC_X2_AXIS: strlcpy_P(buff, PSTR("X2"), lenBuff); break;
    case SRC_Y2_AXIS: strlcpy_P(buff, PSTR("Y2"), lenBuff); break;
    case SRC_X3_AXIS: strlcpy_P(buff, PSTR("X3"), lenBuff); break;
    case SRC_Y3_AXIS: strlcpy_P(buff, PSTR("Y3"), lenBuff); break;
    case SRC_X4_AXIS: strlcpy_P(buff, PSTR("X4"), lenBuff); break;
    case SRC_Y4_AXIS: strlcpy_P(buff, PSTR("Y4"), lenBuff); break;
    case SRC_KNOB_A:  strlcpy_P(buff, PSTR("KnobA"), lenBuff); break;
    case SRC_KNOB_B:  strlcpy_P(buff, PSTR("KnobB"), lenBuff); break;
    case SRC_100PERC: strlcpy_P(buff, PSTR("Max"), lenBuff); break;
    case SRC_THR: strlcpy_P(buff, PSTR("Thr"), lenBuff); break;
    case SRC_AIL: strlcpy_P(buff, Model.type == MODEL_TYPE_MULTICOPTER ? PSTR("Roll") : PSTR("Ail"), lenBuff); break;
    case SRC_ELE: strlcpy_P(buff, Model.type == MODEL_TYPE_MULTICOPTER ? PSTR("Pitch") : PSTR("Ele"), lenBuff); break;
    case SRC_RUD: strlcpy_P(buff, Model.type == MODEL_TYPE_MULTICOPTER ? PSTR("Yaw") : PSTR("Rud"), lenBuff); break;
    default:
    {
      char suffix[5]; //large enough to hold the range we want to convert
      memset(suffix, 0, sizeof(suffix));
      if(idx >= SRC_SW_PHYSICAL_FIRST && idx <= SRC_SW_PHYSICAL_LAST)
      {
        strlcpy_P(buff, PSTR("Sw"), lenBuff);
        suffix[0] = 'A' + (idx - SRC_SW_PHYSICAL_FIRST);
        strlcat(buff, suffix, lenBuff);
      }
      else if(idx >= SRC_FUNCGEN_FIRST && idx <= SRC_FUNCGEN_LAST)
      {
        strlcpy_P(buff, PSTR("Fgen"), lenBuff);
        itoa((idx - SRC_FUNCGEN_FIRST) + 1, suffix, 10);
        strlcat(buff, suffix, lenBuff);
      }
      else if(idx >= SRC_SW_LOGICAL_FIRST && idx <= SRC_SW_LOGICAL_LAST)
      {
        strlcpy_P(buff, PSTR("L"), lenBuff);
        itoa((idx - SRC_SW_LOGICAL_FIRST) + 1, suffix, 10);
        strlcat(buff, suffix, lenBuff);
      }
      else if(idx >= SRC_CH1 && idx < SRC_CH1 + NUM_RC_CHANNELS)
      {
        strlcpy_P(buff, PSTR("Ch"), lenBuff);
        itoa((idx - SRC_CH1) + 1, suffix, 10);
        strlcat(buff, suffix, lenBuff);
      }
      else if(idx >= SRC_VIRTUAL_FIRST && idx <= SRC_VIRTUAL_LAST)
      {
        strlcpy_P(buff, PSTR("Virt"), lenBuff);
        itoa((idx - SRC_VIRTUAL_FIRST) + 1, suffix, 10);
        strlcat(buff, suffix, lenBuff);
      }
      else if(idx < MIXSOURCES_COUNT + NUM_COUNTERS) //counters as sources
      {
        strlcpy_P(buff, PSTR("Counter"), lenBuff);
        itoa((idx - MIXSOURCES_COUNT) + 1, suffix, 10);
        strlcat(buff, suffix, lenBuff);
      }
      else //telemetry as sources
      {
        strlcpy(buff, Model.Telemetry[idx - (MIXSOURCES_COUNT + NUM_COUNTERS)].name, lenBuff);
      }
    }
  }
}

//--------------------------------------------------------------------------------------------------

void getControlSwitchName(char* buff, uint8_t idx, uint8_t lenBuff)
{
  if(idx == CTRL_SW_NONE)
  {
    strlcpy_P(buff, PSTR("--"), lenBuff);
    return;
  }
  
  char suffix[4]; //large enough to hold the range we want to convert
  memset(suffix, 0, sizeof(suffix));
  if(idx >= CTRL_SW_PHYSICAL_FIRST && idx <= CTRL_SW_PHYSICAL_LAST)
  {
    bool isInvert = ((idx - CTRL_SW_PHYSICAL_FIRST) % 6) >= 3;
    strlcpy_P(buff, isInvert ? PSTR("!Sw") : PSTR("Sw"), lenBuff);
    suffix[0] = 'A' + ((idx - CTRL_SW_PHYSICAL_FIRST) / 6);
    uint8_t pos = (idx - CTRL_SW_PHYSICAL_FIRST) % 3;
    if(pos == 0) suffix[1] = 0x18; //up glyph
    if(pos == 1) suffix[1] = '-';  //mid glyph
    if(pos == 2) suffix[1] = 0x19; //down glyph
    strlcat(buff, suffix, lenBuff);
  }
  else if(idx >= CTRL_SW_LOGICAL_FIRST && idx <= CTRL_SW_LOGICAL_LAST)
  {
    strlcpy_P(buff, PSTR("L"), lenBuff);
    itoa((idx - CTRL_SW_LOGICAL_FIRST) + 1, suffix, 10); 
    strlcat(buff, suffix, lenBuff);
  }
  else if(idx >= CTRL_SW_LOGICAL_FIRST_INVERT && idx <= CTRL_SW_LOGICAL_LAST_INVERT)
  {
    strlcpy_P(buff, PSTR("!L"), lenBuff);
    itoa((idx - CTRL_SW_LOGICAL_FIRST_INVERT) + 1, suffix, 10); 
    strlcat(buff, suffix, lenBuff);
  }
  else //flight modes as control switches
  {
    uint8_t i = idx - CTRL_SW_COUNT;
    if(i < NUM_FLIGHT_MODES)
      strlcpy_P(buff, PSTR("FMD"), lenBuff);
    else
    {
      strlcpy_P(buff, PSTR("!FMD"), lenBuff);
      i -= NUM_FLIGHT_MODES;
    }
    itoa(i + 1, suffix, 10);
    strlcat(buff, suffix, lenBuff);
  }
}

//--------------------------------------------------------------------------------------------------

void getControlSwitchName_Clean(char* buff, uint8_t idx, uint8_t lenBuff)
{
  //This function substitutes non printable ascii characters and the "--" 
  //in the switch name so we can write it to a human readable text file format
  
  if(idx == CTRL_SW_NONE)
  {
    strlcpy_P(buff, PSTR("None"), lenBuff);
    return;
  }
  
  getControlSwitchName(buff, idx, lenBuff);
  
  if(idx >= CTRL_SW_PHYSICAL_FIRST && idx <= CTRL_SW_PHYSICAL_LAST)
  {
    //search and replace
    for(uint8_t i = 0; i < lenBuff - 1; i++)
    {
      uint8_t c = *(buff + i);
      if(c == 0x18){ strlcpy_P(buff + i, PSTR("_Up"), lenBuff - i); break; }
      if(c == '-') { strlcpy_P(buff + i, PSTR("_Mid"), lenBuff - i); break; }
      if(c == 0x19){ strlcpy_P(buff + i, PSTR("_Down"), lenBuff - i); break; }
    }
  }
}

