#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#include "../mtx.h"
#include "modelStrings.h"
#include "modelImport.h"

bool isEndOfFile = false;

//Buffers used by the parser to hold the keys and value. 
//Three buffers are used for the keys as we have upto three indention levels. ie level 0,1,2

char keyBuff[3][MAX_STR_SIZE];
char valueBuff[MAX_STR_SIZE];

//--------------------------- Parser ---------------------------------------------------------------

/*
  Reads in the file line by line, extracts the keys and values to the appropriate buffers and skipping 
  comments and empty lines. When the end of file is reached, the isEndOfFile flag is set to true. 
*/

void parser(File& file)
{
  //--- Read characters one by one until a newline or end of file is encountered
  char lineBuff[MAX_STR_SIZE * 3]; //seems enough
  size_t count = 0;
  while(file.available())
  {
    uint8_t c = file.read();
    if(c == '\r') //skip carriage return
      continue;
    if(c == '\n')
      break;
    if(count < sizeof(lineBuff) - 1)
      lineBuff[count++] = c;
  }
  lineBuff[count] = '\0'; //null terminate
  
  //check if end of file
  if(!file.available())
    isEndOfFile = true;
  
  //--- Get the indent level
  uint8_t indentLevel = 0;
  static uint8_t prevIndentLevel = 0;
  uint8_t numLeadingSpaces = 0;
  for(uint8_t i = 0; i < sizeof(lineBuff); i++)
  {
    if(lineBuff[i] != ' ')
      break;
    numLeadingSpaces++;
  }
  indentLevel = numLeadingSpaces / 2;
  //return if invalid indention
  if(indentLevel >= sizeof(keyBuff)/sizeof(keyBuff[0]) || numLeadingSpaces % 2 == 1)
    return;
  
  //--- Trim white space at beginning and end
  trimWhiteSpace(lineBuff, sizeof(lineBuff));
  
  //--- Check if the line is a comment line or empty line
  if(lineBuff[0] == '#' || lineBuff[0] == '\0')
    return;

  //--- Find and remove inline comments
  //Find the '#' character in the line.
  //If '#' is found, truncate the line at that position to remove the comment
  char *commentStart = strchr(lineBuff, '#');
  if(commentStart != NULL)
    *commentStart = '\0';
  
  //--- Get the key
  char tempKeyStr[MAX_STR_SIZE];
  //copy all characters before the ':'
  count = 0;
  for(uint8_t i = 0; i < sizeof(lineBuff); i++)
  {
    uint8_t c = lineBuff[i];
    if(c == '\0')
      break;
    if(c == ':')
    {
      lineBuff[i] = ' '; //substitute with space
      break;
    }
    if(count < sizeof(tempKeyStr) - 1)
      tempKeyStr[count++] = c;
    lineBuff[i] = ' '; //substitute with space
  }
  tempKeyStr[count] = '\0'; //null terminate
  //copy from temp into corresponding buffer
  strlcpy(keyBuff[indentLevel], tempKeyStr, sizeof(keyBuff[0]));
  //check if we have moved backwards of the tree, in which case we nullify all children
  if(indentLevel < prevIndentLevel) 
  {
    uint8_t i = prevIndentLevel;
    while(i > indentLevel)
    {
      keyBuff[i][0] = '\0';
      i--;
    }
  }
  
  //--- Get the value
  trimWhiteSpace(lineBuff, sizeof(lineBuff));
  strlcpy(valueBuff, lineBuff, sizeof(valueBuff));
  
  //--- Save the indention level
  prevIndentLevel = indentLevel;
}

//--------------------------------------------------------------------------------------------------

void readValue_bool(char* str, bool* val)
{
  if(MATCH_P(str, PSTR("true")))
    *val = true;
  else if(MATCH_P(str, PSTR("false")))
    *val = false;
}

//--------------------------------------------------------------------------------------------------

int32_t atoi_with_prefix(const char *str) //todo rename this
{
  int32_t result = 0;
  int32_t sign = 1; // 1 for positive, -1 for negative

  // Skip leading non-numeric characters
  while(*str && (*str < '0' || *str > '9')) 
  {
    if(*str == '-') 
    {
      sign = -1; // Handle negative sign if present
    }
    str++;
  }

  // Process digits and build the integer
  while(*str >= '0' && *str <= '9') 
  {
    result = result * 10 + (*str - '0');
    str++;
  }

  // Apply the sign
  return sign * result;
}

