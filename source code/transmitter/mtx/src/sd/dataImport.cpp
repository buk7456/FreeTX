#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#include "../common.h"
#include "../ui/ui.h" 
#include "../stringDefs.h"
#include "../templates.h"
#include "dataImport.h"

bool isEndOfFile = false;

//for basic error detection only
bool hasEncounteredInvalidParam = false;
uint16_t dbgLineNumber = 0;
uint16_t dbgFirstErrorLineNumber = 0;
uint16_t dbgTotalErrorLines = 0;

//Buffers used by the parser to hold the keys and value. 
//Three buffers are used for the keys as we have up to three indention levels. i.e. level 0,1,2

char keyBuff[3][MAX_STR_SIZE];
char valueBuff[MAX_STR_SIZE];

uint8_t thisIdx; //used inside the extractor functions

//--------------------------- Parser ---------------------------------------------------------------

// Reads in the file line by line, extracts the keys and values to the appropriate buffers and skipping 
// comments and empty lines. When the end of file is reached, the isEndOfFile flag is set to true. 

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
    {
      dbgLineNumber++;
      break;
    }
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

  //--- Trim white space at beginning and end
  trimWhiteSpace(lineBuff, sizeof(lineBuff));
  
  //--- Check if the line is a comment line or empty line
  if(lineBuff[0] == '#' || lineBuff[0] == '\0')
    return;
  
  //--- Check if invalid indention
  if(indentLevel >= sizeof(keyBuff)/sizeof(keyBuff[0]) || numLeadingSpaces % 2 == 1)
  {
    hasEncounteredInvalidParam = true;
    return;
  }

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

void readValue_bool(const char* str, bool* val)
{
  if(MATCH_P(str, PSTR("true")))
    *val = true;
  else if(MATCH_P(str, PSTR("false")))
    *val = false;
  else
    hasEncounteredInvalidParam = true;
}

//--------------------------------------------------------------------------------------------------

int32_t atoi_with_prefix(const char *str)
{
  int32_t result = 0;
  bool isNegative = false;
  // Skip leading non-numeric characters
  while(*str && (*str < '0' || *str > '9')) 
  {
    if(*str == '-') 
      isNegative = true;
    str++;
  }
  // Process digits and build the integer
  while(*str >= '0' && *str <= '9') 
  {
    result = result * 10 + (*str - '0');
    str++;
  }
  return isNegative ? -result : result;
}

//--------------------------------------------------------------------------------------------------

uint8_t getSrcId(const char* str)
{
  char tempBuff[MAX_STR_SIZE];
  tempBuff[0] = '\0';
  for(uint8_t i = 0; i < TOTAL_SOURCES_COUNT; i++)
  {
    getSrcName_Clean(tempBuff, i, sizeof(tempBuff));
    if(MATCH(tempBuff, str))
      return i;
  }
  hasEncounteredInvalidParam = true;
  return SRC_NONE;
}

//--------------------------------------------------------------------------------------------------

uint8_t getControlSwitchID(const char* str)
{
  char tempBuff[MAX_STR_SIZE];
  tempBuff[0] = '\0';
  for(uint8_t i = 0; i < CTRL_SW_COUNT; i++)
  {
    getControlSwitchName_Clean(tempBuff, i, sizeof(tempBuff));
    if(MATCH(tempBuff, str))
      return i;
  }
  hasEncounteredInvalidParam = true;
  return CTRL_SW_NONE;
}

//--------------------------------------------------------------------------------------------------

