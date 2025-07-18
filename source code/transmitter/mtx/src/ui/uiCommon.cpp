
#include "Arduino.h"

#include "../../config.h"
#include "../common.h"
#include "../mathHelpers.h"
#include "../inputs.h"
#include "../mixer.h"
#include "../sd/sdstore.h"
#include "uiCommon.h"

bool isEditMode = false;

const char* toastText;
uint32_t toastEndTime;
uint32_t toastStartTime;
bool     toastExpired = true;

//--------------------------------------------------------------------------------------------------

int16_t incDec(int16_t val, int16_t lowerLimit, int16_t upperLimit, bool wrapEnabled, uint8_t speed)
{
  return incDec(val, lowerLimit, upperLimit, wrapEnabled, speed, speed);
}

int16_t incDec(int16_t val, int16_t lowerLimit, int16_t upperLimit, bool wrapEnabled, uint8_t initialSpeed, uint8_t finalSpeed)
{
  //Increments/decrements the passed value between the specified limits inclusive. 
  //If wrap is enabled, wraps around when either limit is reached.
  
  if(!isEditMode)
    return val;
  
  if(buttonCode != KEY_UP && buttonCode != KEY_DOWN)
    return val;
  
  uint8_t  heldBtnQQ = 0;
  int16_t delta = 1;
  uint8_t speed = initialSpeed;
  uint32_t timeQQ = millis() - buttonStartTime;
  uint32_t timeOffset = 0;
  
  if(initialSpeed == INCDEC_SLOW && finalSpeed == INCDEC_NORMAL)
  {
    if(timeQQ > 2000 + LONGPRESSDELAY)
    {
      speed = INCDEC_NORMAL;
      timeOffset = 2000;
    }
  }
  else if(initialSpeed == INCDEC_SLOW && finalSpeed == INCDEC_FAST)
  {
    if(timeQQ > 2000 + LONGPRESSDELAY)
    {
      speed = INCDEC_NORMAL;
      timeOffset = 2000;
    }
    if(timeQQ > 6000 + LONGPRESSDELAY)
    {
      speed = INCDEC_FAST;
      timeOffset = 6000;
    }
  }
  else if(initialSpeed == INCDEC_NORMAL && finalSpeed == INCDEC_FAST)
  {
    if(timeQQ > 4000 + LONGPRESSDELAY)
    {
      speed = INCDEC_FAST;
      timeOffset = 4000;
    }
  }

  switch(speed)
  {
    case INCDEC_SLOW:
      if((thisLoopNum - heldButtonEntryLoopNum) % divRoundClosest(120, fixedLoopTime) == 0) //about 8.3 items per second
        heldBtnQQ = heldButton;
      break;

    case INCDEC_NORMAL:
      delta = 1;
      if(timeQQ > (2000 + LONGPRESSDELAY + timeOffset))
        delta = 2;
      heldBtnQQ = heldButton;
      break;
      
    case INCDEC_FAST:
      delta = 10;
      if(timeQQ > (2000 + LONGPRESSDELAY + timeOffset))
        delta = 100;
      heldBtnQQ = heldButton;
      break;
  }

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

  //inc dec
  if(pressedButton == incrKey || heldBtnQQ == incrKey)
  {
    val += delta;
    if(val > upperLimit)
    {
      if(wrapEnabled) val = lowerLimit;
      else val = upperLimit;
    }
  }
  else if(pressedButton == decrKey || heldBtnQQ == decrKey)
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
  
  if(buttonCode != KEY_UP && buttonCode != KEY_DOWN)
    return val;
  
  //use an array to hold the valid sources
  uint8_t srcQQ[TOTAL_SOURCES_COUNT]; 
  uint8_t srcCount = 0;
  for(uint8_t i = 0; i < (sizeof(srcQQ)/sizeof(srcQQ[0])); i++)
  {
    if(i == SRC_NONE)
    {
      //nothing here
    }
    else if(i < MIX_SOURCES_COUNT) //mix sources
    {
      if(!(flag & INCDEC_FLAG_MIX_SRC))
      {
        if(!(flag & INCDEC_FLAG_MIX_SRC_RAW_ANALOG))
          continue;
      }
      if(flag & INCDEC_FLAG_MIX_SRC_RAW_ANALOG)
      {
        if(i < SRC_RAW_ANALOG_FIRST || i > SRC_RAW_ANALOG_LAST)
          continue;
      }
      if(Model.type == MODEL_TYPE_OTHER && (i == SRC_RUD || i == SRC_AIL || i == SRC_ELE || i == SRC_THR))
        continue;
      if(i >= SRC_SW_PHYSICAL_FIRST && i <= SRC_SW_PHYSICAL_LAST)
      {
        if(Sys.swType[i - SRC_SW_PHYSICAL_FIRST] == SW_ABSENT)
          continue;
      }
      else if(i >= SRC_STICK_AXIS_FIRST && i <= SRC_STICK_AXIS_LAST)
      {
        if(Sys.StickAxis[i - SRC_STICK_AXIS_FIRST].type == STICK_AXIS_ABSENT)
          continue;
      }
      else if(i >= SRC_KNOB_FIRST && i <= SRC_KNOB_LAST)
      {
        if(Sys.Knob[i - SRC_KNOB_FIRST].type == KNOB_ABSENT)
          continue;
      }
      else if(i == SRC_X1_TRIM)
      {
        if(Model.X1Trim.trimState == TRIM_DISABLED)
          continue;
      }
      else if(i == SRC_Y1_TRIM)
      {
        if(Model.Y1Trim.trimState == TRIM_DISABLED)
          continue;
      }
      else if(i == SRC_X2_TRIM)
      {
        if(Model.X2Trim.trimState == TRIM_DISABLED)
          continue;
      }
      else if(i == SRC_Y2_TRIM)
      {
        if(Model.Y2Trim.trimState == TRIM_DISABLED)
          continue;
      }
    }
    else if(i >= SRC_COUNTER_FIRST && i <= SRC_COUNTER_LAST) //counters
    {
      if(!(flag & INCDEC_FLAG_COUNTER_AS_SRC))
        continue;
    }
    else if(i >= SRC_TIMER_FIRST && i <= SRC_TIMER_LAST) //timers
    {
      if(!(flag & INCDEC_FLAG_TIMER_AS_SRC))
        continue;
    }
    else if(i >= SRC_TELEMETRY_FIRST && i <= SRC_TELEMETRY_LAST) //telemetry
    {
      if(!(flag & INCDEC_FLAG_TELEM_AS_SRC))
        continue;
      uint8_t idx = i - SRC_TELEMETRY_FIRST;
      //skip empty telemetry
      if(isEmptyStr(Model.Telemetry[idx].name, sizeof(Model.Telemetry[0].name)))
        continue;
      //skip special telemetry
      if(Model.Telemetry[idx].type == TELEMETRY_TYPE_GNSS)
        continue;
    }
    else if(i == SRC_INACTIVITY_TIMER) //inactivity timer
    {
      if(!(flag & INCDEC_FLAG_INACTIVITY_TIMER_AS_SRC))
        continue;
    }
    else if(i == SRC_TX_BATTERY_VOLTAGE) //tx battery voltage
    {
      if(!(flag & INCDEC_FLAG_TX_BATTV_AS_SRC))
        continue;
    }
    
    srcQQ[srcCount] = i;
    srcCount++;
  }

  if(srcCount == 0)
    return val;
  
  uint8_t idxQQ = 0;
  for(uint8_t i = 0; i < srcCount; i++) //search for a match
  {
    if(srcQQ[i] == val)
    {
      idxQQ = i;
      break;
    }
  }

  idxQQ = incDec(idxQQ, 0, srcCount - 1, INCDEC_WRAP, INCDEC_SLOW);
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
    else if(Sys.swType[i] == SW_2POS || Sys.swType[i] == SW_2POS_MOMENTARY) //only up and down exist for a two position switch
    {
      uint8_t pos = (idx - CTRL_SW_PHYSICAL_FIRST) % 6;
      if(pos == 1 || pos >= 3)
        result = false;
    }
  }
  return result;
}