//--------------------------------------------------------------------------------------------------

uint8_t getSrcId(char* str)
{
  char tempBuff[MAX_STR_SIZE];
  tempBuff[0] = '\0';
  for(uint8_t i = 0; i < MIXSOURCES_COUNT + NUM_COUNTERS + NUM_CUSTOM_TELEMETRY; i++)
  {
    getSrcName(tempBuff, i, sizeof(tempBuff));
    if(MATCH(tempBuff, str))
      return i;
  }
  return 0;
}

//--------------------------------------------------------------------------------------------------

uint8_t getControlSwitchID(char* str)
{
  char tempBuff[MAX_STR_SIZE];
  tempBuff[0] = '\0';
  for(uint8_t i = 0; i < CTRL_SW_COUNT + NUM_FLIGHT_MODES * 2; i++)
  {
    getControlSwitchName_Clean(tempBuff, i, sizeof(tempBuff));
    if(MATCH(tempBuff, str))
      return i;
  }
  return 0;
}

//--------------------------------------------------------------------------------------------------

uint16_t getTimeFromTimeStr(char* str)
{
  //Converts the time string to deciseconds. Forexample 12.5s to becomes 125

  char tempBuff[MAX_STR_SIZE];
  tempBuff[0] = '\0';
  uint8_t i = 0;

  uint8_t pos = 0;
  bool seenDecimalPoint = false;
  
  while(true)
  {
    uint8_t c = *(str + pos);
    if(c == '\0')
      break;
    if(isDigit(c))
      tempBuff[i++] = c;
    if(c == '.')
    {
      seenDecimalPoint = true;
      pos++;
      continue;
    }
    if(seenDecimalPoint)
      break;
    pos++;
    if(pos == 0xff) //catch infinite loop
      break;
  }

  tempBuff[i] = '\0';

  uint16_t result = atoi_with_prefix(tempBuff); 
  return result;
}

//--------------------------- Extractors -----------------------------------------------------------

void extractConfig_ModelName()
{
  strlcpy(Model.name, valueBuff, sizeof(Model.name));
}

void extractConfig_ModelType()
{
  findIdInIdStr(enum_ModelType, valueBuff, Model.type);
}

void extractConfig_RudSrc()
{
  Model.rudSrcRaw = getSrcId(valueBuff);
}

void extractConfig_AilSrc()
{
  Model.ailSrcRaw = getSrcId(valueBuff);
}

void extractConfig_EleSrc()
{
  Model.eleSrcRaw = getSrcId(valueBuff);
}

void extractConfig_ThrSrc()
{
  Model.thrSrcRaw = getSrcId(valueBuff);
}

void extractConfig_ThrottleWarning()
{
  readValue_bool(valueBuff, &Model.checkThrottle);
}

void extractConfig_SwitchWarning()
{
  uint8_t idx = getSrcId(keyBuff[1]) - SRC_SW_PHYSICAL_FIRST;
  if(idx < MAX_NUM_PHYSICAL_SWITCHES)
    findIdInIdStr(enum_SwitchWarn, valueBuff, Model.switchWarn[idx]);
}

void extractConfig_Telemetry()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_CUSTOM_TELEMETRY)
  {
    telemetry_params_t* tlm = &Model.Telemetry[idx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(tlm->name, valueBuff, sizeof(tlm->name));
    else if(MATCH_P(keyBuff[1], key_UnitsName))
      strlcpy(tlm->unitsName, valueBuff, sizeof(tlm->unitsName));
    else if(MATCH_P(keyBuff[1], key_Identifier))
      tlm->identifier = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Factor10))
      tlm->factor10 = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Multiplier))
      tlm->multiplier = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Offset))
      tlm->offset = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_AlarmCondition))
      findIdInIdStr(enum_TelemetryAlarmCondition, valueBuff, tlm->alarmCondition);
    else if(MATCH_P(keyBuff[1], key_AlarmThreshold))
      tlm->alarmThreshold = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_ShowOnHome))
      readValue_bool(valueBuff, &tlm->showOnHome);
    else if(MATCH_P(keyBuff[1], key_RecordMaximum))
      readValue_bool(valueBuff, &tlm->recordMaximum);
    else if(MATCH_P(keyBuff[1], key_RecordMinimum))
      readValue_bool(valueBuff, &tlm->recordMinimum);
  }
}

