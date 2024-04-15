#include "Arduino.h"

#include "common.h"
#include "mathHelpers.h"
#include "mixer.h"

int16_t calcExpo(int16_t input, int16_t expo);
int16_t calcDifferential(int16_t input, int16_t diff);
int32_t applySlow(int32_t currVal, int32_t targetVal, int32_t multiplier, uint32_t riseTime, uint32_t fallTime);
int16_t weightAndOffset(int16_t input, int16_t weight, int16_t offset);
int8_t  adjustTrim(int8_t val, int8_t lowerLimit, int8_t upperLimit, uint8_t incButton, uint8_t decButton);
int16_t generateWaveform(uint8_t idx, int32_t _currTime);

//mix sources array
int16_t mixSources[MIXSOURCES_COUNT]; 

//variables to help with 'delay' and 'slow' mixer features
int16_t  dlyOldVal[NUM_MIXSLOTS];
int16_t  dlyPrevOperand[NUM_MIXSLOTS];
uint8_t  dlyPrevInput[NUM_MIXSLOTS];
uint32_t dlyStartTime[NUM_MIXSLOTS];   
int32_t  slowCurrVal[NUM_MIXSLOTS]; 

//variables to help with the mix Hold feature
int16_t hldOldVal[NUM_MIXSLOTS];
uint8_t hldOldState[NUM_MIXSLOTS];

//array to store logical switches
bool logicalSwitchState[NUM_LOGICAL_SWITCHES];

bool isReinitialiseMixer = false;

//==================================================================================================

void reinitialiseMixerCalculations()
{
  isReinitialiseMixer = true;
}

//==================================================================================================