//--------------------------------------------------------------------------------------------------

uint8_t incDecControlSwitch(uint8_t val, uint8_t flag)
{
  if(!isEditMode)
    return val;
  
  //detect moved switch
  uint8_t movedCtrlSw = getMovedControlSwitch();
  if(movedCtrlSw != CTRL_SW_NONE)
  {
    if((flag & INCDEC_FLAG_TRIM_AS_SW) && (movedCtrlSw >= CTRL_SW_TRIM_FIRST && movedCtrlSw <= CTRL_SW_TRIM_LAST))
      val = movedCtrlSw;
    else if((flag & INCDEC_FLAG_PHY_SW) && (movedCtrlSw >= CTRL_SW_PHYSICAL_FIRST && movedCtrlSw <= CTRL_SW_PHYSICAL_LAST))
    {
      static uint8_t lastCondition[NUM_PHYSICAL_SWITCHES];
      uint8_t idx = (movedCtrlSw - CTRL_SW_PHYSICAL_FIRST) / 6;
      uint8_t condition = (movedCtrlSw - CTRL_SW_PHYSICAL_FIRST) % 6; //up, mid, down, !up, !mid, !down. The inverts are not relevant here.
      enum { 
        _UP = 0,
        _MID = 1,
        _DOWN = 2
      };
      static bool initialised = false;
      if(!initialised)
      {
        initialised = true;
        for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
        {
          if(Sys.swType[idx] == SW_2POS_MOMENTARY)
            lastCondition[idx] = _UP;
          else if(Sys.swType[idx] == SW_3POS_MOMENTARY)
            lastCondition[idx] = _MID;
        }
      }
      static uint8_t lastVal[NUM_PHYSICAL_SWITCHES];

      if(Sys.swType[idx] == SW_2POS_MOMENTARY)
      {
        if(condition == _DOWN && lastCondition[idx] == _UP)
        {
          lastCondition[idx] = _DOWN;
          //toggle between positions
          if((lastVal[idx] - CTRL_SW_PHYSICAL_FIRST) % 6 == _DOWN)
            val = CTRL_SW_PHYSICAL_FIRST + (idx * 6) + _UP;
          else
            val = CTRL_SW_PHYSICAL_FIRST + (idx * 6) + _DOWN;
        }
        else if(condition == _UP)
          lastCondition[idx] = _UP;
      }
      else if(Sys.swType[idx] == SW_3POS_MOMENTARY)
      {
        if(condition == _DOWN && lastCondition[idx] == _MID)
        {
          lastCondition[idx] = _DOWN;
          //toggle between positions
          if((lastVal[idx] - CTRL_SW_PHYSICAL_FIRST) % 6 == _DOWN)
            val = CTRL_SW_PHYSICAL_FIRST + (idx * 6) + _MID;
          else
            val = CTRL_SW_PHYSICAL_FIRST + (idx * 6) + _DOWN;
        }
        else if(condition == _UP && lastCondition[idx] == _MID) 
        {
          lastCondition[idx] = _UP;
          //toggle between positions
          if((lastVal[idx] - CTRL_SW_PHYSICAL_FIRST) % 6 == _UP)
            val = CTRL_SW_PHYSICAL_FIRST + (idx * 6) + _MID;
          else
            val = CTRL_SW_PHYSICAL_FIRST + (idx * 6) + _UP;
        }
        else if(condition == _MID)
          lastCondition[idx] = _MID;
      }
      else if(Sys.swType[idx] == SW_2POS || Sys.swType[idx] == SW_3POS)
      {
        val = movedCtrlSw;
      }
      
      //store last value
      lastVal[idx] = val;
    }
  }

  if(buttonCode != KEY_UP && buttonCode != KEY_DOWN)
    return val;
  
  //use an array to hold the valid parameters
  uint8_t ctrlQQ[CTRL_SW_COUNT];
  uint8_t ctrlCnt = 0; 
  for(uint8_t i = 0; i < (sizeof(ctrlQQ)/sizeof(ctrlQQ[0])); i++)
  {
    if(i == CTRL_SW_NONE)
    {
      //nothing here
    }
    else if(i >= CTRL_SW_PHYSICAL_FIRST && i <= CTRL_SW_PHYSICAL_LAST)
    {
      if(!(flag & INCDEC_FLAG_PHY_SW))
        continue;
      if(!controlSwitchExists(i)) //skip absent control switches
        continue;
    }
    else if(i >= CTRL_SW_LOGICAL_FIRST && i <= CTRL_SW_LOGICAL_LAST_INVERT)
    {
      if(!(flag & INCDEC_FLAG_LGC_SW))
        continue;
    }
    else if(i >= CTRL_SW_FMD_FIRST && i <= CTRL_SW_FMD_LAST_INVERT)
    {
      if(!(flag & INCDEC_FLAG_FMODE_AS_SW) || Model.type == MODEL_TYPE_OTHER)
        continue;
    }
    else if(i >= CTRL_SW_TRIM_FIRST && i <= CTRL_SW_TRIM_LAST)
    {
      if(!(flag & INCDEC_FLAG_TRIM_AS_SW))
        continue;
    }
    ctrlQQ[ctrlCnt] = i;
    ctrlCnt++;
  }

  if(ctrlCnt == 0)
    return val;

  uint8_t idxQQ = 0;
  for(uint8_t i = 0; i < ctrlCnt; i++) //search for a match
  {
    if(ctrlQQ[i] == val)
    {
      idxQQ = i;
      break;
    }
  }

  idxQQ = incDec(idxQQ, 0, ctrlCnt - 1, INCDEC_WRAP, INCDEC_SLOW);
  return ctrlQQ[idxQQ];
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
  if((start + numRequired) > NUM_MIX_SLOTS)
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
  for(uint8_t i = start; i < (start + numRequired) && i < NUM_MIX_SLOTS; i++)
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
  // Detects which control switch is moved, so we can auto select it in the UI.
  // Physical Switches (up, mid, down states) are detected. Invert of these are not detected.
  // Logical Switches are not detected.
  // Trims as switches are detected.
  // Flight modes as switches are not detected.

  if(!Sys.autoSelectMovedControl)
    return CTRL_SW_NONE;
  
  //use an array to hold the allowed control switches
  uint8_t swQQ[(NUM_PHYSICAL_SWITCHES * 6) + (CTRL_SW_TRIM_LAST - CTRL_SW_TRIM_FIRST + 1)];

  static uint8_t lastState[((sizeof(swQQ)/sizeof(swQQ[0])) + 7) / 8]; //store as 1 bit per control switch to save on memory

  //populate array
  uint8_t swCount = 0;
  for(uint8_t i = 0; i < CTRL_SW_COUNT; i++)
  {
    bool valid = false;
    if(i >= CTRL_SW_PHYSICAL_FIRST && i <= CTRL_SW_PHYSICAL_LAST)
    {
      if(((i - CTRL_SW_PHYSICAL_FIRST) % 6) <= 2) // up, mid, down
        valid = true;
    }
    else if(i >= CTRL_SW_TRIM_FIRST && i <= CTRL_SW_TRIM_LAST)
      valid = true;
    if(valid)
    {
      swQQ[swCount] = i;
      swCount++;
      if(swCount >= (sizeof(swQQ)/sizeof(swQQ[0])))
        break;
    }
  }

  //detect moved switch
  uint8_t movedSw = CTRL_SW_NONE;
  static uint32_t lastLoopNum;
  for(uint8_t idxQQ = 0; idxQQ < swCount; idxQQ++)
  {
    bool state = checkSwitchCondition(swQQ[idxQQ]);
    uint8_t idxLastState = idxQQ / 8;
    uint8_t bit = idxQQ % 8;
    if(state != ((lastState[idxLastState] >> bit) & 0x01))
    {
      if(state) //set the bit
        lastState[idxLastState] |= (1 << bit);
      else //clear the bit
        lastState[idxLastState] &= ~(1 << bit);
        
      if(state && (thisLoopNum - lastLoopNum == 1))
        movedSw = swQQ[idxQQ];
    }
  }
  lastLoopNum = thisLoopNum;
  return movedSw;
}

