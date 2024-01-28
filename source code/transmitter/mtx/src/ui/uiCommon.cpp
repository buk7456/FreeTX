
#include "Arduino.h"

#include "../../config.h"
#include "../common.h"
#include "../mathHelpers.h"
#include "../mixer.h"
#include "uiCommon.h"

bool isEditMode = false;

uint32_t timerLastElapsedTime[NUM_TIMERS];
uint32_t timerLastPaused[NUM_TIMERS];
bool     timerIsRunning[NUM_TIMERS];
bool     timerForceRun[NUM_TIMERS];

const char* toastText;
uint32_t toastEndTime;
uint32_t toastStartTime;
bool     toastExpired = true;

//--------------------------------------------------------------------------------------------------

int16_t incDec(int16_t val, int16_t lowerLimit, int16_t upperLimit, bool wrapEnabled, uint8_t state)
{
  //Increments/decrements the passed value between the specified limits inclusive. 
  //If wrap is enabled, wraps around when either limit is reached.
  
  if(!isEditMode)
    return val;

  uint8_t  _heldBtn = 0;
  uint32_t _timeOffset = 0;
  
  if(state == INCDEC_SLOW || state == INCDEC_SLOW_START)
  {
    if((thisLoopNum - heldButtonEntryLoopNum) % divRoundClosest(120, fixedLoopTime) == 0) //about 8.3 items per second
      _heldBtn = heldButton;
    if(state == INCDEC_SLOW_START && millis() - buttonStartTime > (LONGPRESSDELAY + 2000UL))
    {
      state = INCDEC_NORMAL; 
      _timeOffset = 2000;
    }
  }
  
  if(state == INCDEC_NORMAL || state == INCDEC_FAST)
    _heldBtn = heldButton;

  //Default is KEY_UP increments, KEY_DOWN decrements
  uint8_t incrKey = KEY_UP;
  uint8_t decrKey = KEY_DOWN;
  if(lowerLimit > upperLimit) // KEY_UP decrements, KEY_DOWN increments 
  {
    //swap lower and upper limits
    int16_t _tmp = lowerLimit;
    lowerLimit = upperLimit;
    upperLimit = _tmp;
    //swap key actions
    incrKey = KEY_DOWN;
    decrKey = KEY_UP;
  }
  
  int16_t delta = 1;
  if(state >= INCDEC_NORMAL && _heldBtn > 0)
  {
    uint32_t holdTime = millis() - (buttonStartTime + LONGPRESSDELAY);
    if(holdTime > (2000UL + _timeOffset))
    {
      //only speed up when the fixedLoopTime is above 20, otherwise it might get too fast
      //resulting in overly overshooting the desired value
      if(fixedLoopTime >= 20)
        delta = 2; 
    }
    if(holdTime > (4000UL + _timeOffset) && state == INCDEC_FAST)
      delta = 10;
    if(holdTime > (8000UL + _timeOffset) && state == INCDEC_FAST)
      delta = 100;
  }

  //inc dec
  if(pressedButton == incrKey || _heldBtn == incrKey)
  {
    val += delta;
    if(val > upperLimit)
    {
      if(wrapEnabled) val = lowerLimit;
      else val = upperLimit;
    }
  }
  else if(pressedButton == decrKey || _heldBtn == decrKey)
  {
    val -= delta;
    if(val < lowerLimit)
    {
      if(wrapEnabled) val = upperLimit;
      else val = lowerLimit;
    }
  }

  return val;
}

//--------------------------------------------------------------------------------------------------