void computeChannelOutputs()
{
  if(isCalibratingSticks)
    return;

  if(isReinitialiseMixer)
  {
    //reset logical switch states here to avoid stale data in subsequent calculations 
    for(uint8_t i = 0; i < NUM_LOGICAL_SWITCHES; i++)
      logicalSwitchState[i] = false;
    //resynchronize the waveform generators
    for(uint8_t i = 0; i < NUM_FUNCGEN; i++)
      syncWaveform(i);
  }
  
  //--- Record the current time
  //This ensures consistency of the timing for various stuff irrespective of 
  //the time taken to do all the computations.
  static uint32_t timeOffset;
  if(isReinitialiseMixer)
    timeOffset = millis();
  uint32_t currMillis = millis() - timeOffset;

  //--- Determine the activeFmdIdx 
  //The first one found is considered, the subsequent ones are ignored even if their conditions are met
  activeFmdIdx = 0;
  for(uint8_t i = 0; i < NUM_FLIGHT_MODES; i++)
  {
    if(Model.FlightMode[i].swtch != CTRL_SW_NONE && checkSwitchCondition(Model.FlightMode[i].swtch))
    {
      activeFmdIdx = i;
      break;
    }
  }
  
  flight_mode_t* fmd = &Model.FlightMode[activeFmdIdx];
  
  //--- Get trim values
  //x1 axis
  if(Model.X1Trim.trimState == TRIM_COMMON)
     Model.X1Trim.commonTrim = adjustTrim( Model.X1Trim.commonTrim, -20, 20, KEY_X1_TRIM_UP, KEY_X1_TRIM_DOWN);
  else if(Model.X1Trim.trimState == TRIM_FLIGHT_MODE)
    fmd->x1Trim = adjustTrim(fmd->x1Trim, -20, 20, KEY_X1_TRIM_UP, KEY_X1_TRIM_DOWN);
    //y1 axis
  if(Model.Y1Trim.trimState == TRIM_COMMON)
     Model.Y1Trim.commonTrim = adjustTrim( Model.Y1Trim.commonTrim, -20, 20, KEY_Y1_TRIM_UP, KEY_Y1_TRIM_DOWN);
  else if(Model.Y1Trim.trimState == TRIM_FLIGHT_MODE)
    fmd->y1Trim = adjustTrim(fmd->y1Trim, -20, 20, KEY_Y1_TRIM_UP, KEY_Y1_TRIM_DOWN);
    //x2 axis
  if(Model.X2Trim.trimState == TRIM_COMMON)
    Model.X2Trim.commonTrim = adjustTrim( Model.X2Trim.commonTrim, -20, 20, KEY_X2_TRIM_UP, KEY_X2_TRIM_DOWN);
  else if(Model.X2Trim.trimState == TRIM_FLIGHT_MODE)
    fmd->x2Trim = adjustTrim(fmd->x2Trim, -20, 20, KEY_X2_TRIM_UP, KEY_X2_TRIM_DOWN);
  //y2 axis
  if(Model.Y2Trim.trimState == TRIM_COMMON)
     Model.Y2Trim.commonTrim = adjustTrim(Model.Y2Trim.commonTrim, -20, 20, KEY_Y2_TRIM_UP, KEY_Y2_TRIM_DOWN);
  else if(Model.Y2Trim.trimState == TRIM_FLIGHT_MODE)
    fmd->y2Trim = adjustTrim(fmd->y2Trim, -20, 20, KEY_Y2_TRIM_UP, KEY_Y2_TRIM_DOWN);
  
  //--- Flight mode trims fade-in
  //No need to do very precise calculations here because the range of trim is comparatively small 
  //as well as the specifiable transition time.
  //Note that the transition time is the time it takes to move from -100% to 100% or vice versa.
  
  int16_t  flightModeTrim[4]; //used also in the mixer
  static int16_t  oldFlightModeTrim[4];
  static uint32_t flightModeEntryTime;
  static uint8_t  lastFmdIdx;
  
  if(isReinitialiseMixer)
  {
    lastFmdIdx = activeFmdIdx;
    oldFlightModeTrim[0] = fmd->x1Trim * 5;
    oldFlightModeTrim[1] = fmd->y1Trim * 5;
    oldFlightModeTrim[2] = fmd->x2Trim * 5;
    oldFlightModeTrim[3] = fmd->y2Trim * 5;
  }
  
  flightModeTrim[0] = fmd->x1Trim * 5;
  flightModeTrim[1] = fmd->y1Trim * 5;
  flightModeTrim[2] = fmd->x2Trim * 5;
  flightModeTrim[3] = fmd->y2Trim * 5;

  if(activeFmdIdx != lastFmdIdx)
  {  
    lastFmdIdx = activeFmdIdx;
    flightModeEntryTime = currMillis;
  }
  
  if(currMillis - flightModeEntryTime < (fmd->transitionTime * 20)) //(100*(20/100)). Trim is 20% of full range
  {
    uint16_t tt = fmd->transitionTime * 100;
    for(uint8_t i = 0; i < 4; i++)
    {
      flightModeTrim[i] = applySlow(oldFlightModeTrim[i], flightModeTrim[i], 1, tt, tt);
      oldFlightModeTrim[i] = flightModeTrim[i];
    }
  }

  ///----------- MIX SOURCES ARRAY INITIALIZATIONS --------

  //Mix source raw sticks
  for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
    mixSources[SRC_STICK_AXIS_FIRST + i] = stickAxisIn[i];

  //Mix source knobs
  mixSources[SRC_KNOB_A] = knobAIn;
  mixSources[SRC_KNOB_B] = knobBIn;
  
  //Mix source 100Perc
  mixSources[SRC_100PERC] = 500;
  
  //Mix source Rud, Ail, Ele
  //Handle dual rate and expo
  //Rud
  if(Model.RudDualRate.swtch == CTRL_SW_NONE || !checkSwitchCondition(Model.RudDualRate.swtch))
    mixSources[SRC_RUD] = calcRateExpo(mixSources[Model.rudSrcRaw], Model.RudDualRate.rate1, Model.RudDualRate.expo1);
  else 
    mixSources[SRC_RUD] = calcRateExpo(mixSources[Model.rudSrcRaw], Model.RudDualRate.rate2, Model.RudDualRate.expo2);
  //Ail
  if(Model.AilDualRate.swtch == CTRL_SW_NONE || !checkSwitchCondition(Model.AilDualRate.swtch))
    mixSources[SRC_AIL] = calcRateExpo(mixSources[Model.ailSrcRaw], Model.AilDualRate.rate1, Model.AilDualRate.expo1);
  else 
    mixSources[SRC_AIL] = calcRateExpo(mixSources[Model.ailSrcRaw], Model.AilDualRate.rate2, Model.AilDualRate.expo2);
  //Ele
  if(Model.EleDualRate.swtch == CTRL_SW_NONE || !checkSwitchCondition(Model.EleDualRate.swtch))
    mixSources[SRC_ELE] = calcRateExpo(mixSources[Model.eleSrcRaw], Model.EleDualRate.rate1, Model.EleDualRate.expo1);
  else 
    mixSources[SRC_ELE] = calcRateExpo(mixSources[Model.eleSrcRaw], Model.EleDualRate.rate2, Model.EleDualRate.expo2);
  
  //Mix source throttle curve
  int16_t _xxQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
  int16_t _yyQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
  for(uint8_t pt = 0; pt < Model.ThrottleCurve.numPoints; pt++)
  {
    _xxQQ[pt] = 5 * Model.ThrottleCurve.xVal[pt];
    _yyQQ[pt] = 5 * Model.ThrottleCurve.yVal[pt];
  }
  if(Model.ThrottleCurve.smooth)
    mixSources[SRC_THR] = cubicHermiteInterpolate(_xxQQ, _yyQQ, Model.ThrottleCurve.numPoints, mixSources[Model.thrSrcRaw]);
  else
    mixSources[SRC_THR] = linearInterpolate(_xxQQ, _yyQQ, Model.ThrottleCurve.numPoints, mixSources[Model.thrSrcRaw]);

  //Mix source switches
  for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
  {
    if(Sys.swType[i] == SW_2POS)
      mixSources[SRC_SW_PHYSICAL_FIRST + i] = (swState[i] == SWLOWERPOS) ? 500 : -500;
    else if(Sys.swType[i] == SW_3POS)
      mixSources[SRC_SW_PHYSICAL_FIRST + i] = (swState[i] == SWLOWERPOS) ? 500 : ((swState[i] == SWUPPERPOS) ? -500 : 0);
  }

  //Mix sources logical switches
  for(uint8_t i = 0; i < NUM_LOGICAL_SWITCHES; i++)
    mixSources[SRC_SW_LOGICAL_FIRST + i] = logicalSwitchState[i] ? 500 : -500;
  
  //Mix sources function generators
  for(uint8_t i = 0; i < NUM_FUNCGEN; i++)
    mixSources[SRC_FUNCGEN_FIRST + i] = generateWaveform(i, currMillis);
  
  //Channels
  for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
    mixSources[SRC_CH1 + i] = 0;
  
  //Virtuals
  for(uint8_t i = 0; i < NUM_VIRTUAL_CHANNELS; i++)
    mixSources[SRC_VIRTUAL_FIRST + i] = 0;
  
  
  ///------------------ MIXER LOOP ------------------------
  
  for(uint8_t mixIdx = 0; mixIdx < NUM_MIXSLOTS; mixIdx++)
  {
    mixer_params_t *mxr = &Model.Mixer[mixIdx];
    
    //--- CHECK OUTPUT INDEX
    if(mxr->output == SRC_NONE)
      continue;
    
    //--- HANDLE SPECIAL MIXER OPERATION 'HOLD'
    if(isReinitialiseMixer)
      hldOldState[mixIdx] = false;
    if(mxr->operation == MIX_HOLD)
    {
      if(checkSwitchCondition(mxr->swtch))
      {
        //capture the value at the instant of activating the control switch
        if(!hldOldState[mixIdx])
        {
          hldOldState[mixIdx] = true;
          hldOldVal[mixIdx] = mixSources[mxr->output];
        }
        //override the source with the value we captured
        mixSources[mxr->output] =  hldOldVal[mixIdx];
      }
      else
        hldOldState[mixIdx] = false;

      continue;
    }
    
    //--- CHECK INPUT INDEX
    if(mxr->input == SRC_NONE)
      continue;

    //--- ASSIGN
    int16_t operand = mixSources[mxr->input];
    
    //--- DELAY
    //Reset delay variables if the input or model has been changed
    if((mxr->input != dlyPrevInput[mixIdx]) || isReinitialiseMixer) 
    {
      dlyPrevInput[mixIdx] = mxr->input;
      dlyOldVal[mixIdx]  = operand;
      dlyPrevOperand[mixIdx]  = operand;
      dlyStartTime[mixIdx] = currMillis;
    }
    //For the comparisons here, we add some hysteresis because readings from potentiometers
    //or any analog based measuring sensors always exibit jitter due to random electrical noise, 
    //slight imperfections, etc. It's all physics :) If we don't add hysteresis, the delay could lock up
    //and we would never get out of it. 
    const int16_t hystVal = 4; //empirically determined value 
    if(abs(operand - dlyPrevOperand[mixIdx]) > hystVal) 
    {
      dlyStartTime[mixIdx] = currMillis;
      dlyPrevOperand[mixIdx] = operand;
    }
    if(operand > (dlyOldVal[mixIdx] + hystVal))
    {
      if(mxr->delayUp != 0) //only compute when necessary
      {
        if((currMillis - dlyStartTime[mixIdx]) < ((uint32_t)mxr->delayUp * 100))
          operand = dlyOldVal[mixIdx];
        else
          dlyOldVal[mixIdx] = operand;
      }
      else
        dlyOldVal[mixIdx] = operand;
    }
    else if(operand < (dlyOldVal[mixIdx] - hystVal))
    {
      if(mxr->delayDown != 0) //only compute when necessary
      {
        if((currMillis - dlyStartTime[mixIdx]) < ((uint32_t)mxr->delayDown * 100))
          operand = dlyOldVal[mixIdx];
        else
          dlyOldVal[mixIdx] = operand;
      }
      else
        dlyOldVal[mixIdx] = operand;
    }
    
    //--- SLOW
    //We use a multiplier value here to improve precision
    const int32_t _factor = 4096;
    if(isReinitialiseMixer)
      slowCurrVal[mixIdx] = _factor * operand;
    int32_t _op = _factor * operand;
    if(mxr->slowUp != 0 || mxr->slowDown != 0) //only compute when necessary
    {
      _op = applySlow(slowCurrVal[mixIdx], _op, _factor, (uint32_t)mxr->slowUp * 100, (uint32_t)mxr->slowDown * 100);
      slowCurrVal[mixIdx] = _op;
      operand = _op/_factor;
    }
    else //keep the stored value up to date
      slowCurrVal[mixIdx] = _op;
     
    //--- CHECK MIXSWITCH, FLIGHT MODE
    //This is done after processing Delay and Slow so we can still update them 
    //even when the mixer condition is not met to avoid glitches due to stale values.
    if(!checkSwitchCondition(mxr->swtch) || !((mxr->flightMode >> activeFmdIdx) & 0x01))
      continue;
    
    //--- CURVES
    if(mxr->curveType == MIX_CURVE_TYPE_EXPO)
      operand = calcExpo(operand, mxr->curveVal);
    else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
    {
      uint8_t _crvIdx = mxr->curveVal;
      uint8_t _numPts = Model.CustomCurve[_crvIdx].numPoints;
      int16_t _xx[MAX_NUM_POINTS_CUSTOM_CURVE];
      int16_t _yy[MAX_NUM_POINTS_CUSTOM_CURVE];
      for(uint8_t pt = 0; pt < _numPts; pt++)
      {
        _xx[pt] = 5 * Model.CustomCurve[_crvIdx].xVal[pt];
        _yy[pt] = 5 * Model.CustomCurve[_crvIdx].yVal[pt];
      }
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
    
    //--- WEIGHT, OFFSET
    operand = weightAndOffset(operand, mxr->weight, mxr->offset);
    
    //--- DIFFERENTIAL
    if(mxr->curveType == MIX_CURVE_TYPE_DIFF && mxr->curveVal != 0)
      operand = calcDifferential(operand, mxr->curveVal);
    
    //--- TRIM
    //Apply weighted trim after differential to prevent the output being erratic when differential or
    //curves are specified and trim is non-zero
    if(mxr->trimEnabled)
    {
      uint8_t src = mxr->input;
      if(src == SRC_RUD) src = Model.rudSrcRaw;
      if(src == SRC_THR) src = Model.thrSrcRaw;
      if(src == SRC_AIL) src = Model.ailSrcRaw;
      if(src == SRC_ELE) src = Model.eleSrcRaw;

      if(src == SRC_X1)
      {
        if(Model.X1Trim.trimState == TRIM_FLIGHT_MODE)
          operand += ((int16_t) flightModeTrim[0] * mxr->weight) / 100; 
        else if(Model.X1Trim.trimState == TRIM_COMMON)
          operand += ((int16_t) Model.X1Trim.commonTrim * 5 * mxr->weight) / 100;
      }
      else if(src == SRC_Y1)
      {
        if(Model.Y1Trim.trimState == TRIM_FLIGHT_MODE)
          operand += ((int16_t) flightModeTrim[1] * mxr->weight) / 100; 
        else if(Model.Y1Trim.trimState == TRIM_COMMON)
          operand += ((int16_t) Model.Y1Trim.commonTrim * 5 * mxr->weight) / 100;
      }
      else if(src == SRC_X2)
      {
        if(Model.X2Trim.trimState == TRIM_FLIGHT_MODE)
          operand += ((int16_t) flightModeTrim[2] * mxr->weight) / 100; 
        else if(Model.X2Trim.trimState == TRIM_COMMON)
          operand += ((int16_t) Model.X2Trim.commonTrim * 5 * mxr->weight) / 100;
      }
      else if(src == SRC_Y2)
      {
        if(Model.Y2Trim.trimState == TRIM_FLIGHT_MODE)
          operand += ((int16_t) flightModeTrim[3] * mxr->weight) / 100; 
        else if(Model.Y2Trim.trimState == TRIM_COMMON)
          operand += ((int16_t) Model.Y2Trim.commonTrim * 5 * mxr->weight) / 100;
      }
    }
    
    //--- MIX AND UPDATE
    operand = constrain(operand, -500, 500); 
    int32_t output = mixSources[mxr->output];
    switch(mxr->operation)
    {
      case MIX_ADD:
        output += operand;
        break;
        
      case MIX_MULTIPLY:
        output *= operand;
        output /= 500; 
        break;
        
      case MIX_REPLACE:
        output = operand;
        break;
    }
    output = constrain(output, -500, 500); 
    //update for next iteration
    mixSources[mxr->output] = (int16_t) output;
  }
  
  ///------------------ LOGICAL SWITCH EVALUATIONS --------

  static bool     lsDlyStarted[NUM_LOGICAL_SWITCHES];
  static uint32_t lsDlyStartTime[NUM_LOGICAL_SWITCHES];
  static bool     lsDurOldState[NUM_LOGICAL_SWITCHES];
  static uint32_t lsDurEndTime[NUM_LOGICAL_SWITCHES];
  static bool     lsToggleLastState[NUM_LOGICAL_SWITCHES];
  static int32_t  lsDeltaPrevVal[NUM_LOGICAL_SWITCHES];
  
  if(isReinitialiseMixer)  
  {
    for(uint8_t idx = 0; idx < NUM_LOGICAL_SWITCHES; idx++)
    {
      lsDlyStarted[idx] = false;
      lsDurOldState[idx] = false;
      logical_switch_t *ls = &Model.LogicalSwitch[idx]; 
      if(ls->func == LS_FUNC_TOGGLE)
        lsToggleLastState[idx] = checkSwitchCondition(ls->val1);
      if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
      {
        if(ls->val1 < MIXSOURCES_COUNT) //mixsources
          lsDeltaPrevVal[idx] = mixSources[ls->val1];
        else if(ls->val1 < MIXSOURCES_COUNT + NUM_COUNTERS) //counters
          lsDeltaPrevVal[idx] = counterOut[ls->val1 - MIXSOURCES_COUNT];
        else if(ls->val1 < MIXSOURCES_COUNT + NUM_COUNTERS + NUM_TIMERS) //timers
          lsDeltaPrevVal[idx] = timerElapsedTime[ls->val1 - (MIXSOURCES_COUNT + NUM_COUNTERS)];
      }
    }
  }
    
  for(uint8_t idx = 0; idx < NUM_LOGICAL_SWITCHES; idx++)
  {
    logical_switch_t *ls = &Model.LogicalSwitch[idx];
    
    bool result = false;
    switch(ls->func)
    {
      case LS_FUNC_A_GREATER_THAN_X:
      case LS_FUNC_A_LESS_THAN_X:
      case LS_FUNC_A_EQUAL_X:
      case LS_FUNC_A_GREATER_THAN_OR_EQUAL_X:
      case LS_FUNC_A_LESS_THAN_OR_EQUAL_X:
      case LS_FUNC_ABS_A_GREATER_THAN_X:
      case LS_FUNC_ABS_A_LESS_THAN_X:
      case LS_FUNC_ABS_A_EQUAL_X:
      case LS_FUNC_ABS_A_GREATER_THAN_OR_EQUAL_X:
      case LS_FUNC_ABS_A_LESS_THAN_OR_EQUAL_X:
      case LS_FUNC_ABS_DELTA_GREATER_THAN_X:
        {
          int32_t _val1, _val2;
          if(ls->val1 < MIXSOURCES_COUNT) //mixsources
          {
            _val1 = mixSources[ls->val1];
            _val2 = 5 * ls->val2;
          }
          else if(ls->val1 < MIXSOURCES_COUNT + NUM_COUNTERS) //counters
          {
            _val1 = counterOut[ls->val1 - MIXSOURCES_COUNT];
            _val2 = ls->val2;
          }
          else if(ls->val1 < MIXSOURCES_COUNT + NUM_COUNTERS + NUM_TIMERS) //timers
          {
            uint8_t tmrIdx = ls->val1 - (MIXSOURCES_COUNT + NUM_COUNTERS);
            if(Model.Timer[tmrIdx].initialMinutes == 0) //a count up timer
            {
              _val1 = timerElapsedTime[tmrIdx];
            }
            else //count down timer
            {
              _val1 = (int32_t)Model.Timer[tmrIdx].initialMinutes * 60000;
              _val1 -= timerElapsedTime[tmrIdx];
            }
            _val2 = (int32_t)ls->val2 * 1000;
          }
          else //telemetry
          {
            uint8_t tlmIdx = ls->val1 - (MIXSOURCES_COUNT + NUM_COUNTERS + NUM_TIMERS);
            if(telemetryReceivedValue[tlmIdx] == TELEMETRY_NO_DATA)
              break; 
            else
            {
              _val1 = ((int32_t) telemetryReceivedValue[tlmIdx] * Model.Telemetry[tlmIdx].multiplier) / 100;
              _val1 += Model.Telemetry[tlmIdx].offset;
              _val2 = ls->val2;
            }
          }
          
          if(ls->func == LS_FUNC_A_GREATER_THAN_X)                result = _val1 > _val2;
          else if(ls->func == LS_FUNC_A_LESS_THAN_X)              result = _val1 < _val2;
          else if(ls->func == LS_FUNC_A_EQUAL_X)                  result = _val1 == _val2;
          else if(ls->func == LS_FUNC_A_GREATER_THAN_OR_EQUAL_X)  result = _val1 >= _val2;
          else if(ls->func == LS_FUNC_A_LESS_THAN_OR_EQUAL_X)     result = _val1 <= _val2;
          else if(ls->func == LS_FUNC_ABS_A_GREATER_THAN_X)       result = abs(_val1) > _val2;
          else if(ls->func == LS_FUNC_ABS_A_LESS_THAN_X)          result = abs(_val1) < _val2;
          else if(ls->func == LS_FUNC_ABS_A_EQUAL_X)              result = abs(_val1) == _val2;
          else if(ls->func == LS_FUNC_ABS_A_GREATER_THAN_OR_EQUAL_X) result = abs(_val1) >= _val2;
          else if(ls->func == LS_FUNC_ABS_A_LESS_THAN_OR_EQUAL_X) result = abs(_val1) <= _val2;
          else if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
          {
            int32_t difference = _val1 - lsDeltaPrevVal[idx];
            if(abs(difference) > _val2)
            {
              if(ls->val3 == 0) //only in positive direction
              {
                if(difference > _val2)
                  result = true;
                if(result || difference < 0)
                  lsDeltaPrevVal[idx] = _val1;
              }
              else if(ls->val3 == 1) //only in negative direction
              {
                if(difference < -_val2)
                  result = true;
                if(result || difference > 0)
                  lsDeltaPrevVal[idx] = _val1;
              }
              else //both directions
              {
                result = true;
                lsDeltaPrevVal[idx] = _val1;
              }
            }
          }
        }
        break;

      case LS_FUNC_A_GREATER_THAN_B:
        result = mixSources[ls->val1] > mixSources[ls->val2];
        break;
      
      case LS_FUNC_A_LESS_THAN_B:
        result = mixSources[ls->val1] < mixSources[ls->val2];
        break;
        
      case LS_FUNC_A_EQUAL_B:
        result = mixSources[ls->val1] == mixSources[ls->val2];
        break;
        
      case LS_FUNC_A_GREATER_THAN_OR_EQUAL_B:
        result = mixSources[ls->val1] >= mixSources[ls->val2];
        break;
        
      case LS_FUNC_A_LESS_THAN_OR_EQUAL_B:
        result = mixSources[ls->val1] <= mixSources[ls->val2];
        break;
        
      case LS_FUNC_AND:
        result = checkSwitchCondition(ls->val1) && checkSwitchCondition(ls->val2);
        break;
         
      case LS_FUNC_OR:
        result = checkSwitchCondition(ls->val1) || checkSwitchCondition(ls->val2);
        break;
        
      case LS_FUNC_XOR:
        result = checkSwitchCondition(ls->val1) != checkSwitchCondition(ls->val2);
        break;
        
      case LS_FUNC_LATCH:
        {
          bool _val1 = checkSwitchCondition(ls->val1);
          bool _val2 = checkSwitchCondition(ls->val2);
          if(_val1 && !_val2) result = true;       //set
          else if(!_val1 && _val2) result = false; //reset
          else result = logicalSwitchState[idx];   //no change
        }
        break;
        
      case LS_FUNC_TOGGLE:
        {
          //get the previous result
          result = logicalSwitchState[idx]; 
          //get the state of the switch, the source of our clock
          bool state = checkSwitchCondition(ls->val1);
          //toggle on the rising edge
          if(ls->val2 == 0) 
          {
            if(state && !lsToggleLastState[idx]) //went from low to high
            {
              lsToggleLastState[idx] = true;
              result = !logicalSwitchState[idx];
            }
            else if(!state)
              lsToggleLastState[idx] = false;
          }
          //toggle on falling edge
          if(ls->val2 == 1) 
          {
            if(!state && lsToggleLastState[idx]) //went from high to low
            {
              lsToggleLastState[idx] = false;
              result = !logicalSwitchState[idx];
            }
            else if(state)
              lsToggleLastState[idx] = true;
          }
          //dual edge triggering i.e. both rising and falling edges
          if(ls->val2 == 2)
          {
            if(state != lsToggleLastState[idx])
            {
              lsToggleLastState[idx] = state;
              result = !logicalSwitchState[idx];
            }
          }
          
          //clear. This overrides result to false.
          if(ls->val3 != CTRL_SW_NONE && checkSwitchCondition(ls->val3))
            result = false;
        }
        break;
        
      case LS_FUNC_PULSE:
        {
          uint32_t highTime   = (uint32_t)ls->val1 * 100;
          uint32_t period     = (uint32_t)ls->val2 * 100;
          uint32_t pulseDelay = (uint32_t)ls->val3 * 100;
          uint32_t timeInstance;
          //As we are dealing with unsigned subtraction, we need to prevent strange results here
          //because at start up, the value of currMillis is less than that of pulseDelay.
          if(currMillis >= pulseDelay)
            timeInstance = (currMillis - pulseDelay) % period;
          else
          {
            timeInstance = period - ((pulseDelay - currMillis) % period); 
            timeInstance %= period;
          }
          result = timeInstance < highTime;
        }
        break;
    }
    
    //delay
    //here we delay activation of the logical switch by overriding for the specified time
    if(ls->val3 > 0
       && ls->func != LS_FUNC_PULSE 
       && ls->func != LS_FUNC_TOGGLE 
       && ls->func != LS_FUNC_ABS_DELTA_GREATER_THAN_X)
    {
      if(result && !logicalSwitchState[idx]) //went from false to true
      {
        if(!lsDlyStarted[idx])
        {
          lsDlyStartTime[idx] = currMillis;
          lsDlyStarted[idx] = true;
        }
      }
      if(!result) //reset flag
        lsDlyStarted[idx] = false;
        
      if(currMillis - lsDlyStartTime[idx] < ((uint32_t)ls->val3 * 100)) //override result
        result = false;
    }
    
    //duration
    if(ls->val4 > 0 
       && ls->func != LS_FUNC_PULSE 
       && ls->func != LS_FUNC_LATCH 
       && ls->func != LS_FUNC_TOGGLE)
    {
      if(result && !lsDurOldState[idx]) //went from inactive to active
      {
        lsDurOldState[idx] = true;
        lsDurEndTime[idx] = currMillis + ((uint32_t)ls->val4 * 100); 
      }
      if(currMillis >= lsDurEndTime[idx]) //duration has expired
      {
        if(!result) //only reset old state when result goes false
          lsDurOldState[idx] = false;
        result = false;
      }
      else if(lsDurOldState[idx]) //hold even if the input suddenly became false before duration expired
        result = true;
    }

    //store the result
    logicalSwitchState[idx] = result;
  }

  ///----------------- COUNTER EVALUATIONS ---------------
  
  static bool counterState[NUM_COUNTERS];
  static bool counterToggleLastState[NUM_COUNTERS];

  for(uint8_t idx = 0; idx < NUM_COUNTERS; idx++)
  {
    counter_params_t *counter = &Model.Counter[idx];
    
    if(isReinitialiseMixer)
    {
      counterState[idx] = false;
      counterToggleLastState[idx] = checkSwitchCondition(counter->clock);
      // counterOut[idx] = 0;
      continue;
    }
    
    //get the previous result
    bool result = counterState[idx]; 
    //get the state of the switch, the source of our clock
    bool state = (counter->clock != CTRL_SW_NONE) ? checkSwitchCondition(counter->clock) : false;
    //toggle on the rising edge
    if(counter->edge == 0) 
    {
      if(state && !counterToggleLastState[idx]) //went from low to high
      {
        counterToggleLastState[idx] = true;
        result = !counterState[idx];
        if(counter->direction == 0) 
          counterOut[idx]++;
        else 
          counterOut[idx]--;
      }
      else if(!state)
        counterToggleLastState[idx] = false;
    }
    //toggle on falling edge
    if(counter->edge == 1) 
    {
      if(!state && counterToggleLastState[idx]) //went from high to low
      {
        counterToggleLastState[idx] = false;
        result = !counterState[idx];
        if(counter->direction == 0) 
          counterOut[idx]++;
        else 
          counterOut[idx]--;
      }
      else if(state)
        counterToggleLastState[idx] = true;
    }
    //dual edge triggering i.e. both rising and falling edges
    if(counter->edge == 2)
    {
      if(state != counterToggleLastState[idx])
      {
        counterToggleLastState[idx] = state;
        result = !counterState[idx];
        if(counter->direction == 0) 
          counterOut[idx]++;
        else 
          counterOut[idx]--;
      }
    }
    
    //check against modulus
    if(counterOut[idx] >= counter->modulus) 
      counterOut[idx] = 0;
    if(counterOut[idx] < 0)                 
      counterOut[idx] = counter->modulus - 1;
    
    //clear. This overrides result to false.
    if(counter->clear != CTRL_SW_NONE && checkSwitchCondition(counter->clear))
    {
      result = false;
      counterOut[idx] = 0;
    }
    
    //store the result
    counterState[idx] = result;
    
    //store the register value
    if(counter->isPersistent)
      counter->persistVal = counterOut[idx];
    else
      counter->persistVal = 0;
  }
  
  ///----------------- OUTPUT TO CHANNELS ----------------
  
  for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
  {
    channelOut[i] = mixSources[SRC_CH1 + i];
    //reverse
    if(Model.Channel[i].reverse) 
      channelOut[i] = 0 - channelOut[i]; 
    //subtrim
    channelOut[i] += 5 * Model.Channel[i].subtrim; 
    //override
    if(Model.Channel[i].overrideSwitch != CTRL_SW_NONE && checkSwitchCondition(Model.Channel[i].overrideSwitch))
      channelOut[i] = 5 * Model.Channel[i].overrideVal;
    //endpoints
    int16_t endptL = 5 * Model.Channel[i].endpointL;
    int16_t endptR = 5 * Model.Channel[i].endpointR;
    channelOut[i] = constrain(channelOut[i], endptL, endptR); 
  }
  
  /// ---------------- RESET SOME VARIABLES --------------
  isReinitialiseMixer = false;
}

//==================================================================================================

void moveMix(uint8_t newPos, uint8_t oldPos)
{
  if(newPos >= NUM_MIXSLOTS || oldPos >= NUM_MIXSLOTS)
    return;
  
  // store temporarily the old position's data
  mixer_params_t Temp_Mixer    = Model.Mixer[oldPos];
  int16_t  tempDlyOldVal       = dlyOldVal[oldPos];
  int16_t  tempDlyPrevOperand  = dlyPrevOperand[oldPos];
  uint8_t  tempDlyPrevInputIdx = dlyPrevInput[oldPos];
  uint32_t tempDlyStartTime    = dlyStartTime[oldPos];
  int32_t  tempSlowCurrVal     = slowCurrVal[oldPos];
  int16_t  tempHldOldVal       = hldOldVal[oldPos];
  uint8_t  tempHldOldState     = hldOldState[oldPos];

  //shift elements of the arrays
  uint8_t thisPostn = oldPos;
  if(newPos < oldPos)
  {
    while(thisPostn > newPos)
    {
      Model.Mixer[thisPostn]     = Model.Mixer[thisPostn-1];
      dlyOldVal[thisPostn]       = dlyOldVal[thisPostn-1];
      dlyPrevOperand[thisPostn]  = dlyPrevOperand[thisPostn-1];
      dlyPrevInput[thisPostn]    = dlyPrevInput[thisPostn-1];
      dlyStartTime[thisPostn]    = dlyStartTime[thisPostn-1];
      slowCurrVal[thisPostn]     = slowCurrVal[thisPostn-1];
      hldOldVal[thisPostn]       = hldOldVal[thisPostn-1];
      hldOldState[thisPostn]     = hldOldState[thisPostn-1];
      
      thisPostn--;
    }
  }
  else if(newPos > oldPos) 
  {
    while(thisPostn < newPos)
    {
      Model.Mixer[thisPostn]     = Model.Mixer[thisPostn+1];
      dlyOldVal[thisPostn]       = dlyOldVal[thisPostn+1];
      dlyPrevOperand[thisPostn]  = dlyPrevOperand[thisPostn+1];
      dlyPrevInput[thisPostn]    = dlyPrevInput[thisPostn+1];
      dlyStartTime[thisPostn]    = dlyStartTime[thisPostn+1];
      slowCurrVal[thisPostn]     = slowCurrVal[thisPostn+1];
      hldOldVal[thisPostn]       = hldOldVal[thisPostn+1];
      hldOldState[thisPostn]     = hldOldState[thisPostn+1];
      
      thisPostn++;
    }
  }
  
  //copy from temporary into new position
  Model.Mixer[newPos]     = Temp_Mixer;
  dlyOldVal[newPos]       = tempDlyOldVal;
  dlyPrevOperand[newPos]  = tempDlyPrevOperand;
  dlyPrevInput[newPos]    = tempDlyPrevInputIdx;
  dlyStartTime[newPos]    = tempDlyStartTime;
  slowCurrVal[newPos]     = tempSlowCurrVal;
  hldOldVal[newPos]       = tempHldOldVal;
  hldOldState[newPos]     = tempHldOldState;
}

//==================================================================================================

void swapMix(uint8_t posA, uint8_t posB)
{
  if(posA >= NUM_MIXSLOTS || posB >= NUM_MIXSLOTS)
    return;
  
  //store posA to temp
  mixer_params_t Temp_Mixer    = Model.Mixer[posA];
  int16_t  tempDlyOldVal       = dlyOldVal[posA];
  int16_t  tempDlyPrevOperand  = dlyPrevOperand[posA];
  uint8_t  tempDlyPrevInputIdx = dlyPrevInput[posA];
  uint32_t tempDlyStartTime    = dlyStartTime[posA];
  int32_t  tempSlowCurrVal     = slowCurrVal[posA];
  int16_t  tempHldOldVal       = hldOldVal[posA];
  uint8_t  tempHldOldState     = hldOldState[posA];
  
  //copy posB to posA
  Model.Mixer[posA]     = Model.Mixer[posB];
  dlyOldVal[posA]       = dlyOldVal[posB];
  dlyPrevOperand[posA]  = dlyPrevOperand[posB];
  dlyPrevInput[posA]    = dlyPrevInput[posB];
  dlyStartTime[posA]    = dlyStartTime[posB];
  slowCurrVal[posA]     = slowCurrVal[posB];
  hldOldVal[posA]       = hldOldVal[posB];
  hldOldState[posA]     = hldOldState[posB];
  
  //copy from temp to posB
  Model.Mixer[posB]     = Temp_Mixer;
  dlyOldVal[posB]       = tempDlyOldVal;
  dlyPrevOperand[posB]  = tempDlyPrevOperand;
  dlyPrevInput[posB]    = tempDlyPrevInputIdx;
  dlyStartTime[posB]    = tempDlyStartTime;
  slowCurrVal[posB]     = tempSlowCurrVal;
  hldOldVal[posB]       = tempHldOldVal;
  hldOldState[posB]     = tempHldOldState;
}

//==================================================================================================

int16_t calcRateExpo(int16_t input, int16_t rate, int16_t expo)
{
  /* This function is for applying rate and cubic 'expo' to aileron, elevator and rudder.
     Ranges: input -500 to 500, rate 0 to 100, expo -100 to 100. 0 is linear
  */
  return ((int32_t)rate * calcExpo(input, expo)) / 100;
}

//==================================================================================================

int16_t calcExpo(int16_t input, int16_t expo)
{
  //Range: expo -100 to 100. 0 is linear, input -500 to 500
  
  /*
  The original equation is 
  y = e^(k*ln(x)) which simplifies to y = x^k
  However this is quite hard to compute on a simple microcontroller, therefore we use a cubic equation 
  that gives comparable results for the range we are interested in. 
  y = k*x^3 + (1-k)*x   where k is 'expo factor. k range -1 to 1, x range -1 to 1
  
  For our implementation, we only use the range 0 < k < 1 and simply mirror the result if k is negative.
  We also work with only positive input in the calculation and simply negate the result for negative input.
  
  We need to scale up the above equation to avoid floating point math. After calculation, we scale back the result. 
  Thus the modified equation taking into account the range of our parameters is as follows. 
  y/500 = ((k/100)*(x/500)^3) + ((1-(k/100))*(x/500))
  
  Simplifying and rearranging to ensure no overflow for our range of values,
  y = (((k*x*x + 250000*(100-k))/250000)*x)/100
  */
  
  if(expo == 0)
    return input;

  int32_t x = abs(input);
  int32_t k = abs(expo);
  if(expo < 0)
    x -= 500;
  
  int32_t y = k*x*x + 250000*(100-k);
  y /= 250000;
  y *= x;
  y /= 100;
  
  if(expo < 0)
    y += 500;
  if(input < 0)
    y = -y;
  
  return (int16_t) y;
}

//==================================================================================================

int16_t calcDifferential(int16_t input, int16_t diff)
{
  int32_t output = input;
  if(diff > 0 && input < 0)
  {
    output *= (100 - diff);
    output /= 100;
  }
  else if(diff < 0 && input > 0)
  {
    output *= (100 + diff);
    output /= 100;
  }
 
  return (int16_t) output;
}

//==================================================================================================

int32_t applySlow(int32_t currVal, int32_t targetVal, int32_t multiplier, uint32_t riseTime, uint32_t fallTime)
{
  if(currVal < targetVal && riseTime > 0)
  {
    int32_t step = (multiplier * 1000L * fixedLoopTime) / riseTime;
    currVal += step;
    if(currVal > targetVal)
      currVal = targetVal;
  }
  else if(currVal > targetVal && fallTime > 0) 
  {
    int32_t step = (multiplier * 1000L * fixedLoopTime) / fallTime;
    currVal -= step;
    if(currVal < targetVal)
      currVal = targetVal;
  }
  else
    currVal = targetVal;
  
  return currVal;
}

//==================================================================================================

int16_t weightAndOffset(int16_t input, int16_t weight, int16_t offset)
{ 
  return (((int32_t)input * weight) / 100) + (offset * 5);
}

//==================================================================================================

bool checkSwitchCondition(uint8_t sw)
{
  bool result = false;
  
  if(sw == CTRL_SW_NONE) //always active
  {
    result = true;
  }
  else if(sw >= CTRL_SW_PHYSICAL_FIRST && sw <= CTRL_SW_PHYSICAL_LAST)
  {
    uint8_t idx = (sw - CTRL_SW_PHYSICAL_FIRST) / 6;
    uint8_t condition = (sw - CTRL_SW_PHYSICAL_FIRST) % 6;
    switch(condition) //up, mid, down, !up, !mid, !down
    {
      case 0: result = swState[idx] == SWUPPERPOS; break;
      case 1: result = swState[idx] == SWMIDPOS;   break; 
      case 2: result = swState[idx] == SWLOWERPOS; break;
      case 3: result = swState[idx] != SWUPPERPOS; break;
      case 4: result = swState[idx] != SWMIDPOS;   break;
      case 5: result = swState[idx] != SWLOWERPOS; break;
    }
  }
  else if(sw >= CTRL_SW_LOGICAL_FIRST && sw <= CTRL_SW_LOGICAL_LAST_INVERT)
  {
    bool invertResult = false;
    uint8_t idx = sw - CTRL_SW_LOGICAL_FIRST;
    if(sw >= CTRL_SW_LOGICAL_FIRST_INVERT)
    {
      idx = sw - CTRL_SW_LOGICAL_FIRST_INVERT;
      invertResult = true;
    }
    //check within array
    result = logicalSwitchState[idx];
    
    if(invertResult)
      result = !result;
  }
  else //flight modes as switches
  {
    bool invertResult = false;
    uint8_t fmdIdx = sw - CTRL_SW_COUNT;
    if(fmdIdx >= NUM_FLIGHT_MODES)
    {      
      fmdIdx -= NUM_FLIGHT_MODES;
      invertResult = true;
    }
    //compare with active flight mode
    if(fmdIdx == activeFmdIdx) 
      result = true;
    if(invertResult)
      result = !result;
  }
  
  return result;
}

//==================================================================================================

int8_t adjustTrim(int8_t val, int8_t lowerLimit, int8_t upperLimit, uint8_t incButton, uint8_t decButton)
{
  uint8_t _heldBtn = 0;
  uint8_t _holdDelay = 200;
  if(millis() - buttonStartTime > 1000) //speed up
    _holdDelay = 100;
  if((thisLoopNum - heldButtonEntryLoopNum) % (_holdDelay / fixedLoopTime) == 0) 
    _heldBtn = heldButton;
  
  if((pressedButton == decButton || _heldBtn == decButton) && val > lowerLimit)
  {    
    val--;
    if(Sys.soundTrims)
      audioToPlay = AUDIO_TRIM_MOVED;
  }
  else if((pressedButton == incButton || _heldBtn == incButton) && val < upperLimit)
  {
    val++;
    if(Sys.soundTrims)
      audioToPlay = AUDIO_TRIM_MOVED;
  }
  
  return val;
}

//==================================================================================================

bool isSyncWaveform[NUM_FUNCGEN];

void syncWaveform(uint8_t idx)
{
  if(idx >= NUM_FUNCGEN)
    return;
  isSyncWaveform[idx] = true;
}

int16_t generateWaveform(uint8_t idx, int32_t _currTime)
{
  if(idx >= NUM_FUNCGEN)
    return 0;
  
  int16_t result = 0;
  
  funcgen_t *fgen = &Model.Funcgen[idx];
  
  int32_t period;
  if(fgen->waveform == FUNCGEN_WAVEFORM_PULSE)
  {
    period = (int32_t)fgen->period * 100;
  }
  else
  {
    if(fgen->periodMode == FUNCGEN_PERIODMODE_FIXED)
      period = (int32_t)fgen->period1 * 100;
    else //variable period
    {
      int32_t m = fgen->reverseModulator ? 0 - mixSources[fgen->modulatorSrc] : mixSources[fgen->modulatorSrc];
      period = (((m + 500) * (fgen->period2 - fgen->period1)) / 10) + ((int32_t)fgen->period1 * 100);
    }
  }
  
  static int32_t oldPeriod[NUM_FUNCGEN];
  static int32_t timeOffset[NUM_FUNCGEN];
  
  int32_t timeInstance = 0; 
  
  if(isSyncWaveform[idx])
  {
    isSyncWaveform[idx] = false;
    timeOffset[idx] = 0;
  }

  bool phaseCompensate = false;
  
  if(fgen->waveform == FUNCGEN_WAVEFORM_PULSE)
  {
    if(fgen->phaseMode == FUNCGEN_PHASEMODE_AUTO)
      phaseCompensate = true;
  }
  else
  {
    if((fgen->periodMode == FUNCGEN_PERIODMODE_FIXED && fgen->phaseMode == FUNCGEN_PHASEMODE_AUTO) 
      || fgen->periodMode == FUNCGEN_PERIODMODE_VARIABLE)
    {
      phaseCompensate = true;
    }
  }

  if(phaseCompensate)
  {
    // Smoothly transition to new period when the period is being adjusted.
    // We add some time offset so that we maintain the next ratio of timeInstance/period upon change.
    if(period != oldPeriod[idx])
    {
      if(oldPeriod[idx] == 0) //at initial run
      {
        timeOffset[idx] = 0;
        oldPeriod[idx] = period; 
      }
      else
      {
        int32_t nextTimeInstance = (_currTime + timeOffset[idx]) % oldPeriod[idx];
        timeOffset[idx] = ((period * nextTimeInstance)/oldPeriod[idx]) - (_currTime % period);
        oldPeriod[idx] = period; 
      }
    }
    timeInstance = (_currTime + timeOffset[idx]) % period;
  }
  else //fixed phase
  {
    timeOffset[idx] = ((int32_t) fgen->phase * period) / 360;
    if(_currTime >= timeOffset[idx])
      timeInstance = (_currTime - timeOffset[idx]) % period;
    else
    {
      timeInstance = period - ((timeOffset[idx] - _currTime) % period); 
      timeInstance %= period;
    }
  }

  switch(fgen->waveform)
  {
    case FUNCGEN_WAVEFORM_SINE:
      {
        // Table for generating stepped sine waveform
        static const int8_t sineTable[64] PROGMEM = {
          0, 10, 20, 29, 38, 47, 56, 63, 71, 77, 83, 88, 92, 96, 98, 100, 
          100, 100, 98, 96, 92, 88, 83, 77, 71, 63, 56, 47, 38, 29, 20, 10, 
          0, -10, -20, -29, -38, -47, -56, -63, -71, -77, -83, -88, -92, -96, -98, -100, 
          -100, -100, -98, -96, -92, -88, -83, -77, -71, -63, -56, -47, -38, -29, -20, -10
        };
        
        //--fixed point arithmetic with a fractional part
        // the fractional part is used for linear interpolation
        const int16_t multiplier = 32;
        int16_t index = (timeInstance * sizeof(sineTable) * multiplier)/period; 
        uint8_t indexLUT = index / multiplier;
        int16_t valL = (int16_t)5 * (int8_t)pgm_read_byte(&sineTable[indexLUT]);
        indexLUT++; 
        if(indexLUT >= sizeof(sineTable)) 
          indexLUT = 0;
        int16_t valR = (int16_t)5 * (int8_t)pgm_read_byte(&sineTable[indexLUT]);
        result = valL + (((valR - valL)*(index % multiplier))/multiplier);
      }
      break;
    
    case FUNCGEN_WAVEFORM_SAWTOOTH:
      {
        if(timeInstance < (period/2))
          result = map(timeInstance, 0, period/2, 0, 500);
        else
          result = map(timeInstance, period/2, period, -500, 0);
      }
      break;
    
    case FUNCGEN_WAVEFORM_TRIANGLE:
      {
        if(timeInstance < (period/4))
          result = map(timeInstance, 0, period/4, 0, 500);
        else if(timeInstance < ((3 * period)/4))
          result = map(timeInstance, period/4, (3 * period)/4, 500, -500);
        else
          result = map(timeInstance, (3 * period)/4, period, -500, 0);
      }
      break;
    
    case FUNCGEN_WAVEFORM_SQUARE:
      {
        result = timeInstance < (period/2) ? 500 : -500;
      }
      break;

    case FUNCGEN_WAVEFORM_PULSE:
      {
        if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_FIXED)
          result = (timeInstance < ((int32_t)fgen->width * 100)) ? 500 : -500;
        else if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_VARIABLE)
        {
          //duty cycle is only updated at the end of the PWM cycle to prevent glitches.
          static int32_t prevTimeInstance[NUM_FUNCGEN];
          static int16_t modulatorValue[NUM_FUNCGEN];
          if(timeInstance < prevTimeInstance[idx])
          {
            modulatorValue[idx] = mixSources[fgen->modulatorSrc];
            if(fgen->reverseModulator)
              modulatorValue[idx] = 0 - modulatorValue[idx];
          }
          prevTimeInstance[idx] = timeInstance;
          int32_t highTime = ((modulatorValue[idx] + 500) * period) / 1000;
          result = (timeInstance < highTime) ? 500 : -500;
        }
      }
      break;

    case FUNCGEN_WAVEFORM_RANDOM:
      {
        static bool isExpired[NUM_FUNCGEN];
        static int16_t lastResult[NUM_FUNCGEN];
        if(timeInstance < (period/2) && isExpired[idx])
        {
          isExpired[idx] = false;
          lastResult[idx] = random(-500, 500);
        }
        if(timeInstance >= (period/2))
          isExpired[idx] = true;
        
        result = lastResult[idx];
      }
      break;
  }
  
  result = constrain(result, -500, 500);
  return result;
}