//--------------------------------------------------------------------------------------------------

uint8_t getMovedSource()
{
  //Detects which source is moved.
  //Stick axes, knobs, physical switches, stick axis trims.
  
  if(!Sys.autoSelectMovedControl)
    return SRC_NONE;
  
  //use array to hold values for the valid sources
  uint8_t srcQQ[(SRC_RAW_ANALOG_LAST - SRC_RAW_ANALOG_FIRST + 1) + NUM_PHYSICAL_SWITCHES + (SRC_TRIM_LAST - SRC_TRIM_FIRST + 1)];
  static int8_t lastValQQ[sizeof(srcQQ)/sizeof(srcQQ[0])];

  //populate array
  uint8_t srcCount = 0; 
  for(uint8_t i = 0; i < MIX_SOURCES_COUNT; i++)
  {
    bool valid = false;
    if(i >= SRC_RAW_ANALOG_FIRST && i <= SRC_RAW_ANALOG_LAST)
      valid = true;
    else if(i >= SRC_SW_PHYSICAL_FIRST && i <= SRC_SW_PHYSICAL_LAST)
      valid = true;
    else if(i >= SRC_TRIM_FIRST && i <= SRC_TRIM_LAST)
      valid = true;
    if(valid)
    {
      srcQQ[srcCount] = i;
      srcCount++;
      if(srcCount >= (sizeof(srcQQ)/sizeof(srcQQ[0])))
        break;
    }
  }

  //detect moved source
  uint8_t movedSrc = SRC_NONE;
  static uint32_t lastLoopNum;
  for(uint8_t idxQQ = 0; idxQQ < srcCount; idxQQ++)
  {
    if(srcQQ[idxQQ] >= SRC_TRIM_FIRST && srcQQ[idxQQ] <= SRC_TRIM_LAST)
    {
      int16_t difference = mixSources[srcQQ[idxQQ]] - lastValQQ[idxQQ];
      if(difference != 0) 
      {
        lastValQQ[idxQQ] = mixSources[srcQQ[idxQQ]];
        if(thisLoopNum - lastLoopNum == 1)
          movedSrc = srcQQ[idxQQ];
      }
    }
    else
    {
      int16_t difference = mixSources[srcQQ[idxQQ]]/5 - lastValQQ[idxQQ];
      if(difference > 30 || difference < -30)
      {
        lastValQQ[idxQQ] = mixSources[srcQQ[idxQQ]] / 5;
        if(thisLoopNum - lastLoopNum == 1)
          movedSrc = srcQQ[idxQQ];
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
  if(hasWarning && ((thisLoopNum - lastLoopNum) % (5000 / fixedLoopTime) == 0) && !telemetryMuteAlarms)
    audioToPlay = AUDIO_TELEM_WARNING;
}

//--------------------------------------------------------------------------------------------------

void inactivityAlarmHandler()
{
  if(!Sys.soundOnInactivity || Sys.inactivityMinutes == 0)
    return;

  static uint32_t lastPlayMillis = 0;
  if((millis() - inputsLastMovedTime) > ((uint32_t)Sys.inactivityMinutes * 60000 + 7000)) 
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
    if(Model.Timer[i].initialSeconds > 0)
    {
      uint32_t initMillis = (uint32_t) Model.Timer[i].initialSeconds * 1000;
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

//--------------------------------------------------------------------------------------------------

void notificationHandler()
{
  typedef struct {
    uint8_t notificationId;
  } queue_t;

  const uint8_t MAX_QUEUE_SIZE = 3; //maximum length of the queue. Keep it small
  static queue_t queue[MAX_QUEUE_SIZE];

  static uint8_t lastId = 0xff;
  static bool lastState[NUM_CUSTOM_NOTIFICATIONS];

  static bool initialised = false;

  static uint8_t lastModelIdx = 0xff; 
  if(Sys.activeModelIdx != lastModelIdx) //reset if model changed
  {
    lastModelIdx = Sys.activeModelIdx;
    initialised = false;
  }

  if(!initialised)
  {
    initialised = true;
    for(uint8_t i = 0; i < MAX_QUEUE_SIZE; i++)
    {
      queue[i].notificationId = 0xff;
    }
    for(uint8_t i = 0; i < NUM_CUSTOM_NOTIFICATIONS; i++)
    {
      lastState[i] = checkSwitchCondition(Model.CustomNotification[i].swtch);
    }
    lastId = 0xff;
  }

  //determine the queue's current size
  uint8_t queueSize = 0;
  for(uint8_t i = 0; i < MAX_QUEUE_SIZE; i++)
  {
    if(queue[i].notificationId != 0xff)
      queueSize++;
  }

  //--- Add to the queue
  
  for(uint8_t i = 0; i < NUM_CUSTOM_NOTIFICATIONS; i++)
  {
    if(!Model.CustomNotification[i].enabled)
      continue;
    uint8_t sw = Model.CustomNotification[i].swtch;
    bool state = (checkSwitchCondition(sw) && sw != CTRL_SW_NONE);
    if(state && !lastState[i]) //just went on
    {
      lastState[i] = true;
      //Add to the queue if not already queued and the queue is not full. Enqueue at rear
      if(queueSize < MAX_QUEUE_SIZE)
      {
        bool alreadyQueued = false;
        for(uint8_t j = 0; j < MAX_QUEUE_SIZE; j++)
        {
          if(queue[j].notificationId == i)
          {
            alreadyQueued = true;
            break;
          }
        }
        
        if(!alreadyQueued)
        {
          queue[queueSize].notificationId = i;
          queueSize++; //update the size
        }
      }
    }
    if(!state) //went off
    {
      lastState[i] = false;
    }
  }

  //--- Process queue
  //Here, queue item 0 is considered the head
  
  if(queueSize > 0)
  {
    static bool toneStarted = false;
    static uint32_t startTime;
    static uint32_t endTime;

    if(lastId != queue[0].notificationId)
    {
      lastId = queue[0].notificationId;
      startTime = millis();
      endTime = startTime + 3500;
      toneStarted = false;
    }
    
    if(millis() < endTime)
    {
      //show notification
      if(!isEmptyStr(Model.CustomNotification[queue[0].notificationId].text, sizeof(Model.CustomNotification[0].text)))
      {
        drawNotificationOverlay(queue[0].notificationId, startTime, endTime); 
      }
      //play the specified melody
      if(!toneStarted)
      {
        toneStarted = true;
        audioToPlay = Model.CustomNotification[queue[0].notificationId].tone;
      }
    }
    else 
    {
      //--- Dequeue
      //Item 0 has been processed, remove it.
      //Here we shift array elements to the left by one position.
      for(uint8_t i = 0; i < MAX_QUEUE_SIZE - 1; i++)
      {
        queue[i] = queue[i + 1];
      }
      queue[MAX_QUEUE_SIZE - 1].notificationId = 0xff;
      //decrement the count
      queueSize--;
      //reset other variables
      lastId = 0xff;
    }
  }
}

//-------------------------------------------------------------------------------------------------

void screenshotHandler()
{
  static bool lastState = false;
  static bool isFirstRun = true; //helps prevent nuisance screenshot when initially setting up the switch
  bool state = (screenshotSwtch != CTRL_SW_NONE && checkSwitchCondition(screenshotSwtch));
  if(state && !lastState) //just went on
  {
    lastState = true;
    if(isFirstRun)
      isFirstRun = false;
    else
    {
      if(sdWriteScreenshot())
        audioToPlay = AUDIO_SCREENSHOT_CAPTURED;
    }
  }
  if(!state) //went off
  {
    lastState = false;
  }
}
