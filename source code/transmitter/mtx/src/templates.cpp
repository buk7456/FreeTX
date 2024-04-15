#include "Arduino.h"

#include "common.h"
#include "templates.h"
#include "mixer.h"

//---------------------------- MIXER TEMPLATES -----------------------------------------------------

void loadMix(
  uint8_t idx, const char* name, uint8_t output, uint8_t operation,uint8_t sw,
  uint8_t input, int8_t  weight, int8_t  offset, uint8_t curveType, int8_t  curveVal,
  bool trimEnabled, uint8_t flightMode, 
  uint8_t delayUp, uint8_t delayDown,uint8_t slowUp, uint8_t slowDown)
{
  if(idx >= NUM_MIXSLOTS)
    return;
  
  if(name != NULL)
    strlcpy_P(Model.Mixer[idx].name, name, sizeof(Model.Mixer[0].name));
  else
    Model.Mixer[idx].name[0] = '\0';
  
  Model.Mixer[idx].output      = output;
  Model.Mixer[idx].swtch       = sw;
  Model.Mixer[idx].operation   = operation;
  Model.Mixer[idx].input       = input;
  Model.Mixer[idx].weight      = weight;
  Model.Mixer[idx].offset      = offset;
  Model.Mixer[idx].curveType   = curveType;
  Model.Mixer[idx].curveVal    = curveVal;
  Model.Mixer[idx].trimEnabled = trimEnabled;
  Model.Mixer[idx].flightMode  = flightMode;
  Model.Mixer[idx].delayUp     = delayUp;
  Model.Mixer[idx].delayDown   = delayDown;
  Model.Mixer[idx].slowUp      = slowUp;
  Model.Mixer[idx].slowDown    = slowDown;
}

uint8_t getChannelIdx(char c)
{
  //c is any of 'A', 'E', 'R', 'T'
  
  //---Look up table.
  // A - bits 7,6
  // E - bits 5,4
  // T - bits 3,2
  // R - bits 1,0
  //For example for order AETR, we have 00011011 which is hex 1B.
  //There are 4P4 = 4! = 24 possible arrangements.
  
  static const uint8_t lut[] PROGMEM = {
    0x1B, 0x1E, 0x39, 0x2D, 0x36, 0x27,
    0x4B, 0x4E, 0xC9, 0x8D, 0xC6, 0x87,
    0xD8, 0x9C, 0x78, 0x6C, 0xB4, 0xE4,
    0x93, 0xD2, 0xB1, 0xE1, 0x72, 0x63 
  };
  
  uint8_t result = 0;
  switch (c)
  {
    case 'A': result = pgm_read_byte(&lut[Sys.defaultChannelOrder]) >> 6 & 0x03; break;
    case 'E': result = pgm_read_byte(&lut[Sys.defaultChannelOrder]) >> 4 & 0x03; break;  
    case 'T': result = pgm_read_byte(&lut[Sys.defaultChannelOrder]) >> 2 & 0x03; break;  
    case 'R': result = pgm_read_byte(&lut[Sys.defaultChannelOrder])      & 0x03; break;
  }
  return result;
}