int32_t getFixedPointVal10(const char* str)
{
  //Converts decimal string to fixed point representation with factor of 10
  //For example 12.3 becomes 123,  -12.3 becomes -123
  //If no decimal point exists, the value is still multiplied by 10

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
    if((c == '-' || isDigit(c)) && i < sizeof(tempBuff) - 2)
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
  
  if(!seenDecimalPoint)
    tempBuff[i++] = '0';
 
  tempBuff[i] = '\0';
  return atoi_with_prefix(tempBuff); 
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

void extractConfig_SecondaryRcvrEnabled()
{
  readValue_bool(valueBuff, &Model.secondaryRcvrEnabled);
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
  thisIdx = getSrcId(keyBuff[1]) - SRC_SW_PHYSICAL_FIRST;
  if(thisIdx < NUM_PHYSICAL_SWITCHES)
    findIdInIdStr(enum_SwitchWarn, valueBuff, Model.switchWarn[thisIdx]);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Telemetry()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_CUSTOM_TELEMETRY)
  {
    telemetry_params_t* tlm = &Model.Telemetry[thisIdx];
    if(MATCH_P(keyBuff[1], key_Type))
      findIdInIdStr(enum_TelemetryType, valueBuff, tlm->type);
    else if(MATCH_P(keyBuff[1], key_Name))
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
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_GnssDistanceUnits()
{
  findIdInIdStr(enum_DisplayedUnits, valueBuff, Model.gnssDistanceUnits);
}

void extractConfig_GnssSpeedUnits()
{
  findIdInIdStr(enum_DisplayedUnits, valueBuff, Model.gnssSpeedUnits);
}

void extractConfig_GnssAltitudeUnits()
{
  findIdInIdStr(enum_DisplayedUnits, valueBuff, Model.gnssAltitudeUnits);
}

void extractConfig_Timers()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_TIMERS)
  {
    timer_params_t* tmr = &Model.Timer[thisIdx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(tmr->name, valueBuff, sizeof(tmr->name));
    else if(MATCH_P(keyBuff[1], key_Switch))
      tmr->swtch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_ResetSwitch))
      tmr->resetSwitch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_InitialMinutes)) //deprecated
      tmr->initialSeconds = atoi_with_prefix(valueBuff) * 60;
    else if(MATCH_P(keyBuff[1], key_InitialSeconds))
      tmr->initialSeconds = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_IsPersistent))
      readValue_bool(valueBuff, &tmr->isPersistent);
    else if(MATCH_P(keyBuff[1], key_PersistVal))
      tmr->persistVal = atoi_with_prefix(valueBuff);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_X1Trim()
{
  if(MATCH_P(keyBuff[1], key_TrimState))
    findIdInIdStr(enum_TrimState, valueBuff, Model.X1Trim.trimState);
  else if(MATCH_P(keyBuff[1], key_CommonTrim))
    Model.X1Trim.commonTrim = getFixedPointVal10(valueBuff);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Y1Trim()
{
  if(MATCH_P(keyBuff[1], key_TrimState))
    findIdInIdStr(enum_TrimState, valueBuff, Model.Y1Trim.trimState);
  else if(MATCH_P(keyBuff[1], key_CommonTrim))
    Model.Y1Trim.commonTrim = getFixedPointVal10(valueBuff);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_X2Trim()
{
  if(MATCH_P(keyBuff[1], key_TrimState))
    findIdInIdStr(enum_TrimState, valueBuff, Model.X2Trim.trimState);
  else if(MATCH_P(keyBuff[1], key_CommonTrim))
    Model.X2Trim.commonTrim = getFixedPointVal10(valueBuff);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Y2Trim()
{
  if(MATCH_P(keyBuff[1], key_TrimState))
    findIdInIdStr(enum_TrimState, valueBuff, Model.Y2Trim.trimState);
  else if(MATCH_P(keyBuff[1], key_CommonTrim))
    Model.Y2Trim.commonTrim = getFixedPointVal10(valueBuff);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_TrimStep()
{
  findIdInIdStr(enum_TrimStep, valueBuff, Model.trimStep);
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
  else
    hasEncounteredInvalidParam = true;
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
  else
    hasEncounteredInvalidParam = true;
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
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_ThrottleCurve()
{
  custom_curve_t* crv = &Model.ThrottleCurve;
  if(MATCH_P(keyBuff[1], key_NumPoints))
  {
    crv->numPoints = atoi_with_prefix(valueBuff);
    if(crv->numPoints > MAX_NUM_POINTS_CUSTOM_CURVE)
    {
      crv->numPoints = MAX_NUM_POINTS_CUSTOM_CURVE;
      hasEncounteredInvalidParam = true;
    }
    else if(crv->numPoints < MIN_NUM_POINTS_CUSTOM_CURVE)
    {
      crv->numPoints = MIN_NUM_POINTS_CUSTOM_CURVE;
      hasEncounteredInvalidParam = true;
    }
  }
  else if(MATCH_P(keyBuff[1], key_XVal))
  {
    uint8_t pt = atoi_with_prefix(keyBuff[2]);
    if(pt == 0) 
      crv->xVal[0] = -100;
    else if(pt == crv->numPoints - 1) 
      crv->xVal[crv->numPoints - 1] = 100;
    else if(pt < MAX_NUM_POINTS_CUSTOM_CURVE)
      crv->xVal[pt] = atoi_with_prefix(valueBuff);
    else
      hasEncounteredInvalidParam = true;
  }
  else if(MATCH_P(keyBuff[1], key_YVal))
  {
    uint8_t pt = atoi_with_prefix(keyBuff[2]);
    if(pt >= crv->numPoints)
    {
      pt = crv->numPoints - 1;
      hasEncounteredInvalidParam = true;
    }
    crv->yVal[pt] = atoi_with_prefix(valueBuff);
  }
  else if(MATCH_P(keyBuff[1], key_Smooth))
    readValue_bool(valueBuff, &crv->smooth);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_FunctionGenerators()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_FUNCGEN)
  {
    funcgen_t* fgen = &Model.Funcgen[thisIdx];
    if(MATCH_P(keyBuff[1], key_Waveform))
      findIdInIdStr(enum_FuncgenWaveform, valueBuff, fgen->waveform);
    else if(MATCH_P(keyBuff[1], key_PeriodMode))
      findIdInIdStr(enum_FuncgenPeriodMode, valueBuff, fgen->periodMode);
    else if(MATCH_P(keyBuff[1], key_Period1))
      fgen->period1 = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Period2))
      fgen->period2 = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_ModulatorSrc))
      fgen->modulatorSrc = getSrcId(valueBuff);
    else if(MATCH_P(keyBuff[1], key_ReverseModulator))
      readValue_bool(valueBuff, &fgen->reverseModulator);
    else if(MATCH_P(keyBuff[1], key_PhaseMode))
      findIdInIdStr(enum_FuncgenPhaseMode, valueBuff, fgen->phaseMode);
    else if(MATCH_P(keyBuff[1], key_Phase))
      fgen->phase = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_PhaseAngle)) //deprecated, replaced with key_Phase
      fgen->phase = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_WidthMode))
      findIdInIdStr(enum_FuncgenWidthMode, valueBuff, fgen->widthMode);
    else if(MATCH_P(keyBuff[1], key_Width))
      fgen->width = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Period))
      fgen->period = getFixedPointVal10(valueBuff);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Mixer()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_MIX_SLOTS)
  {
    mixer_params_t* mxr = &Model.Mixer[thisIdx];
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
      {
        uint8_t crvIdx = atoi_with_prefix(valueBuff) - 1;
        if(crvIdx < NUM_CUSTOM_CURVES)
          mxr->curveVal = crvIdx;
      }
    }
    else if(MATCH_P(keyBuff[1], key_TrimEnabled))
      readValue_bool(valueBuff, &mxr->trimEnabled);
    else if(MATCH_P(keyBuff[1], key_FlightMode))
    {
      if(MATCH_P(valueBuff, PSTR("All")))
        mxr->flightMode = 0xff;
      else //example 1,2,5,
      {
        mxr->flightMode = (0xff << NUM_FLIGHT_MODES) & 0xff;
        char *str = valueBuff;
        while(*str)
        {
          if(isDigit(*str))
          {
            uint8_t i = *str - '1';
            if(i < NUM_FLIGHT_MODES)
              mxr->flightMode |= 1 << i;
          }
          str++;
        }
      }
    }
    else if(MATCH_P(keyBuff[1], key_DelayUp))
      mxr->delayUp = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_DelayDown))
      mxr->delayDown = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_SlowUp))
      mxr->slowUp = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_SlowDown))
      mxr->slowDown = getFixedPointVal10(valueBuff);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_CustomCurves()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_CUSTOM_CURVES)
  {
    custom_curve_t* crv = &Model.CustomCurve[thisIdx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(crv->name, valueBuff, sizeof(crv->name));
    else if(MATCH_P(keyBuff[1], key_NumPoints))
    {
      crv->numPoints = atoi_with_prefix(valueBuff);
      if(crv->numPoints > MAX_NUM_POINTS_CUSTOM_CURVE)
      {
        crv->numPoints = MAX_NUM_POINTS_CUSTOM_CURVE;
        hasEncounteredInvalidParam = true;
      }
      else if(crv->numPoints < MIN_NUM_POINTS_CUSTOM_CURVE)
      {
        crv->numPoints = MIN_NUM_POINTS_CUSTOM_CURVE;
        hasEncounteredInvalidParam = true;
      }
    }
    else if(MATCH_P(keyBuff[1], key_XVal))
    {
      uint8_t pt = atoi_with_prefix(keyBuff[2]);
      if(pt == 0) 
        crv->xVal[0] = -100;
      else if(pt == crv->numPoints - 1) 
        crv->xVal[crv->numPoints - 1] = 100;
      else if(pt < MAX_NUM_POINTS_CUSTOM_CURVE)
        crv->xVal[pt] = atoi_with_prefix(valueBuff);
      else
        hasEncounteredInvalidParam = true;
    }
    else if(MATCH_P(keyBuff[1], key_YVal))
    {
      uint8_t pt = atoi_with_prefix(keyBuff[2]);
      if(pt >= crv->numPoints)
      {
        pt = crv->numPoints - 1;
        hasEncounteredInvalidParam = true;
      }
      crv->yVal[pt] = atoi_with_prefix(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_Smooth))
      readValue_bool(valueBuff, &crv->smooth);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_LogicalSwitches()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_LOGICAL_SWITCHES)
  {
    logical_switch_t* ls = &Model.LogicalSwitch[thisIdx];
    if(MATCH_P(keyBuff[1], key_Func))
      findIdInIdStr(enum_LogicalSwitch_Func, valueBuff, ls->func);
    else if(MATCH_P(keyBuff[1], key_Val1))
    {
      if(ls->func <= LS_FUNC_GROUP3_LAST)
        ls->val1 = getSrcId(valueBuff);
      else if(ls->func <= LS_FUNC_GROUP4_LAST)
        ls->val1 = getSrcId(valueBuff);
      else if(ls->func <= LS_FUNC_GROUP5_LAST)
        ls->val1 = getControlSwitchID(valueBuff);
      else if(ls->func == LS_FUNC_TOGGLE)
        ls->val1 = getControlSwitchID(valueBuff);
      else if(ls->func == LS_FUNC_PULSE)
        ls->val1 = getFixedPointVal10(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_Val2))
    {
      if(ls->func <= LS_FUNC_GROUP3_LAST)
        ls->val2 = atoi_with_prefix(valueBuff);
      else if(ls->func <= LS_FUNC_GROUP4_LAST)
        ls->val2 = getSrcId(valueBuff);
      else if(ls->func <= LS_FUNC_GROUP5_LAST)
        ls->val2 = getControlSwitchID(valueBuff);
      else if(ls->func == LS_FUNC_TOGGLE)
        findIdInIdStr(enum_ClockEdge, valueBuff, ls->val2);
      else if(ls->func == LS_FUNC_PULSE)
        ls->val2 = getFixedPointVal10(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_Val3))
    {
      if(ls->func == LS_FUNC_TOGGLE)
        ls->val3 = getControlSwitchID(valueBuff);
      else if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
        findIdInIdStr(enum_DirectionOfChange, valueBuff, ls->val3);
      else
        ls->val3 = getFixedPointVal10(valueBuff);
    }
    else if(MATCH_P(keyBuff[1], key_Val4))
    {
      ls->val4 = getFixedPointVal10(valueBuff);
    }
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Counters()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_COUNTERS)
  {
    counter_params_t* counter = &Model.Counter[thisIdx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(counter->name, valueBuff, sizeof(counter->name));
    else if(MATCH_P(keyBuff[1], key_Type))
      findIdInIdStr(enum_CounterType, valueBuff, counter->type);
    else if(MATCH_P(keyBuff[1], key_Clock))
      counter->clock = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_IncrementClock))
      counter->incrementClock = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_DecrementClock))
      counter->decrementClock = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Edge))
      findIdInIdStr(enum_ClockEdge, valueBuff, counter->edge);
    else if(MATCH_P(keyBuff[1], key_IncrementEdge))
      findIdInIdStr(enum_ClockEdge, valueBuff, counter->incrementEdge);
    else if(MATCH_P(keyBuff[1], key_DecrementEdge))
      findIdInIdStr(enum_ClockEdge, valueBuff, counter->decrementEdge);
    else if(MATCH_P(keyBuff[1], key_Clear))
      counter->clear = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Modulus))
      counter->modulus = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Direction))
      findIdInIdStr(enum_CounterDirection, valueBuff, counter->direction);
    else if(MATCH_P(keyBuff[1], key_RolloverEnabled))
      readValue_bool(valueBuff, &counter->rolloverEnabled);
    else if(MATCH_P(keyBuff[1], key_IsPersistent))
      readValue_bool(valueBuff, &counter->isPersistent);
    else if(MATCH_P(keyBuff[1], key_PersistVal))
      counter->persistVal = atoi_with_prefix(valueBuff);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_FlightModes()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_FLIGHT_MODES)
  {
    flight_mode_t* fmd = &Model.FlightMode[thisIdx];
    if(MATCH_P(keyBuff[1], key_Name))
      strlcpy(fmd->name, valueBuff, sizeof(fmd->name));
    else if(MATCH_P(keyBuff[1], key_Switch))
      fmd->swtch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_X1Trim))
      fmd->x1Trim = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Y1Trim))
      fmd->y1Trim = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_X2Trim))
      fmd->x2Trim = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Y2Trim))
      fmd->y2Trim = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_TransitionTime))
      fmd->transitionTime = getFixedPointVal10(valueBuff);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Channels()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_RC_CHANNELS)
  {
    channel_params_t* ch = &Model.Channel[thisIdx];
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
      {
        uint8_t crvIdx = atoi_with_prefix(valueBuff) - 1;
        if(crvIdx < NUM_CUSTOM_CURVES)
          ch->curve = crvIdx;
      }
    }
    else if(MATCH_P(keyBuff[1], key_Reverse))
      readValue_bool(valueBuff, &ch->reverse);
    else if(MATCH_P(keyBuff[1], key_Subtrim))
      ch->subtrim = getFixedPointVal10(valueBuff);
    else if(MATCH_P(keyBuff[1], key_OverrideSwitch))
      ch->overrideSwitch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_OverrideVal))
      ch->overrideVal = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Failsafe))
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
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Widgets()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_WIDGETS)
  {
    widget_params_t* widget = &Model.Widget[thisIdx];
    if(MATCH_P(keyBuff[1], key_Type))
      findIdInIdStr(enum_WidgetType, valueBuff, widget->type);
    else if(MATCH_P(keyBuff[1], key_Src))
    {
      if(widget->type == WIDGET_TYPE_MIXSOURCES)
        widget->src = getSrcId(valueBuff);
      else if(widget->type == WIDGET_TYPE_COUNTERS)
      {
        uint8_t counterIdx = atoi_with_prefix(valueBuff) - 1;
        if(counterIdx < NUM_COUNTERS)
          widget->src = counterIdx;
      }
      else if(widget->type == WIDGET_TYPE_TIMERS)
      {
        uint8_t timerIdx = atoi_with_prefix(valueBuff) - 1;
        if(timerIdx < NUM_TIMERS)
          widget->src = timerIdx;
      }
      else if(widget->type == WIDGET_TYPE_OUTPUTS)
      {
        uint8_t chIdx = atoi_with_prefix(valueBuff) - 1;
        if(chIdx < NUM_RC_CHANNELS)
          widget->src = chIdx;
      }
      else if(widget->type == WIDGET_TYPE_TELEMETRY)
      {
        char tempBuff[MAX_STR_SIZE];
        tempBuff[0] = '\0';
        strlcpy(tempBuff, findStringInIdStr(enum_WidgetSource, WIDGET_SRC_AUTO), sizeof(tempBuff));
        if(MATCH(tempBuff, valueBuff))
          widget->src = WIDGET_SRC_AUTO;
        else
        {
          uint8_t tlmIdx = getSrcId(valueBuff) - SRC_TELEMETRY_FIRST;
          if(tlmIdx < NUM_CUSTOM_TELEMETRY)
            widget->src = tlmIdx;
        }
      }
    }
    else if(MATCH_P(keyBuff[1], key_Disp))
      findIdInIdStr(enum_WidgetDisplay, valueBuff, widget->disp);
    else if(MATCH_P(keyBuff[1], key_GaugeMin))
      widget->gaugeMin = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_GaugeMax))
      widget->gaugeMax = atoi_with_prefix(valueBuff);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Notifications()
{
  if(MATCH_P(keyBuff[1], key_Number))
    thisIdx = atoi_with_prefix(valueBuff) - 1;
  else if(thisIdx < NUM_CUSTOM_NOTIFICATIONS)
  {
    notification_params_t *notification = &Model.CustomNotification[thisIdx];
    if(MATCH_P(keyBuff[1], key_Enabled))
      readValue_bool(valueBuff, &notification->enabled);
    else if(MATCH_P(keyBuff[1], key_Text))
      strlcpy(notification->text, valueBuff, sizeof(notification->text));
    else if(MATCH_P(keyBuff[1], key_Switch))
      notification->swtch = getControlSwitchID(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Tone))
    {
      notification->tone = AUDIO_NOTIFICATION_TONE_FIRST + atoi_with_prefix(valueBuff) - 1;
      if(notification->tone < AUDIO_NOTIFICATION_TONE_FIRST) notification->tone = AUDIO_NOTIFICATION_TONE_FIRST;
      if(notification->tone > AUDIO_NOTIFICATION_TONE_LAST) notification->tone = AUDIO_NOTIFICATION_TONE_LAST;
    }
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

//--- system related --- 

void extractConfig_Sticks()
{
  if(MATCH_P(keyBuff[1], key_Name))
    thisIdx = getSrcId(valueBuff) - SRC_STICK_AXIS_FIRST;
  else if(thisIdx < NUM_STICK_AXES)
  {
    if(MATCH_P(keyBuff[1], key_MinVal))
      Sys.StickAxis[thisIdx].minVal = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_CenterVal))
      Sys.StickAxis[thisIdx].centerVal = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_MaxVal))
      Sys.StickAxis[thisIdx].maxVal = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Deadzone))
      Sys.StickAxis[thisIdx].deadzone = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Type))
      findIdInIdStr(enum_StickAxisType, valueBuff, Sys.StickAxis[thisIdx].type);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_StickMode()
{
  findIdInIdStr(enum_StickMode, valueBuff, Sys.defaultStickMode);
}