void extractConfig_Timers()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_TIMERS)
  {
    timer_params_t* tmr = &Model.Timer[idx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(tmr->name, valueBuff, sizeof(tmr->name));
    else if(MATCH_P(keyBuff[1], key_Switch))
      tmr->swtch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_ResetSwitch))
      tmr->resetSwitch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_InitialMinutes))
      tmr->initialMinutes = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_IsPersistent))
      readValue_bool(valueBuff, &tmr->isPersistent);
    else if(MATCH_P(keyBuff[1], key_PersistVal))
      tmr->persistVal = atoi_with_prefix(valueBuff);
  }
}

void extractConfig_X1Trim()
{
  if(MATCH_P(keyBuff[1], key_TrimState))
    findIdInIdStr(enum_TrimState, valueBuff, Model.X1Trim.trimState);
  else if(MATCH_P(keyBuff[1], key_CommonTrim))
    Model.X1Trim.commonTrim = atoi_with_prefix(valueBuff);
}

void extractConfig_Y1Trim()
{
  if(MATCH_P(keyBuff[1], key_TrimState))
    findIdInIdStr(enum_TrimState, valueBuff, Model.Y1Trim.trimState);
  else if(MATCH_P(keyBuff[1], key_CommonTrim))
    Model.Y1Trim.commonTrim = atoi_with_prefix(valueBuff);
}

void extractConfig_X2Trim()
{
  if(MATCH_P(keyBuff[1], key_TrimState))
    findIdInIdStr(enum_TrimState, valueBuff, Model.X2Trim.trimState);
  else if(MATCH_P(keyBuff[1], key_CommonTrim))
    Model.X2Trim.commonTrim = atoi_with_prefix(valueBuff);
}

void extractConfig_Y2Trim()
{
  if(MATCH_P(keyBuff[1], key_TrimState))
    findIdInIdStr(enum_TrimState, valueBuff, Model.Y2Trim.trimState);
  else if(MATCH_P(keyBuff[1], key_CommonTrim))
    Model.Y2Trim.commonTrim = atoi_with_prefix(valueBuff);
}

void extractConfig_RudDualRate()
{
  if(MATCH_P(keyBuff[1], key_Rate1))
    Model.RudDualRate.rate1 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Rate2))
    Model.RudDualRate.rate2 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Expo1))
    Model.RudDualRate.expo1 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Expo2))
    Model.RudDualRate.expo2 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Switch))
    Model.RudDualRate.swtch = getControlSwitchID(valueBuff);
}

void extractConfig_AilDualRate()
{
  if(MATCH_P(keyBuff[1], key_Rate1))
    Model.AilDualRate.rate1 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Rate2))
    Model.AilDualRate.rate2 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Expo1))
    Model.AilDualRate.expo1 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Expo2))
    Model.AilDualRate.expo2 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Switch))
    Model.AilDualRate.swtch = getControlSwitchID(valueBuff);
}

void extractConfig_EleDualRate()
{
  if(MATCH_P(keyBuff[1], key_Rate1))
    Model.EleDualRate.rate1 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Rate2))
    Model.EleDualRate.rate2 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Expo1))
    Model.EleDualRate.expo1 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Expo2))
    Model.EleDualRate.expo2 = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Switch))
    Model.EleDualRate.swtch = getControlSwitchID(valueBuff);
}

void extractConfig_ThrottleCurve()
{
  custom_curve_t* crv = &Model.ThrottleCurve;
  if(MATCH_P(keyBuff[1], key_NumPoints))
  {
    crv->numPoints = atoi_with_prefix(valueBuff);
    if(crv->numPoints > MAX_NUM_POINTS_CUSTOM_CURVE)
      crv->numPoints = MAX_NUM_POINTS_CUSTOM_CURVE;
  }
  else if(MATCH_P(keyBuff[1], key_XVal))
  {
    uint8_t pt = atoi_with_prefix(keyBuff[2]);
    if(pt == 0) 
      crv->xVal[0] = -100;
    else if(pt == crv->numPoints - 1) 
      crv->xVal[crv->numPoints - 1] = 100;
    else 
      crv->xVal[pt] = atoi_with_prefix(valueBuff);
  }
  else if(MATCH_P(keyBuff[1], key_YVal))
  {
    uint8_t pt = atoi_with_prefix(keyBuff[2]);
    if(pt >= crv->numPoints)
      pt = crv->numPoints - 1;
    crv->yVal[pt] = atoi_with_prefix(valueBuff);
  }
  else if(MATCH_P(keyBuff[1], key_Smooth))
    readValue_bool(valueBuff, &crv->smooth);
}

