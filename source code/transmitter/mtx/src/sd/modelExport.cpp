#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#include "../../config.h"
#include "../common.h"
#include "../stringDefs.h"
#include "modelExport.h"

//----------------------------------- helpers ------------------------------------------------------

void printIndention_helper(File& file, uint8_t level)
{
  while(level)
  {
    file.print(F("  "));
    level--;
  }
}

void writeKey_helper(File& file, uint8_t level, const char* keyStr_P)
{
  printIndention_helper(file, level);
  char tempBuff[MAX_STR_SIZE];
  strlcpy_P(tempBuff, keyStr_P, sizeof(tempBuff));
  file.print(tempBuff);
  file.print(F(": "));
}

void writeKeyValue_Char(File& file, uint8_t level, const char* keyStr_P, const char* valStr)
{
  writeKey_helper(file, level, keyStr_P);
  if(valStr != NULL)
    file.print(valStr);
  file.println();
}

void writeKeyValue_S32(File& file, uint8_t level, const char* keyStr_P, int32_t value)
{
  writeKey_helper(file, level, keyStr_P);
  file.print(value);
  file.println();
}

void writeKeyValue_U32(File& file, uint8_t level, const char* keyStr_P, uint32_t value)
{
  writeKey_helper(file, level, keyStr_P);
  file.print(value);
  file.println();
}

void writeKeyValue_bool(File& file, uint8_t level, const char* keyStr_P, bool value)
{
  writeKey_helper(file, level, keyStr_P);
  if(value)
    file.print(F("true"));
  else 
    file.print(F("false"));
  file.println();
}

void writeKeyValue_TimeSeconds(File& file, uint8_t level, const char* keyStr_P, int32_t deciseconds)
{
  writeKey_helper(file, level, keyStr_P);
  if(deciseconds < 0)
  {
    file.print(F("-"));
    deciseconds = -deciseconds;
  }
  file.print(deciseconds / 10);
  file.print(F("."));
  file.print(deciseconds % 10);
  file.print(F("s"));
  file.println();
}

void writeKeyValue_TrimVal(File& file, uint8_t level, const char* keyStr_P, int32_t trimVal)
{
  writeKey_helper(file, level, keyStr_P);
  if(trimVal < 0)
  {
    file.print(F("-"));
    trimVal = -trimVal;
  }
  file.print(trimVal / 10);
  file.print(F("."));
  file.print(trimVal % 10);
  file.println();
}

void writeArray_U8(File& file, uint8_t level, const char* keyStr_P, uint8_t values[], uint8_t numValues)
{
  writeKey_helper(file, level, keyStr_P);
  file.println();
  for(uint8_t i = 0; i < numValues; i++)
  {
    printIndention_helper(file, level + 1);
    file.print(i);
    file.print(F(": "));
    file.print(values[i]);
    file.println();
  }
}

void writeArray_S8(File& file, uint8_t level, const char* keyStr_P, int8_t values[], uint8_t numValues)
{
  writeKey_helper(file, level, keyStr_P);
  file.println();
  for(uint8_t i = 0; i < numValues; i++)
  {
    printIndention_helper(file, level + 1);
    file.print(i);
    file.print(F(": "));
    file.print(values[i]);
    file.println();
  }
}

//==================================================================================================