void extractConfig_Knobs()
{
  if(MATCH_P(keyBuff[1], key_Name))
    thisIdx = getSrcId(valueBuff) - SRC_KNOB_FIRST;
  else if(thisIdx < NUM_KNOBS)
  {
    if(MATCH_P(keyBuff[1], key_MinVal))
      Sys.Knob[thisIdx].minVal = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_CenterVal))
      Sys.Knob[thisIdx].centerVal = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_MaxVal))
      Sys.Knob[thisIdx].maxVal = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Deadzone))
      Sys.Knob[thisIdx].deadzone = atoi_with_prefix(valueBuff);
    else if(MATCH_P(keyBuff[1], key_Type))
      findIdInIdStr(enum_KnobType, valueBuff, Sys.Knob[thisIdx].type);
    else
      hasEncounteredInvalidParam = true;
  }
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Switches()
{
  thisIdx = getSrcId(keyBuff[1]) - SRC_SW_PHYSICAL_FIRST;
  if(thisIdx < NUM_PHYSICAL_SWITCHES)
    findIdInIdStr(enum_SwitchType, valueBuff, Sys.swType[thisIdx]);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Battery()
{
  if(MATCH_P(keyBuff[1], key_VoltsMin))
    Sys.batteryVoltsMin = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_VoltsMax))
    Sys.batteryVoltsMax = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_CalibrationFactor))
    Sys.batteryCalibrationFactor = atoi_with_prefix(valueBuff);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Security()
{
  if(MATCH_P(keyBuff[1], key_LockStartup))
    readValue_bool(valueBuff, &Sys.lockStartup);
  else if(MATCH_P(keyBuff[1], key_LockModels))
    readValue_bool(valueBuff, &Sys.lockModels);
  else if(MATCH_P(keyBuff[1], key_LockOnInactivity))
    readValue_bool(valueBuff, &Sys.lockOnInactivity);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_RF()
{
  if(MATCH_P(keyBuff[1], key_Power))
    findIdInIdStr(enum_RFpower, valueBuff, Sys.rfPower);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Sound()
{
  if(MATCH_P(keyBuff[1], key_Enabled))
    readValue_bool(valueBuff, &Sys.soundEnabled);
  else if(MATCH_P(keyBuff[1], key_Inactivity))
    readValue_bool(valueBuff, &Sys.soundOnInactivity);
  else if(MATCH_P(keyBuff[1], key_Switches))
    readValue_bool(valueBuff, &Sys.soundSwitches);
  else if(MATCH_P(keyBuff[1], key_KnobCenter))
    readValue_bool(valueBuff, &Sys.soundKnobCenter);
  else if(MATCH_P(keyBuff[1], key_Keys))
    readValue_bool(valueBuff, &Sys.soundKeys);
  else if(MATCH_P(keyBuff[1], key_Trims))
    readValue_bool(valueBuff, &Sys.soundTrims);
  else if(MATCH_P(keyBuff[1], key_TrimToneFreqMode))
    findIdInIdStr(enum_TrimToneFreqMode, valueBuff, Sys.trimToneFreqMode);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Backlight()
{
  if(MATCH_P(keyBuff[1], key_Enabled))
    readValue_bool(valueBuff, &Sys.backlightEnabled);
  else if(MATCH_P(keyBuff[1], key_Brightness))
    Sys.backlightBrightness = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_Timeout))
    findIdInIdStr(enum_BacklightTimeout, valueBuff, Sys.backlightTimeout);
  else if(MATCH_P(keyBuff[1], key_Wakeup))
    findIdInIdStr(enum_BacklightWakeup, valueBuff, Sys.backlightWakeup);
  else if(MATCH_P(keyBuff[1], key_SuppressFirstKey))
    readValue_bool(valueBuff, &Sys.backlightSuppressFirstKey);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Contrast()
{
  Sys.contrast = atoi_with_prefix(valueBuff);
}

void extractConfig_Appearance()
{
  if(MATCH_P(keyBuff[1], key_ShowMenuIcons))
    readValue_bool(valueBuff, &Sys.showMenuIcons);
  else if(MATCH_P(keyBuff[1], key_RememberMenuPosition))
    readValue_bool(valueBuff, &Sys.rememberMenuPosition);
  else if(MATCH_P(keyBuff[1], key_UseRoundRect))
    readValue_bool(valueBuff, &Sys.useRoundRect);
  else if(MATCH_P(keyBuff[1], key_UseDenserMenus))
    readValue_bool(valueBuff, &Sys.useDenserMenus);
  else if(MATCH_P(keyBuff[1], key_AnimationsEnabled))
    readValue_bool(valueBuff, &Sys.animationsEnabled);
  else if(MATCH_P(keyBuff[1], key_AutohideTrims))
    readValue_bool(valueBuff, &Sys.autohideTrims);
  else if(MATCH_P(keyBuff[1], key_NumericalBatteryIndicator))
    readValue_bool(valueBuff, &Sys.useNumericalBatteryIndicator);
  else if(MATCH_P(keyBuff[1], key_ShowSplashScreen))
    readValue_bool(valueBuff, &Sys.showSplashScreen);
  else if(MATCH_P(keyBuff[1], key_ShowWelcomeMessage))
    readValue_bool(valueBuff, &Sys.showWelcomeMessage);
  else if(MATCH_P(keyBuff[1], key_AlwaysShowHours))
    readValue_bool(valueBuff, &Sys.alwaysShowHours);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Miscellaneous()
{
  if(MATCH_P(keyBuff[1], key_AutoSelectMovedControl))
    readValue_bool(valueBuff, &Sys.autoSelectMovedControl);
  else if(MATCH_P(keyBuff[1], key_MixerTemplatesEnabled))
    readValue_bool(valueBuff, &Sys.mixerTemplatesEnabled);
  else if(MATCH_P(keyBuff[1], key_DefaultChannelOrder))
  {
    uint8_t *val = &Sys.defaultChannelOrder;
    bool found = false;
    char tmpStr[5];
    for(*val = 0; *val < 24; (*val)++)
    {
      //make the string for the channel order and compare with the valueBuff
      tmpStr[getChannelIdx('A')] = 'A';
      tmpStr[getChannelIdx('E')] = 'E';
      tmpStr[getChannelIdx('T')] = 'T';
      tmpStr[getChannelIdx('R')] = 'R';
      tmpStr[4] = '\0';
      if(MATCH(tmpStr, valueBuff))
      {
        found = true;
        break;
      }
    }
    if(!found)
    {
      Sys.defaultChannelOrder = 0;
      hasEncounteredInvalidParam = true;
    }
  }
  else if(MATCH_P(keyBuff[1], key_InactivityMinutes))
    Sys.inactivityMinutes = atoi_with_prefix(valueBuff);
  else if(MATCH_P(keyBuff[1], key_MixerCurvePreview))
    readValue_bool(valueBuff, &Sys.showCurvePreviewInMixer);
  else if(MATCH_P(keyBuff[1], key_DefaultGnssUnits))
    findIdInIdStr(enum_DefaultGNSSUnits, valueBuff, Sys.defaultGnssUnits);
  else if(MATCH_P(keyBuff[1], key_CustomGnssDistanceUnits))
    findIdInIdStr(enum_DisplayedUnits, valueBuff, Sys.customGnssDistanceUnits);
  else if(MATCH_P(keyBuff[1], key_CustomGnssSpeedUnits))
    findIdInIdStr(enum_DisplayedUnits, valueBuff, Sys.customGnssSpeedUnits);
  else if(MATCH_P(keyBuff[1], key_CustomGnssAltitudeUnits))
    findIdInIdStr(enum_DisplayedUnits, valueBuff, Sys.customGnssAltitudeUnits);
  else
    hasEncounteredInvalidParam = true;
}

void extractConfig_Debug()
{
  if(MATCH_P(keyBuff[1], key_ShowLoopTime))
    readValue_bool(valueBuff, &Sys.DBG_showLoopTime);
  else if(MATCH_P(keyBuff[1], key_SimulateTelemetry))
    readValue_bool(valueBuff, &Sys.DBG_simulateTelemetry);
  else if(MATCH_P(keyBuff[1], key_DisableInterlacing))
    readValue_bool(valueBuff, &Sys.DBG_disableInterlacing);
  else if(MATCH_P(keyBuff[1], key_ScreenshotSeqNo))
    Sys.screenshotSeqNo = atoi_with_prefix(valueBuff);
  else
    hasEncounteredInvalidParam = true;
}

//============================ Model data importer =================================================

void importModelData(File& file)
{
  // Calls the parser and then the extractor functions on the corresponding root item. 
  // The extractor functions have no control over the parser. They neither call it to fetch the next line,
  // nor modify the key buffers used by the parser.
  
  //--- Initialise
  isEndOfFile = false;
  hasEncounteredInvalidParam = false;
  idNotFoundInIdStr = false;
  dbgLineNumber = 0;
  dbgFirstErrorLineNumber = 0;
  dbgTotalErrorLines = 0;
  memset(keyBuff[0], 0, sizeof(keyBuff[0]));
  memset(keyBuff[1], 0, sizeof(keyBuff[0]));
  memset(keyBuff[2], 0, sizeof(keyBuff[0]));
  memset(valueBuff, 0, sizeof(valueBuff));

  //--- Loop 
  while(!isEndOfFile)
  {
    //clear value buffer so we don't work with stale values
    *valueBuff = '\0';

    //call parser
    parser(file);

    //if value is empty, continue parsing
    if(*valueBuff == '\0')
      continue;
    
    //call the appropriate extractor function, depending on the root key
    if(MATCH_P(keyBuff[0], key_ModelName)) extractConfig_ModelName();
    else if(MATCH_P(keyBuff[0], key_ModelType)) extractConfig_ModelType();
    else if(MATCH_P(keyBuff[0], key_SecondaryRcvrEnabled)) extractConfig_SecondaryRcvrEnabled();
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
    else if(MATCH_P(keyBuff[0], key_GnssDistanceUnits)) extractConfig_GnssDistanceUnits();
    else if(MATCH_P(keyBuff[0], key_GnssSpeedUnits)) extractConfig_GnssSpeedUnits();
    else if(MATCH_P(keyBuff[0], key_GnssAltitudeUnits)) extractConfig_GnssAltitudeUnits();
    else if(MATCH_P(keyBuff[0], key_Timer)) extractConfig_Timers();
    else if(MATCH_P(keyBuff[0], key_X1Trim)) extractConfig_X1Trim();
    else if(MATCH_P(keyBuff[0], key_Y1Trim)) extractConfig_Y1Trim();
    else if(MATCH_P(keyBuff[0], key_X2Trim)) extractConfig_X2Trim();
    else if(MATCH_P(keyBuff[0], key_Y2Trim)) extractConfig_Y2Trim();
    else if(MATCH_P(keyBuff[0], key_TrimStep)) extractConfig_TrimStep();
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
    else if(MATCH_P(keyBuff[0], key_Notification)) extractConfig_Notifications();
    else 
      hasEncounteredInvalidParam = true;
    
    if((hasEncounteredInvalidParam || idNotFoundInIdStr))
    {
      dbgTotalErrorLines++;
      if(dbgFirstErrorLineNumber == 0)
        dbgFirstErrorLineNumber = dbgLineNumber;
      //reset flags
      hasEncounteredInvalidParam = false;
      idNotFoundInIdStr = false;
    }
  }

}

//============================ System data importer ===============================================

void importSystemData(File& file)
{
  // Calls the parser and then the extractor functions on the corresponding root item. 
  // The extractor functions have no control over the parser. They neither call it to fetch the next line,
  // nor modify the key buffers used by the parser.
  
  //--- Initialise
  isEndOfFile = false;
  hasEncounteredInvalidParam = false;
  idNotFoundInIdStr = false;
  dbgLineNumber = 0;
  dbgFirstErrorLineNumber = 0;
  dbgTotalErrorLines = 0;
  memset(keyBuff[0], 0, sizeof(keyBuff[0]));
  memset(keyBuff[1], 0, sizeof(keyBuff[0]));
  memset(keyBuff[2], 0, sizeof(keyBuff[0]));
  memset(valueBuff, 0, sizeof(valueBuff));

  //--- Loop 
  while(!isEndOfFile)
  {
    //clear value buffer so we don't work with stale values
    *valueBuff = '\0';

    //call parser
    parser(file);

    //if value is empty, continue parsing
    if(*valueBuff == '\0')
      continue;
    
    //call the appropriate extractor function, depending on the root key
    if(MATCH_P(keyBuff[0], key_StickAxis)) extractConfig_Sticks();
    else if(MATCH_P(keyBuff[0], key_StickMode)) extractConfig_StickMode();
    else if(MATCH_P(keyBuff[0], key_Knob)) extractConfig_Knobs();
    else if(MATCH_P(keyBuff[0], key_Switches)) extractConfig_Switches();
    else if(MATCH_P(keyBuff[0], key_Battery)) extractConfig_Battery();
    else if(MATCH_P(keyBuff[0], key_Security)) extractConfig_Security();
    else if(MATCH_P(keyBuff[0], key_RF)) extractConfig_RF();
    else if(MATCH_P(keyBuff[0], key_Sound)) extractConfig_Sound();
    else if(MATCH_P(keyBuff[0], key_Backlight)) extractConfig_Backlight();
    else if(MATCH_P(keyBuff[0], key_Contrast)) extractConfig_Contrast();
    else if(MATCH_P(keyBuff[0], key_Appearance)) extractConfig_Appearance();
    else if(MATCH_P(keyBuff[0], key_Miscellaneous)) extractConfig_Miscellaneous();
    else if(MATCH_P(keyBuff[0], key_Debug)) extractConfig_Debug();
    else 
      hasEncounteredInvalidParam = true;
    
    if((hasEncounteredInvalidParam || idNotFoundInIdStr))
    {
      dbgTotalErrorLines++;
      if(dbgFirstErrorLineNumber == 0)
        dbgFirstErrorLineNumber = dbgLineNumber;
      //reset flags
      hasEncounteredInvalidParam = false;
      idNotFoundInIdStr = false;
    }
  }
}