void extractConfig_FunctionGenerators()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_FUNCGEN)
  {
    funcgen_t* fgen = &Model.Funcgen[idx];
    if(MATCH_P(keyBuff[1], key_Waveform))
      findIdInIdStr(enum_FuncgenWaveform, valueBuff, fgen->waveform);
    else if(MATCH_P(keyBuff[1], key_PeriodMode))
      findIdInIdStr(enum_FuncgenPeriodMode, valueBuff, fgen->periodMode);
    else if(MATCH_P(keyBuff[1], key_Period1))
      fgen->period1 = getTimeFromTimeStr(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Period2))
      fgen->period2 = getTimeFromTimeStr(valueBuff);
    else if(MATCH_P(keyBuff[1], key_ModulatorSrc))
      fgen->modulatorSrc = getSrcId(valueBuff);
    else if(MATCH_P(keyBuff[1], key_ReverseModulator))
      readValue_bool(valueBuff, &fgen->reverseModulator);
    else if(MATCH_P(keyBuff[1], key_PhaseMode))
      findIdInIdStr(enum_FuncgenPhaseMode, valueBuff, fgen->phaseMode);
    else if(MATCH_P(keyBuff[1], key_PhaseAngle))
      fgen->phaseAngle = atoi_with_prefix(valueBuff);
  }
}

void extractConfig_Mixer()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_MIXSLOTS)
  {
    mixer_params_t* mxr = &Model.Mixer[idx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(mxr->name, valueBuff, sizeof(mxr->name));
    else if(MATCH_P(keyBuff[1], key_Output))
      mxr->output = getSrcId(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Switch))
      mxr->swtch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Operation))
      findIdInIdStr(enum_MixerOperation, valueBuff, mxr->operation);
    else if(MATCH_P(keyBuff[1], key_Input))
      mxr->input = getSrcId(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Weight))
      mxr->weight = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Offset))
      mxr->offset = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_CurveType))
      findIdInIdStr(enum_MixerCurveType, valueBuff, mxr->curveType);
    else if(MATCH_P(keyBuff[1], key_CurveVal))
    {
      if(mxr->curveType == MIX_CURVE_TYPE_DIFF || mxr->curveType == MIX_CURVE_TYPE_EXPO)
        mxr->curveVal = atoi_with_prefix(valueBuff);
      else if(mxr->curveType == MIX_CURVE_TYPE_FUNCTION)
        findIdInIdStr(enum_MixerCurveType_Func, valueBuff, mxr->curveVal);
      else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
        mxr->curveVal = atoi_with_prefix(valueBuff) - 1;
    }
    else if(MATCH_P(keyBuff[1], key_TrimEnabled))
      readValue_bool(valueBuff, &mxr->trimEnabled);
    else if(MATCH_P(keyBuff[1], key_FlightMode))
    {
      if(MATCH_P(valueBuff, PSTR("All")))
        mxr->flightMode = 0xff;
      else //1,2,3,5,
      {
        mxr->flightMode = 0xff;
        mxr->flightMode <<= NUM_FLIGHT_MODES;
        uint8_t pos = 0;
        while(true)
        {
          if(valueBuff[pos] == '\0' || pos == 0xff) //reached end
            break;
          if(isDigit(valueBuff[pos]))
          {
            uint8_t i = valueBuff[pos] - '1';
            mxr->flightMode |= 1 << i;
          }
          pos++;
        }
      }
    }
    else if(MATCH_P(keyBuff[1], key_DelayUp))
      mxr->delayUp = getTimeFromTimeStr(valueBuff);
    else if(MATCH_P(keyBuff[1], key_DelayDown))
      mxr->delayDown = getTimeFromTimeStr(valueBuff);
    else if(MATCH_P(keyBuff[1], key_SlowUp))
      mxr->slowUp = getTimeFromTimeStr(valueBuff);
    else if(MATCH_P(keyBuff[1], key_SlowDown))
      mxr->slowDown = getTimeFromTimeStr(valueBuff);
  }
}