uint8_t incDecSource(uint8_t val, uint8_t flag)
{
  if(!isEditMode)
    return val;
  
  //use an array to hold the valid sources
  uint8_t srcQQ[MIXSOURCES_COUNT + NUM_COUNTERS + NUM_TIMERS + NUM_CUSTOM_TELEMETRY];
  uint8_t srcCnt = 0;
  for(uint8_t i = 0; i < sizeof(srcQQ); i++)
  {
    if(i < MIXSOURCES_COUNT) //mix sources
    {
      if(!(flag & INCDEC_FLAG_MIX_SRC))
        continue;
      if(Model.type == MODEL_TYPE_OTHER && (i == SRC_RUD || i == SRC_AIL || i == SRC_ELE || i == SRC_THR))
        continue;
      if(i >= SRC_SW_PHYSICAL_FIRST && i <= SRC_SW_PHYSICAL_LAST && Sys.swType[i - SRC_SW_PHYSICAL_FIRST] == SW_ABSENT)
        continue;
    }
    else if(i < MIXSOURCES_COUNT + NUM_COUNTERS) //counters
    {
      if(!(flag & INCDEC_FLAG_COUNTER_SRC))
        continue;
    }
    else if(i < MIXSOURCES_COUNT + NUM_COUNTERS + NUM_TIMERS) //timers
    {
      if(!(flag & INCDEC_FLAG_TIMER_SRC))
        continue;
    }
    else //telemetry sources
    {
      if(!(flag & INCDEC_FLAG_TELEM_SRC))
        continue;
      //skip empty telemetry
      uint8_t idx = i - (MIXSOURCES_COUNT + NUM_COUNTERS + NUM_TIMERS);
      if(isEmptyStr(Model.Telemetry[idx].name, sizeof(Model.Telemetry[0].name)))
        continue;
    }
    
    srcQQ[srcCnt] = i;
    srcCnt++;
  }
  
  uint8_t idxQQ = 0;
  for(uint8_t i = 0; i < srcCnt; i++) //search for a match
  {
    if(srcQQ[i] == val)
    {
      idxQQ = i;
      break;
    }
  }
  idxQQ = incDec(idxQQ, 0, srcCnt - 1, INCDEC_NOWRAP, INCDEC_SLOW);
  return srcQQ[idxQQ];
}

//--------------------------------------------------------------------------------------------------

bool controlSwitchExists(uint8_t idx)
{
  bool result = true;
  if(idx >= CTRL_SW_PHYSICAL_FIRST && idx <= CTRL_SW_PHYSICAL_LAST)
  {
    uint8_t i = (idx - CTRL_SW_PHYSICAL_FIRST) / 6;
    if(Sys.swType[i] == SW_ABSENT)
      result = false;
    else if(Sys.swType[i] == SW_2POS) //only up and down exist for a two position switch
    {
      uint8_t pos = (idx - CTRL_SW_PHYSICAL_FIRST) % 6;
      if(pos == 1 || pos >= 3)
        result = false;
    }
  }
  return result;
}