void loadMixerTemplateBasic(uint8_t mixIdx)
{
  loadMix(mixIdx,     NULL,   SRC_CH1 + getChannelIdx('A'), MIX_ADD, CTRL_SW_NONE, SRC_AIL,  100, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  if(Model.type == MODEL_TYPE_AIRPLANE)
    loadMix(mixIdx + 1, NULL, SRC_CH1 + getChannelIdx('E'), MIX_ADD, CTRL_SW_NONE, SRC_ELE, -100, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  else
    loadMix(mixIdx + 1, NULL, SRC_CH1 + getChannelIdx('E'), MIX_ADD, CTRL_SW_NONE, SRC_ELE,  100, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 2, NULL,   SRC_CH1 + getChannelIdx('T'), MIX_ADD, CTRL_SW_NONE, SRC_THR,  100, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 3, NULL,   SRC_CH1 + getChannelIdx('R'), MIX_ADD, CTRL_SW_NONE, SRC_RUD,  100, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  
  //Sort the mixes we have just loaded by their output in ascending order
  //using bubble sort
  uint8_t i, j,  n = 4;
  for(i = 0; i < n-1; i++)
  {
    for(j = 0; j < n-i-1; j++)
    {
      if(Model.Mixer[mixIdx+j].output > Model.Mixer[mixIdx+j+1].output)
        swapMix(mixIdx+j, mixIdx+j+1);
    }
  }
}

void loadMixerTemplateElevon(uint8_t mixIdx)
{
  loadMix(mixIdx,     PSTR("ElvonL"), SRC_CH1 + getChannelIdx('A'), MIX_ADD, CTRL_SW_NONE, SRC_AIL, -50, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 1, PSTR("ElvonL"), SRC_CH1 + getChannelIdx('A'), MIX_ADD, CTRL_SW_NONE, SRC_ELE, -50, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 2, PSTR("ElvonR"), SRC_CH1 + getChannelIdx('E'), MIX_ADD, CTRL_SW_NONE, SRC_AIL,  50, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 3, PSTR("ElvonR"), SRC_CH1 + getChannelIdx('E'), MIX_ADD, CTRL_SW_NONE, SRC_ELE, -50, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
}

void loadMixerTemplateVtail(uint8_t mixIdx)
{
  loadMix(mixIdx,     PSTR("VtailL"), SRC_CH1 + getChannelIdx('R'), MIX_ADD, CTRL_SW_NONE, SRC_RUD,  50, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 1, PSTR("VtailL"), SRC_CH1 + getChannelIdx('R'), MIX_ADD, CTRL_SW_NONE, SRC_ELE, -50, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 2, PSTR("VtailR"), SRC_CH1 + getChannelIdx('E'), MIX_ADD, CTRL_SW_NONE, SRC_RUD, -50, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 3, PSTR("VtailR"), SRC_CH1 + getChannelIdx('E'), MIX_ADD, CTRL_SW_NONE, SRC_ELE, -50, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
}

void loadMixerTemplateDiffThrust(uint8_t mixIdx)
{
  loadMix(mixIdx,     PSTR("MotorL"), SRC_CH1 + getChannelIdx('T'), MIX_ADD, CTRL_SW_NONE, SRC_THR, 100, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 1, PSTR("MotorL"), SRC_CH1 + getChannelIdx('T'), MIX_ADD, CTRL_SW_NONE, SRC_X1, 40, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 2, PSTR("MotorR"), SRC_CH1 + 5,                  MIX_ADD, CTRL_SW_NONE, SRC_THR, 100, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
  loadMix(mixIdx + 3, PSTR("MotorR"), SRC_CH1 + 5,                  MIX_ADD, CTRL_SW_NONE, SRC_X1, -40, 0, 0, 0, 1, 0xFF, 0, 0, 0, 0);
}

//---------------------------- TELEMETRY SENSOR TEMPLATES ------------------------------------------

void loadTelemetryParams( 
  uint8_t idx, const char* name, const char* unitsName, uint8_t identifier,
  int16_t multiplier, int8_t factor10, int16_t offset,       
  bool logEnabled,  uint8_t alarmCondition, int16_t  alarmThreshold,
  uint8_t alarmMelody, bool showOnHome, bool recordMaximum, bool recordMinimum )
{
  if(idx >= NUM_CUSTOM_TELEMETRY)
    return;
  
  if(name != NULL)
    strlcpy_P(Model.Telemetry[idx].name, name, sizeof(Model.Telemetry[0].name));
  
  if(unitsName != NULL)
    strlcpy_P(Model.Telemetry[idx].unitsName, unitsName, sizeof(Model.Telemetry[0].unitsName));
  else
    Model.Telemetry[idx].unitsName[0] = '\0';

  Model.Telemetry[idx].identifier     = identifier;
  Model.Telemetry[idx].multiplier     = multiplier;
  Model.Telemetry[idx].factor10       = factor10;
  Model.Telemetry[idx].offset         = offset;
  Model.Telemetry[idx].logEnabled     = logEnabled;
  Model.Telemetry[idx].alarmCondition = alarmCondition;
  Model.Telemetry[idx].alarmThreshold = alarmThreshold;
  Model.Telemetry[idx].alarmMelody    = alarmMelody; 
  Model.Telemetry[idx].showOnHome     = showOnHome; 
  Model.Telemetry[idx].recordMaximum  = recordMaximum; 
  Model.Telemetry[idx].recordMinimum  = recordMinimum; 
}

void loadSensorTemplateExtVolts2S(uint8_t telemIdx)
{
  loadTelemetryParams(telemIdx, PSTR("ExtVolts"), PSTR("V"), 0x01, 100, -2, 0, true, 
                      TELEMETRY_ALARM_CONDITION_LESS_THAN, 710, 0, true, true, true);
}

void loadSensorTemplateExtVolts3S(uint8_t telemIdx)
{
  loadTelemetryParams(telemIdx, PSTR("ExtVolts"), PSTR("V"), 0x01, 100, -2, 0, true, 
                      TELEMETRY_ALARM_CONDITION_LESS_THAN, 1065, 0, true, true, true);
}

void loadSensorTemplateExtVolts4S(uint8_t telemIdx)
{
  loadTelemetryParams(telemIdx, PSTR("ExtVolts"), PSTR("V"), 0x01, 100, -2, 0, true, 
                      TELEMETRY_ALARM_CONDITION_LESS_THAN, 1420, 0, true, true, true);
}

void loadSensorTemplateRSSI(uint8_t telemIdx)
{
  loadTelemetryParams(telemIdx, PSTR("RSSI"), PSTR("dBm"), 0x7F, 100, 0, 0, true, 
                     TELEMETRY_ALARM_CONDITION_NONE, 0, 0, true, true, true);
}

void loadSensorTemplateLinkQuality(uint8_t telemIdx)
{
  loadTelemetryParams(telemIdx, PSTR("LinkQlty"), PSTR("%"), 0x70, 100, 0, 0, true, 
                     TELEMETRY_ALARM_CONDITION_NONE, 0, 0, true, true, true);
}