void extractConfig_CustomCurves()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_CUSTOM_CURVES)
  {
    custom_curve_t* crv = &Model.CustomCurve[idx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(crv->name, valueBuff, sizeof(crv->name));
    else if(MATCH_P(keyBuff[1], key_NumPoints))
    {
      crv->numPoints = atoi_with_prefix(valueBuff);
      if(crv->numPoints > MAX_NUM_POINTS_CUSTOM_CURVE)
        crv->numPoints = MAX_NUM_POINTS_CUSTOM_CURVE;
    }
    else if(MATCH_P(keyBuff[1], key_XVal))
    {
      uint8_t pt = atoi_with_prefix(keyBuff[2]);
      if(pt == 0) 
        crv->xVal[0] = -100;
      else if(pt == crv->numPoints - 1) 
        crv->xVal[crv->numPoints - 1] = 100;
      else 
        crv->xVal[pt] = atoi_with_prefix(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_YVal))
    {
      uint8_t pt = atoi_with_prefix(keyBuff[2]);
      if(pt >= crv->numPoints)
        pt = crv->numPoints - 1;
      crv->yVal[pt] = atoi_with_prefix(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_Smooth))
      readValue_bool(valueBuff, &crv->smooth);
  }
}

void extractConfig_LogicalSwitches()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_LOGICAL_SWITCHES)
  {
    logical_switch_t* ls = &Model.LogicalSwitch[idx];
    if(MATCH_P(keyBuff[1], key_Func))
      findIdInIdStr(enum_LogicalSwitch_Func, valueBuff, ls->func);
    else if(MATCH_P(keyBuff[1], key_Val1))
    {
      if(ls->func <= LS_FUNC_GROUP1_LAST)
        ls->val1 = getSrcId(valueBuff);
      else if(ls->func <= LS_FUNC_GROUP2_LAST)
        ls->val1 = getSrcId(valueBuff);
      else if(ls->func <= LS_FUNC_GROUP3_LAST)
        ls->val1 = getControlSwitchID(valueBuff);
      else if(ls->func == LS_FUNC_TOGGLE)
        ls->val1 = getControlSwitchID(valueBuff);
      else if(ls->func == LS_FUNC_PULSE)
        ls->val1 = getTimeFromTimeStr(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_Val2))
    {
      if(ls->func <= LS_FUNC_GROUP1_LAST)
        ls->val2 = atoi_with_prefix(valueBuff);
      else if(ls->func <= LS_FUNC_GROUP2_LAST)
        ls->val2 = getSrcId(valueBuff);
      else if(ls->func <= LS_FUNC_GROUP3_LAST)
        ls->val2 = getControlSwitchID(valueBuff);
      else if(ls->func == LS_FUNC_TOGGLE)
        findIdInIdStr(enum_ClockEdge, valueBuff, ls->val2);
      else if(ls->func == LS_FUNC_PULSE)
        ls->val2 = getTimeFromTimeStr(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_Val3))
    {
      if(ls->func == LS_FUNC_TOGGLE)
        ls->val3 = getControlSwitchID(valueBuff);
      else
        ls->val3 = getTimeFromTimeStr(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_Val4))
    {
      ls->val4 = getTimeFromTimeStr(valueBuff);
    }
  }
}

void extractConfig_Counters()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_COUNTERS)
  {
    counter_params_t* counter = &Model.Counter[idx];
    if(MATCH_P(keyBuff[1], key_Clock))
      counter->clock = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Edge))
      findIdInIdStr(enum_ClockEdge, valueBuff, counter->edge);
    else if(MATCH_P(keyBuff[1], key_Clear))
      counter->clear = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Modulus))
      counter->modulus = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Direction))
    {
      if(MATCH_P(valueBuff, PSTR("Up")))
        counter->direction = 0;
      else if(MATCH_P(valueBuff, PSTR("Down")))
        counter->direction = 1;
    }
    else if(MATCH_P(keyBuff[1], key_IsPersistent))
      readValue_bool(valueBuff, &counter->isPersistent);
    else if(MATCH_P(keyBuff[1], key_PersistVal))
      counter->persistVal = atoi_with_prefix(valueBuff);
  }
}