uint8_t incDecControlSwitch(uint8_t val, uint8_t flag)
{
  if(!isEditMode)
    return val;
  
  //detect moved switch
  uint8_t movedCtrlSw = getMovedControlSwitch();
  if(movedCtrlSw != CTRL_SW_NONE)
    val = movedCtrlSw;
  
  //use an array to hold the valid parameters
  uint8_t ctrlQQ[CTRL_SW_COUNT + (NUM_FLIGHT_MODES * 2)];
  uint8_t ctrlCnt = 0; 
  for(uint8_t i = 0; i < sizeof(ctrlQQ); i++)
  {
    if(i >= CTRL_SW_PHYSICAL_FIRST && i <= CTRL_SW_PHYSICAL_LAST)
    {
      if(!(flag & INCDEC_FLAG_PHY_SW))
        continue;
      if(!controlSwitchExists(i)) //skip absent control switches
        continue;
    }
    if(i >= CTRL_SW_LOGICAL_FIRST && i <= CTRL_SW_LOGICAL_LAST_INVERT)
    {
      if(!(flag & INCDEC_FLAG_LGC_SW))
        continue;
    }
    if(i >= CTRL_SW_COUNT) //flight modes
    {
      if(!(flag & INCDEC_FLAG_FMODE))
        continue;
    }
    ctrlQQ[ctrlCnt] = i;
    ctrlCnt++;
  }

  uint8_t idxQQ = 0;
  for(uint8_t i = 0; i < ctrlCnt; i++) //search for a match
  {
    if(ctrlQQ[i] == val)
    {
      idxQQ = i;
      break;
    }
  }
  idxQQ = incDec(idxQQ, 0, ctrlCnt - 1, INCDEC_NOWRAP, INCDEC_SLOW);
  return ctrlQQ[idxQQ];
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

void resetTimerRegister(uint8_t idx)
{
  timerElapsedTime[idx] = 0;
  timerLastElapsedTime[idx] = 0;
  timerLastPaused[idx] = millis();
  timerIsRunning[idx] = false;
}

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

void resetCounterRegisters()
{
  for(uint8_t i = 0; i < NUM_COUNTERS; i++)
    resetCounterRegister(i);
}

void resetCounterRegister(uint8_t idx)
{
  counterOut[idx] = 0;
}

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

void makeToast(const char* text, uint16_t duration, uint16_t dly)
{
  toastText = text;
  toastStartTime = millis() + dly;
  toastEndTime = toastStartTime + duration;
  toastExpired = false;
}

//--------------------------------------------------------------------------------------------------

bool hasEnoughMixSlots(uint8_t start, uint8_t numRequired)
{
  if((start + numRequired) > NUM_MIXSLOTS)
  {
    makeToast(PSTR("Can't load here"), 2000, 0);
    return false;
  }
  else 
    return true;
}

//--------------------------------------------------------------------------------------------------

bool hasOccupiedMixSlots(uint8_t start, uint8_t numRequired)
{
  for(uint8_t i = start; i < (start + numRequired) && i < NUM_MIXSLOTS; i++)
  {
    if(Model.Mixer[i].output != SRC_NONE || Model.Mixer[i].input != SRC_NONE)
      return true;
  }
  return false;
}

//--------------------------------------------------------------------------------------------------

uint8_t getLSFuncGroup(uint8_t func)
{
  uint8_t result = 0;
  if(func <= LS_FUNC_GROUP1_LAST) result = 1;
  else if(func <= LS_FUNC_GROUP2_LAST) result = 2;
  else if(func <= LS_FUNC_GROUP3_LAST) result = 3;
  else if(func <= LS_FUNC_GROUP4_LAST) result = 4;
  else if(func <= LS_FUNC_GROUP5_LAST) result = 5;
  else if(func <= LS_FUNC_GROUP6_LAST) result = 6;
  else if(func <= LS_FUNC_GROUP7_LAST) result = 7;
  return result;
}

//--------------------------------------------------------------------------------------------------

uint8_t getMovedControlSwitch()
{
  //Detect which control switch is moved, so we can auto select it in the UI.
  //Only UP, MID, DOWN allowed. Inverse of these, and also the logical switches don't apply here.
  if(!Sys.autoSelectMovedControl)
    return CTRL_SW_NONE;
  static bool lastState[MAX_NUM_PHYSICAL_SWITCHES * 6]; 
  uint8_t movedSw = CTRL_SW_NONE;
  static uint32_t lastLoopNum;
  for(uint8_t i = CTRL_SW_PHYSICAL_FIRST; i <= CTRL_SW_PHYSICAL_LAST; i++)
  {
    uint8_t pos = (i - CTRL_SW_PHYSICAL_FIRST) % 6;
    if(pos > 2) // !up, !mid, !down. Skip these
      continue;
    
    bool state = checkSwitchCondition(i);
    if(state != lastState[i - CTRL_SW_PHYSICAL_FIRST])
    {
      lastState[i - CTRL_SW_PHYSICAL_FIRST] = state;
      if(state && (thisLoopNum - lastLoopNum == 1))
        movedSw = i;
    }
  }
  lastLoopNum = thisLoopNum;
  return movedSw;
}

//--------------------------------------------------------------------------------------------------

uint8_t getMovedSource()
{
  //Detects which source is moved
  //Only detects stick axes, knobs and physical switches.
  
  if(!Sys.autoSelectMovedControl)
    return SRC_NONE;
  //use array to hold values for the valid sources
  const uint8_t srcCnt = (SRC_RAW_ANALOG_LAST - SRC_RAW_ANALOG_FIRST + 1) + MAX_NUM_PHYSICAL_SWITCHES;
  uint8_t  srcQQ[srcCnt];
  static int8_t lastValQQ[srcCnt];
  //populate array
  uint8_t cnt = 0; 
  for(uint8_t i = 0; i < MIXSOURCES_COUNT; i++)
  {
    if((i >= SRC_RAW_ANALOG_FIRST && i <= SRC_RAW_ANALOG_LAST) || (i >= SRC_SW_PHYSICAL_FIRST && i <= SRC_SW_PHYSICAL_LAST))
    {
      srcQQ[cnt] = i;
      cnt++;
      if(cnt >= srcCnt)
        break;
    }
  }
  uint8_t movedSrc = SRC_NONE;
  static uint32_t lastLoopNum;
  for(uint8_t i = 0; i < MIXSOURCES_COUNT; i++)
  {
    if((i >= SRC_RAW_ANALOG_FIRST && i <= SRC_RAW_ANALOG_LAST) || (i >= SRC_SW_PHYSICAL_FIRST && i <= SRC_SW_PHYSICAL_LAST))
    {
      //find corresponding index in the srcQQ array
      uint8_t idxQQ = 0;
      for(uint8_t j = 0; j < srcCnt; j++)
      {
        if(srcQQ[j] == i)
        {
          idxQQ = j;
          break;
        }
      }
      //check if moved
      int16_t difference = mixSources[i]/5 - lastValQQ[idxQQ];
      if(difference > 30 || difference < -30) 
      {
        lastValQQ[idxQQ] = mixSources[i] / 5;
        if(thisLoopNum - lastLoopNum == 1)
          movedSrc = i;
      }
    }
  }
  lastLoopNum = thisLoopNum;
  return movedSrc; 
}

//--------------------------------------------------------------------------------------------------

uint8_t procMovedSource(uint8_t src)
{
  if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
  {
    if(src == Model.rudSrcRaw) return SRC_RUD;
    if(src == Model.ailSrcRaw) return SRC_AIL;
    if(src == Model.eleSrcRaw) return SRC_ELE;
    if(src == Model.thrSrcRaw) return SRC_THR;
  }
  return src;
}

//--------------------------------------------------------------------------------------------------

void calcNewCurvePts(custom_curve_t *crv, uint8_t numOldPts)
{
  //old values
  int16_t xValOld[MAX_NUM_POINTS_CUSTOM_CURVE];
  int16_t yValOld[MAX_NUM_POINTS_CUSTOM_CURVE];
  for(uint8_t pt = 0; pt < numOldPts; pt++)
  {
    xValOld[pt] = 5 * crv->xVal[pt];
    yValOld[pt] = 5 * crv->yVal[pt];
  }
  //calculate new values 
  for(uint8_t pt = 0; pt < crv->numPoints; pt++)
  {
    int16_t xNew = 5 * (-100 + ((200 * pt)/(crv->numPoints - 1)));
    //calc new y value by interpolating using old data
    int16_t yNew;
    if(crv->smooth)
      yNew = cubicHermiteInterpolate(xValOld, yValOld, numOldPts, xNew);
    else
      yNew = linearInterpolate(xValOld, yValOld, numOldPts, xNew);
    //write
    crv->xVal[pt] = xNew / 5;
    crv->yVal[pt] = yNew / 5;
  }
}

//--------------------------------------------------------------------------------------------------

void telemetryAlarmHandler()
{
  //determine alarm state
  for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
  {
    telemetryAlarmState[i] = 0;
    if(telemetryReceivedValue[i] == TELEMETRY_NO_DATA || Model.Telemetry[i].alarmCondition == TELEMETRY_ALARM_CONDITION_NONE)
      continue;
    int32_t tVal = ((int32_t) telemetryReceivedValue[i] * Model.Telemetry[i].multiplier) / 100;
    tVal += Model.Telemetry[i].offset;
    switch(Model.Telemetry[i].alarmCondition)
    {
      case TELEMETRY_ALARM_CONDITION_GREATER_THAN:
        if(tVal > Model.Telemetry[i].alarmThreshold) 
          telemetryAlarmState[i] = 1;
        break;
      case TELEMETRY_ALARM_CONDITION_LESS_THAN:
        if(tVal < Model.Telemetry[i].alarmThreshold) 
          telemetryAlarmState[i] = 1;
        break;
    }
  }
  
  //--- play audio
  //TODO change implementation to actually play specified melody.
  //TODO possibly use an audio queue. Only add melody if its not already queued
  
  static uint8_t tCounter[NUM_CUSTOM_TELEMETRY];
  static bool tWarnStarted[NUM_CUSTOM_TELEMETRY];
  for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
  {
    //increment or decrement counter
    if(telemetryAlarmState[i] == 1)
    {
      if(!tWarnStarted[i]) 
        ++tCounter[i];
    }
    else
    {
      if(tCounter[i] > 0) --tCounter[i];
      if(tCounter[i] == 0) tWarnStarted[i] = false;
    }
    //if more than 3 seconds, trigger warning
    if(tCounter[i] > (3000 / fixedLoopTime) && !tWarnStarted[i]) 
      tWarnStarted[i] = true;
  }
  
  //single audio instance
  static uint32_t lastLoopNum = 0;
  static bool hasWarning = false;
  for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
  {
    //search for first instance and break
    if(tWarnStarted[i])
    {
      if(!hasWarning)
        lastLoopNum = thisLoopNum;
      hasWarning = true;
      break;
    }
    if(i == NUM_CUSTOM_TELEMETRY - 1) //nothing found
      hasWarning = false;
  }
  
  //sound the alarm, repeat every 5 seconds
  if(hasWarning && ((thisLoopNum - lastLoopNum)%(5000/fixedLoopTime) == 0) && !telemetryMuteAlarms)
    audioToPlay = AUDIO_TELEM_WARN;
}

//--------------------------------------------------------------------------------------------------

void inactivityAlarmHandler()
{
  if(Sys.inactivityMinutes == 0)
    return;
  
  static uint32_t lastPlayMillis = 0;
  if((millis() - inputsLastMoved) > ((uint32_t)Sys.inactivityMinutes * 60000 + 7000)) 
  {
    //7 seconds added to the above comparison to reduce chances of colliding with other audio
    if(millis() - lastPlayMillis > 30000) //repeat every 30 seconds
    {
      lastPlayMillis = millis();
      audioToPlay = AUDIO_INACTIVITY;
    }
  }
}

//--------------------------------------------------------------------------------------------------

void timerHandler()
{
  for(uint8_t i = 0; i < NUM_TIMERS; i++)
  {
    static bool alarmTriggered[NUM_TIMERS];
    
    if(Model.Timer[i].swtch != CTRL_SW_NONE)
      timerForceRun[i] =  false;
    
    if((Model.Timer[i].swtch != CTRL_SW_NONE && checkSwitchCondition(Model.Timer[i].swtch)) 
      || timerForceRun[i]) //run timer
    {
      timerElapsedTime[i] = timerLastElapsedTime[i] + millis() - timerLastPaused[i];
      timerIsRunning[i] = true;
    }
    else //pause timer
    {
      timerLastElapsedTime[i] = timerElapsedTime[i];
      timerLastPaused[i] = millis();
      timerIsRunning[i] = false;
    }
    
    //reset timer register
    if(Model.Timer[i].resetSwitch != CTRL_SW_NONE && checkSwitchCondition(Model.Timer[i].resetSwitch))
      resetTimerRegister(i);
    
    //play sound when timer expires (if count down timer)
    if(Model.Timer[i].initialMinutes > 0)
    {
      uint32_t initMillis = Model.Timer[i].initialMinutes * 60000UL;
      if((timerElapsedTime[i] > initMillis) && timerElapsedTime[i] < (initMillis + 500) && !alarmTriggered[i])
      {
        audioToPlay = AUDIO_TIMER_ELAPSED;
        alarmTriggered[i] = true;
      }
      else if(timerElapsedTime[i] < initMillis)
        alarmTriggered[i] = false;
    }
    
    //store if persistent
    if(Model.Timer[i].isPersistent)
      Model.Timer[i].persistVal = timerElapsedTime[i];
    else
      Model.Timer[i].persistVal = 0;
  }
}