void exportModelData(File& file)
{
  char tempBuff[MAX_STR_SIZE];
  memset(tempBuff, 0, sizeof(tempBuff));
  
  file.println(F("##### Model configuration file #####"));
  file.println(F("##### Exported from version " _FIRMWARE_VERSION " #####"));

  writeKeyValue_Char(file, 0, key_ModelName, Model.name);
  writeKeyValue_Char(file, 0, key_ModelType, findStringInIdStr(enum_ModelType, Model.type));

  writeKeyValue_bool(file, 0, key_SecondaryRcvrEnabled, Model.secondaryRcvrEnabled);
  
  if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
  {
    getSrcName_Clean(tempBuff, Model.rudSrcRaw, sizeof(tempBuff));
    writeKeyValue_Char(file, 0, (Model.type == MODEL_TYPE_AIRPLANE) ? key_RudSrc : key_YawSrc, tempBuff);

    getSrcName_Clean(tempBuff, Model.ailSrcRaw, sizeof(tempBuff));
    writeKeyValue_Char(file, 0, (Model.type == MODEL_TYPE_AIRPLANE) ? key_AilSrc : key_RollSrc, tempBuff);
    
    getSrcName_Clean(tempBuff, Model.eleSrcRaw, sizeof(tempBuff));
    writeKeyValue_Char(file, 0, (Model.type == MODEL_TYPE_AIRPLANE) ? key_EleSrc : key_PitchSrc, tempBuff);

    getSrcName_Clean(tempBuff, Model.thrSrcRaw, sizeof(tempBuff));
    writeKeyValue_Char(file, 0, key_ThrSrc, tempBuff);
  }

  file.println(F("# ------ Warnings ------"));
  if((Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER))
    writeKeyValue_bool(file, 0, key_CheckThrottle, Model.checkThrottle);
  writeKeyValue_Char(file, 0, key_SwitchWarn, NULL);
  for(uint8_t idx = 0; idx < NUM_PHYSICAL_SWITCHES; idx++)
  {
    printIndention_helper(file, 1);
    getSrcName_Clean(tempBuff, SRC_SW_PHYSICAL_FIRST + idx, sizeof(tempBuff));
    file.print(tempBuff);
    file.print(F(": "));
    file.print(findStringInIdStr(enum_SwitchWarn, Model.switchWarn[idx]));
    file.println();
  }
  
  file.println(F("# ------ Telemetry ------"));
  
  for(uint8_t idx = 0; idx < NUM_CUSTOM_TELEMETRY; idx++)
  {
    telemetry_params_t *tlm = &Model.Telemetry[idx];
    if(idx > 0 && isEmptyStr(tlm->name, sizeof(tlm->name)))
      continue;

    writeKeyValue_Char(file, 0, key_Telemetry, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);

    writeKeyValue_Char(file, 1, key_Type, findStringInIdStr(enum_TelemetryType, tlm->type));
    writeKeyValue_Char(file, 1, key_Name, tlm->name);
    
    if(tlm->type == TELEMETRY_TYPE_GNSS)
      continue;

    writeKeyValue_Char(file, 1, key_UnitsName, tlm->unitsName);
    
    writeKey_helper(file, 1, key_Identifier);
    // file.print(tlm->identifier, HEX);
    file.print(tlm->identifier);
    file.println();
    
    writeKeyValue_S32(file, 1, key_Factor10, tlm->factor10);
    writeKeyValue_S32(file, 1, key_Multiplier, tlm->multiplier);
    writeKeyValue_S32(file, 1, key_Offset, tlm->offset);
    writeKeyValue_Char(file, 1, key_AlarmCondition, findStringInIdStr(enum_TelemetryAlarmCondition, tlm->alarmCondition));
    writeKeyValue_S32(file, 1, key_AlarmThreshold, tlm->alarmThreshold);
    writeKeyValue_bool(file, 1, key_ShowOnHome, tlm->showOnHome);
    writeKeyValue_bool(file, 1, key_RecordMaximum, tlm->recordMaximum);
    writeKeyValue_bool(file, 1, key_RecordMinimum, tlm->recordMinimum);
  }

  file.println(F("# ------ Timers ------"));
  
  for(uint8_t idx = 0; idx < NUM_TIMERS; idx++)
  {
    timer_params_t *tmr = &Model.Timer[idx];
    
    writeKeyValue_Char(file, 0, key_Timer, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);
    writeKeyValue_Char(file, 1, key_Name, tmr->name);
    
    getControlSwitchName_Clean(tempBuff, tmr->swtch, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Switch, tempBuff);
    
    getControlSwitchName_Clean(tempBuff, tmr->resetSwitch, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_ResetSwitch, tempBuff);
    
    writeKeyValue_S32(file, 1, key_InitialSeconds, tmr->initialSeconds);
    writeKeyValue_bool(file, 1, key_IsPersistent, tmr->isPersistent);
    writeKeyValue_U32(file, 1, key_PersistVal, tmr->persistVal);
  }
  
  file.println(F("# ------ Trims ------"));
  
  writeKeyValue_Char(file, 0, key_X1Trim, NULL);
  writeKeyValue_Char(file, 1, key_TrimState, findStringInIdStr(enum_TrimState, Model.X1Trim.trimState));
  writeKeyValue_TrimVal(file, 1, key_CommonTrim, Model.X1Trim.commonTrim);
  
  writeKeyValue_Char(file, 0, key_Y1Trim, NULL);
  writeKeyValue_Char(file, 1, key_TrimState, findStringInIdStr(enum_TrimState, Model.Y1Trim.trimState));
  writeKeyValue_TrimVal(file, 1, key_CommonTrim, Model.Y1Trim.commonTrim);
  
  writeKeyValue_Char(file, 0, key_X2Trim, NULL);
  writeKeyValue_Char(file, 1, key_TrimState, findStringInIdStr(enum_TrimState, Model.X2Trim.trimState));
  writeKeyValue_TrimVal(file, 1, key_CommonTrim, Model.X2Trim.commonTrim);
  
  writeKeyValue_Char(file, 0, key_Y2Trim, NULL);
  writeKeyValue_Char(file, 1, key_TrimState, findStringInIdStr(enum_TrimState, Model.Y2Trim.trimState));
  writeKeyValue_TrimVal(file, 1, key_CommonTrim, Model.Y2Trim.commonTrim);

  writeKeyValue_Char(file, 0, key_TrimStep, findStringInIdStr(enum_TrimStep, Model.trimStep));
  
  if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
  {
    file.println(F("# ------ Rates and expo ------"));
    
    writeKeyValue_Char(file, 0, (Model.type ==  MODEL_TYPE_AIRPLANE) ? key_RudDualRate : key_YawDualRate, NULL);
    writeKeyValue_S32(file, 1, key_Rate1, Model.RudDualRate.rate1);
    writeKeyValue_S32(file, 1, key_Rate2, Model.RudDualRate.rate2);
    writeKeyValue_S32(file, 1, key_Expo1, Model.RudDualRate.expo1);
    writeKeyValue_S32(file, 1, key_Expo2, Model.RudDualRate.expo2);
    getControlSwitchName_Clean(tempBuff, Model.RudDualRate.swtch, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Switch, tempBuff);
    
    writeKeyValue_Char(file, 0, (Model.type ==  MODEL_TYPE_AIRPLANE) ? key_AilDualRate : key_RollDualRate, NULL);
    writeKeyValue_S32(file, 1, key_Rate1, Model.AilDualRate.rate1);
    writeKeyValue_S32(file, 1, key_Rate2, Model.AilDualRate.rate2);
    writeKeyValue_S32(file, 1, key_Expo1, Model.AilDualRate.expo1);
    writeKeyValue_S32(file, 1, key_Expo2, Model.AilDualRate.expo2);
    getControlSwitchName_Clean(tempBuff, Model.AilDualRate.swtch, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Switch, tempBuff);
    
    writeKeyValue_Char(file, 0, (Model.type ==  MODEL_TYPE_AIRPLANE) ? key_EleDualRate : key_PitchDualRate, NULL);
    writeKeyValue_S32(file, 1, key_Rate1, Model.EleDualRate.rate1);
    writeKeyValue_S32(file, 1, key_Rate2, Model.EleDualRate.rate2);
    writeKeyValue_S32(file, 1, key_Expo1, Model.EleDualRate.expo1);
    writeKeyValue_S32(file, 1, key_Expo2, Model.EleDualRate.expo2);
    getControlSwitchName_Clean(tempBuff, Model.EleDualRate.swtch, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Switch, tempBuff);
    
    file.println(F("# ------ Throttle curve ------"));
  
    writeKeyValue_Char(file, 0, key_ThrottleCurve, NULL);
    writeKeyValue_S32(file, 1, key_NumPoints, Model.ThrottleCurve.numPoints);
    writeArray_S8(file, 1, key_XVal, Model.ThrottleCurve.xVal, Model.ThrottleCurve.numPoints);
    writeArray_S8(file, 1, key_YVal, Model.ThrottleCurve.yVal, Model.ThrottleCurve.numPoints);
    writeKeyValue_bool(file, 1, key_Smooth, Model.ThrottleCurve.smooth);
  }

  file.println(F("# ------ Function generators ------"));
  
  for(uint8_t idx = 0; idx < NUM_FUNCGEN; idx++)
  {
    funcgen_t *fgen = &Model.Funcgen[idx];
    
    writeKeyValue_Char(file, 0, key_Funcgen, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);
    writeKeyValue_Char(file, 1, key_Waveform, findStringInIdStr(enum_FuncgenWaveform, fgen->waveform));

    if(fgen->waveform == FUNCGEN_WAVEFORM_PULSE)
    {
      writeKeyValue_Char(file, 1, key_WidthMode, findStringInIdStr(enum_FuncgenWidthMode, fgen->widthMode));
      writeKeyValue_TimeSeconds(file, 1, key_Width, fgen->width);
      writeKeyValue_TimeSeconds(file, 1, key_Period, fgen->period);
    }
    else
    {
      writeKeyValue_Char(file, 1, key_PeriodMode, findStringInIdStr(enum_FuncgenPeriodMode, fgen->periodMode));
      writeKeyValue_TimeSeconds(file, 1, key_Period1, fgen->period1);
      writeKeyValue_TimeSeconds(file, 1, key_Period2, fgen->period2);
    }

    getSrcName_Clean(tempBuff, fgen->modulatorSrc, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_ModulatorSrc, tempBuff);

    writeKeyValue_bool(file, 1, key_ReverseModulator, fgen->reverseModulator);
    writeKeyValue_Char(file, 1, key_PhaseMode, findStringInIdStr(enum_FuncgenPhaseMode, fgen->phaseMode));
    writeKeyValue_S32(file, 1, key_Phase, fgen->phase);
  }
  
  file.println(F("# ------ Mixer ------"));
  
  for(uint8_t idx = 0; idx < NUM_MIX_SLOTS; idx++)
  {
    mixer_params_t *mxr = &Model.Mixer[idx];
    //skip if input and output are both none
    if(idx > 0 && mxr->input == SRC_NONE && mxr->output == SRC_NONE)
      continue;
    
    writeKeyValue_Char(file, 0, key_Mixer, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);
    writeKeyValue_Char(file, 1, key_Name, mxr->name);
    
    getSrcName_Clean(tempBuff, mxr->output, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Output, tempBuff);
    
    getControlSwitchName_Clean(tempBuff, mxr->swtch, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Switch, tempBuff);

    writeKeyValue_Char(file, 1, key_Operation, findStringInIdStr(enum_MixerOperation, mxr->operation));
    
    if(mxr->operation == MIX_HOLD)
      continue;
    
    getSrcName_Clean(tempBuff, mxr->input, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Input, tempBuff);
    
    writeKeyValue_S32(file, 1, key_Weight, mxr->weight);
    writeKeyValue_S32(file, 1, key_Offset, mxr->offset);
    
    writeKeyValue_Char(file, 1, key_CurveType, findStringInIdStr(enum_MixerCurveType, mxr->curveType));
    if(mxr->curveType == MIX_CURVE_TYPE_FUNCTION)
      writeKeyValue_Char(file, 1, key_CurveVal, findStringInIdStr(enum_MixerCurveType_Func, mxr->curveVal));
    else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
    {
      writeKey_helper(file, 1, key_CurveVal);
      file.print(F("Crv"));
      file.print(mxr->curveVal + 1);
      file.println();
    }
    else
      writeKeyValue_S32(file, 1, key_CurveVal, mxr->curveVal);
    
    writeKeyValue_bool(file, 1, key_TrimEnabled, mxr->trimEnabled);
    
    if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
    {
      writeKey_helper(file, 1, key_FlightMode);
      if(mxr->flightMode == 0xff)
        file.print(F("All"));
      else
      {
        for(uint8_t i = 0; i < NUM_FLIGHT_MODES; i++)
        {
          if((mxr->flightMode >> i) & 0x01)
          {
            file.print(i+1);
            file.print(F(","));
          }
        }
      }
      file.println();
    }

    writeKeyValue_TimeSeconds(file, 1, key_DelayUp, mxr->delayUp);
    writeKeyValue_TimeSeconds(file, 1, key_DelayDown, mxr->delayDown);
    writeKeyValue_TimeSeconds(file, 1, key_SlowUp, mxr->slowUp);
    writeKeyValue_TimeSeconds(file, 1, key_SlowDown, mxr->slowDown);
  }
  
  file.println(F("# ------ Custom curves ------"));
  
  for(uint8_t idx = 0; idx < NUM_CUSTOM_CURVES; idx++)
  {
    custom_curve_t *crv = &Model.CustomCurve[idx];
    writeKeyValue_Char(file, 0, key_CustomCurve, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);
    writeKeyValue_Char(file, 1, key_Name, crv->name);
    writeKeyValue_S32(file, 1, key_NumPoints, crv->numPoints);
    writeArray_S8(file, 1, key_XVal, crv->xVal, crv->numPoints);
    writeArray_S8(file, 1, key_YVal, crv->yVal, crv->numPoints);
    writeKeyValue_bool(file, 1, key_Smooth, crv->smooth);
  }
  
  file.println(F("# ------ Logical switches ------"));
  
  for(uint8_t idx = 0; idx < NUM_LOGICAL_SWITCHES; idx++)
  {
    logical_switch_t *ls = &Model.LogicalSwitch[idx];
    if(idx > 0 && ls->func == LS_FUNC_NONE)
      continue;
    writeKeyValue_Char(file, 0, key_LogicalSwitch, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);
    writeKeyValue_Char(file, 1, key_Func, findStringInIdStr(enum_LogicalSwitch_Func, ls->func));
    
    if(ls->func <= LS_FUNC_GROUP3_LAST)
    {
      getSrcName_Clean(tempBuff, ls->val1, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Val1, tempBuff);
      
      writeKeyValue_S32(file, 1, key_Val2, ls->val2);
      if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
        writeKeyValue_Char(file, 1, key_Val3, findStringInIdStr(enum_DirectionOfChange, ls->val3));
      else
        writeKeyValue_TimeSeconds(file, 1, key_Val3, ls->val3);
      writeKeyValue_TimeSeconds(file, 1, key_Val4, ls->val4);
    }
    else if(ls->func <= LS_FUNC_GROUP4_LAST)
    {
      getSrcName_Clean(tempBuff, ls->val1, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Val1, tempBuff);
      
      getSrcName_Clean(tempBuff, ls->val2, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Val2, tempBuff);
      
      writeKeyValue_TimeSeconds(file, 1, key_Val3, ls->val3);
      writeKeyValue_TimeSeconds(file, 1, key_Val4, ls->val4);
    }
    else if(ls->func <= LS_FUNC_GROUP5_LAST)
    {
      getControlSwitchName_Clean(tempBuff, ls->val1, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Val1, tempBuff);
      
      getControlSwitchName_Clean(tempBuff, ls->val2, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Val2, tempBuff);
      
      writeKeyValue_TimeSeconds(file, 1, key_Val3, ls->val3);
      if(ls->func != LS_FUNC_LATCH)
        writeKeyValue_TimeSeconds(file, 1, key_Val4, ls->val4);
    }
    else if(ls->func == LS_FUNC_TOGGLE)
    {
      getControlSwitchName_Clean(tempBuff, ls->val1, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Val1, tempBuff);
      
      writeKeyValue_Char(file, 1, key_Val2, findStringInIdStr(enum_ClockEdge, ls->val2));
      
      getControlSwitchName_Clean(tempBuff, ls->val3, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Val3, tempBuff);
    }
    else if(ls->func == LS_FUNC_PULSE)
    {
      writeKeyValue_TimeSeconds(file, 1, key_Val1, ls->val1);
      writeKeyValue_TimeSeconds(file, 1, key_Val2, ls->val2);
      writeKeyValue_TimeSeconds(file, 1, key_Val3, ls->val3);
    }
  }

  file.println(F("# ------ Counters ------"));

  for(uint8_t idx = 0; idx < NUM_COUNTERS; idx++)
  {
    counter_params_t *counter = &Model.Counter[idx];

    if(idx > 0 && counter->clock == CTRL_SW_NONE && isEmptyStr(counter->name, sizeof(counter->name)))
      continue;

    writeKeyValue_Char(file, 0, key_Counter, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);
    writeKeyValue_Char(file, 1, key_Name, counter->name);
    writeKeyValue_Char(file, 1, key_Type, findStringInIdStr(enum_CounterType, counter->type));
    if(counter->type == COUNTER_TYPE_BASIC)
    {
      getControlSwitchName_Clean(tempBuff, counter->clock, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Clock, tempBuff);
      writeKeyValue_Char(file, 1, key_Edge, findStringInIdStr(enum_ClockEdge, counter->edge));
      writeKeyValue_S32(file, 1, key_Modulus, counter->modulus);
      writeKeyValue_Char(file, 1, key_Direction, findStringInIdStr(enum_CounterDirection, counter->direction));
    }
    else if(counter->type == COUNTER_TYPE_ADVANCED)
    {
      getControlSwitchName_Clean(tempBuff, counter->incrementClock, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_IncrementClock, tempBuff);
      writeKeyValue_Char(file, 1, key_IncrementEdge, findStringInIdStr(enum_ClockEdge, counter->incrementEdge));
      getControlSwitchName_Clean(tempBuff, counter->decrementClock, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_DecrementClock, tempBuff);
      writeKeyValue_Char(file, 1, key_DecrementEdge, findStringInIdStr(enum_ClockEdge, counter->decrementEdge));
      writeKeyValue_S32(file, 1, key_Modulus, counter->modulus);
    }
    getControlSwitchName_Clean(tempBuff, counter->clear, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Clear, tempBuff);
    writeKeyValue_bool(file, 1, key_RolloverEnabled, counter->rolloverEnabled);
    writeKeyValue_bool(file, 1, key_IsPersistent, counter->isPersistent);
    writeKeyValue_S32(file, 1, key_PersistVal, counter->persistVal);
  }

  if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
  {
    file.println(F("# ------ Flight modes ------"));
    
    for(uint8_t idx = 0; idx < NUM_FLIGHT_MODES; idx++)
    {
      flight_mode_t *fmd = &Model.FlightMode[idx];

      if(idx > 0 && fmd->swtch == CTRL_SW_NONE && isEmptyStr(fmd->name, sizeof(fmd->name)))
        continue;
      
      writeKeyValue_Char(file, 0, key_FlightMode, NULL);
      writeKeyValue_S32(file, 1, key_Number, idx + 1);
      writeKeyValue_Char(file, 1, key_Name, fmd->name);
    
      getControlSwitchName_Clean(tempBuff, fmd->swtch, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Switch, tempBuff);
      
      writeKeyValue_TrimVal(file, 1, key_X1Trim, fmd->x1Trim);
      writeKeyValue_TrimVal(file, 1, key_Y1Trim, fmd->y1Trim);
      writeKeyValue_TrimVal(file, 1, key_X2Trim, fmd->x2Trim);
      writeKeyValue_TrimVal(file, 1, key_Y2Trim, fmd->y2Trim);
      writeKeyValue_TimeSeconds(file, 1, key_TransitionTime, fmd->transitionTime);
    }
  }
  
  file.println(F("# ------ Output channels ------"));
  
  for(uint8_t idx = 0; idx < NUM_RC_CHANNELS; idx++)
  {
    channel_params_t *ch = &Model.Channel[idx];
    writeKeyValue_Char(file, 0, key_Channel, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);
    writeKeyValue_Char(file, 1, key_Name, ch->name);

    if(ch->curve == -1)
      writeKeyValue_Char(file, 1, key_Curve, findStringInIdStr(enum_ChannelCurve, ch->curve));
    else
    {
      writeKey_helper(file, 1, key_Curve);
      file.print(F("Crv"));
      file.print(ch->curve + 1);
      file.println();
    }

    writeKeyValue_bool(file, 1, key_Reverse, ch->reverse);
    writeKeyValue_TrimVal(file, 1, key_Subtrim, ch->subtrim);

    getControlSwitchName_Clean(tempBuff, ch->overrideSwitch, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_OverrideSwitch, tempBuff);

    writeKeyValue_S32(file, 1, key_OverrideVal, ch->overrideVal);
    if(ch->failsafe >= -100)
      writeKeyValue_S32(file, 1, key_Failsafe, ch->failsafe);
    else
      writeKeyValue_Char(file, 1, key_Failsafe, findStringInIdStr(enum_ChannelFailsafe, ch->failsafe));
    writeKeyValue_S32(file, 1, key_EndpointL, ch->endpointL);
    writeKeyValue_S32(file, 1, key_EndpointR, ch->endpointR);
  }

  file.println(F("# ------ Widgets ------"));
  
  for(uint8_t idx = 0; idx < NUM_WIDGETS; idx++)
  {
    widget_params_t *widget = &Model.Widget[idx];
    writeKeyValue_Char(file, 0, key_Widget, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);
    writeKeyValue_Char(file, 1, key_Type, findStringInIdStr(enum_WidgetType, widget->type));
    
    if(widget->type == WIDGET_TYPE_TELEMETRY)
    {
      if(widget->src == WIDGET_SRC_AUTO)
        writeKeyValue_Char(file, 1, key_Src, findStringInIdStr(enum_WidgetSource, widget->src));
      else
      {
        writeKey_helper(file, 1, key_Src);
        file.print(Model.Telemetry[widget->src].name);
        file.println();
        
        writeKeyValue_Char(file, 1, key_Disp, findStringInIdStr(enum_WidgetDisplay, widget->disp));
        writeKeyValue_S32(file, 1, key_GaugeMin, widget->gaugeMin);
        writeKeyValue_S32(file, 1, key_GaugeMax, widget->gaugeMax);
      }
    }
    else if(widget->type == WIDGET_TYPE_MIXSOURCES)
    {
      getSrcName_Clean(tempBuff, widget->src, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Src, tempBuff);
      writeKeyValue_Char(file, 1, key_Disp, findStringInIdStr(enum_WidgetDisplay, widget->disp));
      writeKeyValue_S32(file, 1, key_GaugeMin, widget->gaugeMin);
      writeKeyValue_S32(file, 1, key_GaugeMax, widget->gaugeMax);
    }
    else if(widget->type == WIDGET_TYPE_TIMERS)
    {
      getSrcName_Clean(tempBuff, SRC_TIMER_FIRST + widget->src, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Src, tempBuff);
    }
    else if(widget->type == WIDGET_TYPE_COUNTERS)
    {
      getSrcName_Clean(tempBuff, SRC_COUNTER_FIRST + widget->src, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Src, tempBuff);
    }
    else if(widget->type == WIDGET_TYPE_OUTPUTS)
    {
      getSrcName_Clean(tempBuff, SRC_CH1 + widget->src, sizeof(tempBuff));
      writeKeyValue_Char(file, 1, key_Src, tempBuff);
      writeKeyValue_Char(file, 1, key_Disp, findStringInIdStr(enum_WidgetDisplay, widget->disp));
      writeKeyValue_S32(file, 1, key_GaugeMin, widget->gaugeMin);
      writeKeyValue_S32(file, 1, key_GaugeMax, widget->gaugeMax);
    }
  }

  file.println(F("# ------ Notifications ------"));

  for(uint8_t idx = 0; idx < NUM_CUSTOM_NOTIFICATIONS; idx++)
  {
    notification_params_t *notification = &Model.CustomNotification[idx];

    if(idx > 0 && notification->swtch == CTRL_SW_NONE && isEmptyStr(notification->text, sizeof(notification->text)))
      continue;

    writeKeyValue_Char(file, 0, key_Notification, NULL);
    writeKeyValue_S32(file, 1, key_Number, idx + 1);

    writeKeyValue_bool(file, 1, key_Enabled, notification->enabled);

    getControlSwitchName_Clean(tempBuff, notification->swtch, sizeof(tempBuff));
    writeKeyValue_Char(file, 1, key_Switch, tempBuff);

    writeKey_helper(file, 1, key_Tone);
    file.print(F("Tone"));
    file.print(notification->tone - AUDIO_NOTIFICATION_TONE_FIRST + 1);
    file.println();

    writeKeyValue_Char(file, 1, key_Text, notification->text);
  }

}