void extractConfig_FlightModes()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_FLIGHT_MODES)
  {
    flight_mode_t* fmd = &Model.FlightMode[idx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(fmd->name, valueBuff, sizeof(fmd->name));
    else if(MATCH_P(keyBuff[1], key_Switch))
      fmd->swtch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_X1Trim))
      fmd->x1Trim = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Y1Trim))
      fmd->y1Trim = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_X2Trim))
      fmd->x2Trim = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Y2Trim))
      fmd->y2Trim = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_TransitionTime))
      fmd->transitionTime = getTimeFromTimeStr(valueBuff);
  }
}

void extractConfig_Channels()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_RC_CHANNELS)
  {
    channel_params_t* ch = &Model.Channel[idx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(ch->name, valueBuff, sizeof(ch->name));
    else if(MATCH_P(keyBuff[1], key_Curve))
    {
      char tempBuff[MAX_STR_SIZE];
      tempBuff[0] = '\0';
      strlcpy(tempBuff, findStringInIdStr(enum_ChannelCurve, -1), sizeof(tempBuff));
      if(MATCH(tempBuff, valueBuff))
        ch->curve = -1;
      else
        ch->curve = atoi_with_prefix(valueBuff) - 1;
    }
    else if(MATCH_P(keyBuff[1], key_Reverse))
      readValue_bool(valueBuff, &ch->reverse);
    else if(MATCH_P(keyBuff[1], key_Subtrim))
      ch->subtrim = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_OverrideSwitch))
      ch->overrideSwitch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_OverrideVal))
      ch->overrideVal = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Failsafe)) //todo improve this
    {
      char tempBuff[MAX_STR_SIZE];
      tempBuff[0] = '\0';
      strlcpy(tempBuff, findStringInIdStr(enum_ChannelFailsafe, -102), sizeof(tempBuff));
      if(MATCH(tempBuff, valueBuff))
        ch->failsafe = -102;
      else
      {
        strlcpy(tempBuff, findStringInIdStr(enum_ChannelFailsafe, -101), sizeof(tempBuff));
        if(MATCH(tempBuff, valueBuff))
          ch->failsafe = -101;
        else
          ch->failsafe = atoi_with_prefix(valueBuff);
      }
    }
    else if(MATCH_P(keyBuff[1], key_EndpointL))
      ch->endpointL = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_EndpointR))
      ch->endpointR = atoi_with_prefix(valueBuff);
  }
}

void extractConfig_Widgets()
{
  static uint8_t idx = 0xff;
  if(MATCH_P(keyBuff[1], key_Number))
    idx = atoi_with_prefix(valueBuff) - 1;
  if(idx < NUM_WIDGETS)
  {
    widget_params_t* widget = &Model.Widget[idx];
    if(MATCH_P(keyBuff[1], key_Type))
      findIdInIdStr(enum_WidgetType, valueBuff, widget->type);
    else if(MATCH_P(keyBuff[1], key_Src))
    {
      if(widget->type == WIDGET_TYPE_MIXSOURCES)
        widget->src = getSrcId(valueBuff);
      else if(widget->type == WIDGET_TYPE_COUNTERS 
              || widget->type == WIDGET_TYPE_TIMERS 
              || widget->type == WIDGET_TYPE_OUTPUTS)
      {
        widget->src = atoi_with_prefix(valueBuff) - 1;
      }
      else if(widget->type == WIDGET_TYPE_TELEMETRY)
      {
        char tempBuff[MAX_STR_SIZE];
        tempBuff[0] = '\0';
        strlcpy(tempBuff, findStringInIdStr(enum_WidgetSource, WIDGET_SRC_AUTO), sizeof(tempBuff));
        if(MATCH(tempBuff, valueBuff))
          widget->src = WIDGET_SRC_AUTO;
        else
          widget->src = getSrcId(valueBuff) - (MIXSOURCES_COUNT + NUM_COUNTERS);
      }
    }
    else if(MATCH_P(keyBuff[1], key_Disp))
      findIdInIdStr(enum_WidgetDisplay, valueBuff, widget->disp);
    else if(MATCH_P(keyBuff[1], key_GaugeMin))
      widget->gaugeMin = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_GaugeMax))
      widget->gaugeMax = atoi_with_prefix(valueBuff);
  }
}

//============================ Core function =======================================================

void importModelData(File& file)
{
  // Calls the parser and then the extractor functions on the corresponding root item. 
  // The extractor functions have no control over the parser. They neither call it to fetch the next line,
  // nor modify the key buffers used by the parser.
  
  //--- Initialise
  isEndOfFile = false;
  memset(keyBuff[0], 0, sizeof(keyBuff[0]));
  memset(keyBuff[1], 0, sizeof(keyBuff[0]));
  memset(keyBuff[2], 0, sizeof(keyBuff[0]));
  memset(valueBuff, 0, sizeof(valueBuff));

  //--- Loop 
  while(!isEndOfFile)
  {
    //clear value buffer so we dont work with stale values
    *valueBuff = '\0';

    //call parser
    parser(file);

    //if value is empty, continue parsing
    if(*valueBuff == '\0')
      continue;
    
    //call the appropriate extractor function, depending on the root key
    if(MATCH_P(keyBuff[0], key_ModelName)) extractConfig_ModelName();
    else if(MATCH_P(keyBuff[0], key_ModelType)) extractConfig_ModelType();
    else if(MATCH_P(keyBuff[0], key_RudSrc)) extractConfig_RudSrc();
    else if(MATCH_P(keyBuff[0], key_YawSrc)) extractConfig_RudSrc();
    else if(MATCH_P(keyBuff[0], key_AilSrc)) extractConfig_AilSrc();
    else if(MATCH_P(keyBuff[0], key_RollSrc)) extractConfig_AilSrc();
    else if(MATCH_P(keyBuff[0], key_EleSrc)) extractConfig_EleSrc();
    else if(MATCH_P(keyBuff[0], key_PitchSrc)) extractConfig_EleSrc();
    else if(MATCH_P(keyBuff[0], key_ThrSrc)) extractConfig_ThrSrc();
    else if(MATCH_P(keyBuff[0], key_CheckThrottle)) extractConfig_ThrottleWarning();
    else if(MATCH_P(keyBuff[0], key_SwitchWarn)) extractConfig_SwitchWarning();
    else if(MATCH_P(keyBuff[0], key_Telemetry)) extractConfig_Telemetry();
    else if(MATCH_P(keyBuff[0], key_Timer)) extractConfig_Timers();
    else if(MATCH_P(keyBuff[0], key_X1Trim)) extractConfig_X1Trim();
    else if(MATCH_P(keyBuff[0], key_Y1Trim)) extractConfig_Y1Trim();
    else if(MATCH_P(keyBuff[0], key_X2Trim)) extractConfig_X2Trim();
    else if(MATCH_P(keyBuff[0], key_Y2Trim)) extractConfig_Y2Trim();
    else if(MATCH_P(keyBuff[0], key_RudDualRate)) extractConfig_RudDualRate();
    else if(MATCH_P(keyBuff[0], key_YawDualRate)) extractConfig_RudDualRate();
    else if(MATCH_P(keyBuff[0], key_AilDualRate)) extractConfig_AilDualRate();
    else if(MATCH_P(keyBuff[0], key_RollDualRate)) extractConfig_AilDualRate();
    else if(MATCH_P(keyBuff[0], key_EleDualRate)) extractConfig_EleDualRate();
    else if(MATCH_P(keyBuff[0], key_PitchDualRate)) extractConfig_EleDualRate();
    else if(MATCH_P(keyBuff[0], key_ThrottleCurve)) extractConfig_ThrottleCurve();
    else if(MATCH_P(keyBuff[0], key_Funcgen)) extractConfig_FunctionGenerators();
    else if(MATCH_P(keyBuff[0], key_Mixer)) extractConfig_Mixer();
    else if(MATCH_P(keyBuff[0], key_CustomCurve)) extractConfig_CustomCurves();
    else if(MATCH_P(keyBuff[0], key_LogicalSwitch)) extractConfig_LogicalSwitches();
    else if(MATCH_P(keyBuff[0], key_Counter)) extractConfig_Counters();
    else if(MATCH_P(keyBuff[0], key_FlightMode)) extractConfig_FlightModes();
    else if(MATCH_P(keyBuff[0], key_Channel)) extractConfig_Channels();
    else if(MATCH_P(keyBuff[0], key_Widget)) extractConfig_Widgets();
  }
}

