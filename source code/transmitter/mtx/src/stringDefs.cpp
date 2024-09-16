#include "Arduino.h"

#include "common.h"
#include "stringDefs.h"

bool idNotFoundInIdStr = false;

//---- Strings for the enumerations ----
//Arrays of structs in PROGMEM

//System related

const id_string_t enum_RFpower[] PROGMEM = {
  {RF_POWER_LOW, "Low"},
  {RF_POWER_MEDIUM, "Medium"},
  {RF_POWER_MAX, "Maximum"},
  {0, ""} //indicates end so we omit passing sizeof(enum_RFpower)/sizeof(enum_RFpower[0])
};

const id_string_t enum_BacklightWakeup[] PROGMEM = {
  {BACKLIGHT_WAKEUP_KEYS, "Keys"},
  {BACKLIGHT_WAKEUP_ACTIVITY, "Activity"},
  {0, ""}
};

const id_string_t enum_BacklightTimeout[] PROGMEM = {
  {BACKLIGHT_TIMEOUT_5SEC, "5s"},
  {BACKLIGHT_TIMEOUT_15SEC, "15s"},
  {BACKLIGHT_TIMEOUT_1MIN, "1min"},
  {BACKLIGHT_TIMEOUT_2MIN, "2min"},
  {BACKLIGHT_TIMEOUT_5MIN, "5min"},
  {BACKLIGHT_TIMEOUT_10MIN, "10min"},
  {BACKLIGHT_TIMEOUT_30MIN, "30min"},
  {BACKLIGHT_TIMEOUT_NEVER, "Never"},
  {0, ""}
};

const id_string_t enum_TrimToneFreqMode[] PROGMEM = {
  {TRIM_TONE_FREQ_FIXED, "Fixed"},
  {TRIM_TONE_FREQ_VARIABLE, "Variable"},
  {0, ""}
};

const id_string_t enum_SwitchType[] PROGMEM = {
  {SW_ABSENT, "Absent"},
  {SW_2POS, "2pos"},
  {SW_3POS, "3pos"},
  {0, ""}
};

const id_string_t enum_StickMode[] PROGMEM = {
  {STICK_MODE_RTAE, "RTAE"},
  {STICK_MODE_AERT, "AERT"},
  {STICK_MODE_REAT, "REAT"},
  {STICK_MODE_ATRE, "ATRE"},
  {0, ""}
};

const id_string_t enum_StickAxisType[] PROGMEM = {
  {STICK_AXIS_ABSENT, "Absent"},
  {STICK_AXIS_SELF_CENTERING, "Self-centering"},
  {STICK_AXIS_NON_CENTERING, "Non-centering"},
  {0, ""}
};

const id_string_t enum_StickAxisName[] PROGMEM = {
  {SRC_X1_AXIS, "X1"},
  {SRC_Y1_AXIS, "Y1"},
  {SRC_Z1_AXIS, "Z1"},
  {SRC_X2_AXIS, "X2"},
  {SRC_Y2_AXIS, "Y2"},
  {SRC_Z2_AXIS, "Z2"},
  {SRC_X3_AXIS, "X3"},
  {SRC_Y3_AXIS, "Y3"},
  {SRC_X4_AXIS, "X4"},
  {SRC_Y4_AXIS, "Y4"},
  {0, ""}
};

const id_string_t enum_KnobType[] PROGMEM = {
  {KNOB_ABSENT, "Absent"},
  {KNOB_CENTER_DETENT, "Center detent"},
  {KNOB_NO_CENTER_DETENT, "No detent"},
  {0, ""}
};

//Model related

const id_string_t enum_ModelType[] PROGMEM = {
  {MODEL_TYPE_AIRPLANE, "Airplane"},
  {MODEL_TYPE_MULTICOPTER, "Multicopter"},
  {MODEL_TYPE_OTHER, "Other"},
  {0, ""} 
};

const id_string_t enum_TrimState[] PROGMEM = {
  {TRIM_DISABLED, "Off"},
  {TRIM_DISABLED, "Disabled"}, //legacy support
  {TRIM_COMMON, "Common"},
  {TRIM_FLIGHT_MODE, "F-mode"},
  {TRIM_FLIGHT_MODE, "FlightMode"}, //legacy support
  {0, ""}
};

const id_string_t enum_TrimStep[] PROGMEM = {
  {TRIM_STEP_COARSE, "Coarse"},
  {TRIM_STEP_MEDIUM, "Medium"},
  {TRIM_STEP_FINE, "Fine"},
  {0, ""}
};

const id_string_t enum_FuncgenWaveform[] PROGMEM = {
  {FUNCGEN_WAVEFORM_SINE, "Sine"},
  {FUNCGEN_WAVEFORM_SQUARE, "Square"},
  {FUNCGEN_WAVEFORM_TRIANGLE, "Triangle"},
  {FUNCGEN_WAVEFORM_SAWTOOTH, "Sawtooth"},
  {FUNCGEN_WAVEFORM_PULSE, "Pulse"},
  {FUNCGEN_WAVEFORM_RANDOM, "Random"},
  {0, ""}
};

const id_string_t enum_FuncgenPeriodMode[] PROGMEM = {
  {FUNCGEN_PERIODMODE_VARIABLE, "Variable"},
  {FUNCGEN_PERIODMODE_FIXED, "Fixed"},
  {0, ""}
};

const id_string_t enum_FuncgenWidthMode[] PROGMEM = {
  {FUNCGEN_PULSE_WIDTH_VARIABLE, "Variable"},
  {FUNCGEN_PULSE_WIDTH_FIXED, "Fixed"},
  {0, ""}
};

const id_string_t enum_FuncgenPhaseMode[] PROGMEM = {
  {FUNCGEN_PHASEMODE_AUTO, "Auto"},
  {FUNCGEN_PHASEMODE_FIXED, "Fixed"},
  {0, ""}
};

const id_string_t enum_CounterDirection[] PROGMEM = {
  {0, "Up"},
  {1, "Down"},
  {0, ""}
};

const id_string_t enum_MixerOperation[] PROGMEM = {
  {MIX_ADD, "Add"},
  {MIX_MULTIPLY, "Mltply"}, //alias
  {MIX_MULTIPLY, "Multiply"},
  {MIX_REPLACE, "RplcW"}, //alias
  {MIX_REPLACE, "Replace"},
  {MIX_HOLD, "Hold"},
  {0, ""}
};

const id_string_t enum_MixerCurveType[] PROGMEM = {
  {MIX_CURVE_TYPE_DIFF, "Diff"},
  {MIX_CURVE_TYPE_EXPO, "Expo"},
  {MIX_CURVE_TYPE_FUNCTION, "Func"},
  {MIX_CURVE_TYPE_CUSTOM, "Custom"},
  {0, ""}
};

const id_string_t enum_MixerCurveType_Func[] PROGMEM = {
  {MIX_CURVE_FUNC_NONE, "None"},
  {MIX_CURVE_FUNC_X_GREATER_THAN_ZERO, "x>0"},
  {MIX_CURVE_FUNC_X_LESS_THAN_ZERO, "x<0"},
  {MIX_CURVE_FUNC_ABS_X, "|x|"},
  {0, ""}
};

const id_string_t enum_LogicalSwitch_Func[] PROGMEM = {
  {LS_FUNC_NONE, "None"},
  {LS_FUNC_A_GREATER_THAN_X, "a>x"},
  {LS_FUNC_A_LESS_THAN_X, "a<x"},
  {LS_FUNC_A_EQUAL_X, "a==x"},
  {LS_FUNC_A_GREATER_THAN_OR_EQUAL_X, "a>=x"},
  {LS_FUNC_A_LESS_THAN_OR_EQUAL_X, "a<=x"},
  {LS_FUNC_ABS_A_GREATER_THAN_X, "|a|>x"},
  {LS_FUNC_ABS_A_LESS_THAN_X, "|a|<x"},
  {LS_FUNC_ABS_A_EQUAL_X, "|a|==x"},
  {LS_FUNC_ABS_A_GREATER_THAN_OR_EQUAL_X, "|a|>=x"},
  {LS_FUNC_ABS_A_LESS_THAN_OR_EQUAL_X, "|a|<=x"},
  {LS_FUNC_ABS_DELTA_GREATER_THAN_X, "|delta|>x"},
  {LS_FUNC_A_GREATER_THAN_B, "a>b"},
  {LS_FUNC_A_LESS_THAN_B, "a<b"},
  {LS_FUNC_A_EQUAL_B, "a==b"},
  {LS_FUNC_A_GREATER_THAN_OR_EQUAL_B, "a>=b"},
  {LS_FUNC_A_LESS_THAN_OR_EQUAL_B, "a<=b"},
  {LS_FUNC_AND, "AND"},
  {LS_FUNC_OR, "OR"},
  {LS_FUNC_XOR, "XOR"},
  {LS_FUNC_LATCH, "Latch"},
  {LS_FUNC_TOGGLE, "Toggle"},
  {LS_FUNC_PULSE, "Pulse"},
  {0, ""}
};

const id_string_t enum_DirectionOfChange[] PROGMEM = {
  {0, "Positive"},
  {1, "Negative"},
  {2, "Both"},
  {0, ""}
};

const id_string_t enum_ChannelFailsafe[] PROGMEM = {
  {-102, "Hold"},
  {-101, "NoPulse"},
  {0, ""}
};

const id_string_t enum_ChannelCurve[] PROGMEM = {
  {-1, "None"},
  {0, ""}
};

const id_string_t enum_TelemetryAlarmCondition[] PROGMEM = {
  {TELEMETRY_ALARM_CONDITION_NONE, "Off"},
  {TELEMETRY_ALARM_CONDITION_GREATER_THAN, ">Threshold"},
  {TELEMETRY_ALARM_CONDITION_LESS_THAN, "<Threshold"},
  {TELEMETRY_ALARM_CONDITION_GREATER_THAN, ">Thresh"}, //legacy support
  {TELEMETRY_ALARM_CONDITION_LESS_THAN, "<Thresh"}, //legacy support
  {0, ""}
};

const id_string_t enum_WidgetType[] PROGMEM = {
  {WIDGET_TYPE_TELEMETRY, "Telemetry"},
  {WIDGET_TYPE_MIXSOURCES, "MixSources"},
  {WIDGET_TYPE_OUTPUTS, "Outputs"},
  {WIDGET_TYPE_TIMERS, "Timers"},
  {WIDGET_TYPE_COUNTERS, "Counters"},
  {0, ""}
};

const id_string_t enum_WidgetSource[] PROGMEM = {
  {WIDGET_SRC_AUTO, "Auto"},
  {0, ""}
};

const id_string_t enum_WidgetDisplay[] PROGMEM = {
  {WIDGET_DISP_NUMERICAL, "Numerical"},
  {WIDGET_DISP_GAUGE, "Gauge"},
  {WIDGET_DISP_GAUGE_ZERO_CENTERED, "CenterGauge"},
  {0, ""}
};

const id_string_t enum_SwitchWarn[] PROGMEM = {
  {0, "Up"},
  {1, "Down"},
  {2, "Mid"},
  {-1, "None"},
  {0, ""}
};

const id_string_t enum_ClockEdge[] PROGMEM = {
  {0, "Rising"},
  {1, "Falling"},
  {2, "Dual"},
  {0, ""}
};

//---- Strings for the keys -----
//Max 20 characters
//Key descriptors should be unique, but can also be shared.
//Repeated keys should be commented out, they are here only for convenience.

const char key_ModelName[] PROGMEM = "ModelName";
const char key_ModelType[] PROGMEM = "ModelType";

const char key_RudSrc[] PROGMEM = "RudSrc";
const char key_ThrSrc[] PROGMEM = "ThrSrc";
const char key_AilSrc[] PROGMEM = "AilSrc";
const char key_EleSrc[] PROGMEM = "EleSrc";

const char key_YawSrc[] PROGMEM = "YawSrc";
// const char key_ThrSrc[] PROGMEM = "ThrSrc";
const char key_RollSrc[] PROGMEM = "RollSrc";
const char key_PitchSrc[] PROGMEM = "PitchSrc";

const char key_Number[] PROGMEM = "No.";

const char key_Timer[] PROGMEM = "Timer";
const char key_ResetSwitch[] PROGMEM = "Reset";
const char key_InitialMinutes[] PROGMEM = "InitMin";
const char key_IsPersistent[] PROGMEM = "Persistent";
const char key_PersistVal[] PROGMEM = "PersistVal";

const char key_X1Trim[] PROGMEM = "X1Trim";
const char key_Y1Trim[] PROGMEM = "Y1Trim";
const char key_X2Trim[] PROGMEM = "X2Trim";
const char key_Y2Trim[] PROGMEM = "Y2Trim";
const char key_TrimState[] PROGMEM = "TrimState";
const char key_CommonTrim[] PROGMEM = "CommonTrim";

const char key_TrimStep[] PROGMEM = "TrimStep";

const char key_RudDualRate[] PROGMEM = "RudDualRate";
const char key_AilDualRate[] PROGMEM = "AilDualRate";
const char key_EleDualRate[] PROGMEM = "EleDualRate";

const char key_YawDualRate[] PROGMEM = "YawDualRate";
const char key_RollDualRate[] PROGMEM = "RollDualRate";
const char key_PitchDualRate[] PROGMEM = "PitchDualRate";

const char key_Rate1[] PROGMEM = "Rate1";
const char key_Rate2[] PROGMEM = "Rate2";
const char key_Expo1[] PROGMEM = "Expo1";
const char key_Expo2[] PROGMEM = "Expo2";
const char key_Switch[] PROGMEM = "Switch";

const char key_ThrottleCurve[] PROGMEM = "ThrottleCurve";
const char key_CustomCurve[] PROGMEM = "CustomCurve";
const char key_Name[] PROGMEM = "Name";
const char key_NumPoints[] PROGMEM = "NumPoints";
const char key_XVal[] PROGMEM = "XVal";
const char key_YVal[] PROGMEM = "YVal";
const char key_Smooth[] PROGMEM = "Smooth";

const char key_Funcgen[] PROGMEM = "Funcgen";
const char key_Waveform[] PROGMEM = "Waveform";
const char key_PeriodMode[] PROGMEM = "PeriodMode";
const char key_Period1[] PROGMEM = "Period1";
const char key_Period2[] PROGMEM = "Period2";
const char key_ModulatorSrc[] PROGMEM = "ModulatorSrc";
const char key_ReverseModulator[] PROGMEM = "ReverseModulator";
const char key_PhaseMode[] PROGMEM = "PhaseMode";
const char key_Phase[] PROGMEM = "Phase";
const char key_WidthMode[] PROGMEM = "WidthMode";
const char key_Width[] PROGMEM = "Width";
const char key_Period[] PROGMEM = "Period";

const char key_Mixer[] PROGMEM = "Mixer";
// const char key_Name[] PROGMEM = "Name";
const char key_Output[] PROGMEM = "Output";
const char key_Operation[] PROGMEM = "Operation";
// const char key_Switch[] PROGMEM = "Switch";
const char key_Input[] PROGMEM = "Input";
const char key_Weight[] PROGMEM = "Weight";
const char key_Offset[] PROGMEM = "Offset";
const char key_CurveType[] PROGMEM = "CurveType";
const char key_CurveVal[] PROGMEM = "CurveVal";
const char key_TrimEnabled[] PROGMEM = "TrimEnabled";
const char key_FlightMode[] PROGMEM = "FlightMode";
const char key_DelayUp[] PROGMEM = "DelayUp";
const char key_DelayDown[] PROGMEM = "DelayDown";
const char key_SlowUp[] PROGMEM = "SlowUp";
const char key_SlowDown[] PROGMEM = "SlowDown";

const char key_LogicalSwitch[] PROGMEM = "LogicalSwitch";
const char key_Func[] PROGMEM = "Func";
const char key_Val1[] PROGMEM = "Val1";
const char key_Val2[] PROGMEM = "Val2";
const char key_Val3[] PROGMEM = "Val3";
const char key_Val4[] PROGMEM = "Val4";

const char key_Counter[] PROGMEM = "Counter";
// const char key_Name[] PROGMEM = "Name";
const char key_Clock[] PROGMEM = "Clock";
const char key_Edge[] PROGMEM = "Edge";
const char key_Clear[] PROGMEM = "Clear";
const char key_Modulus[] PROGMEM = "Modulus";
const char key_Direction[] PROGMEM = "Direction";
// const char key_IsPersistent[] PROGMEM = "Persistent";
// const char key_PersistVal[] PROGMEM = "PersistVal";

// const char key_Name[] PROGMEM = "Name";
// const char key_Switch[] PROGMEM = "Switch";
// const char key_X1Trim[] PROGMEM = "X1Trim";
// const char key_Y1Trim[] PROGMEM = "Y1Trim";
// const char key_X2Trim[] PROGMEM = "X2Trim";
// const char key_Y2Trim[] PROGMEM = "Y2Trim";
const char key_TransitionTime[] PROGMEM = "TransitionTime";

const char key_CheckThrottle[] PROGMEM = "CheckThrottle";
const char key_SwitchWarn[] PROGMEM = "SwitchWarn";

const char key_Channel[] PROGMEM = "Channel";
// const char key_Name[] PROGMEM = "Name";
const char key_Curve[] PROGMEM = "Curve";
const char key_Reverse[] PROGMEM = "Reverse";
const char key_Subtrim[] PROGMEM = "Subtrim";
const char key_OverrideSwitch[] PROGMEM = "OverrideSwitch";
const char key_OverrideVal[] PROGMEM = "OverrideVal";
const char key_Failsafe[] PROGMEM = "Failsafe";
const char key_EndpointL[] PROGMEM = "EndpointL";
const char key_EndpointR[] PROGMEM = "EndpointR";

const char key_Telemetry[] PROGMEM = "Telemetry";
// const char key_Name[] PROGMEM = "Name";
const char key_UnitsName[] PROGMEM = "UnitsName";
const char key_Identifier[] PROGMEM = "Identifier";
const char key_Multiplier[] PROGMEM = "Multiplier";
const char key_Factor10[] PROGMEM = "Factor10";
// const char key_Offset[] PROGMEM = "Offset";
const char key_LogEnabled[] PROGMEM = "LogEnabled";
const char key_AlarmCondition[] PROGMEM = "AlarmCondition";
const char key_AlarmThreshold[] PROGMEM = "AlarmThreshold";
const char key_AlarmMelody[] PROGMEM = "AlarmMelody";
const char key_ShowOnHome[] PROGMEM = "ShowOnHome";
const char key_RecordMaximum[] PROGMEM = "RecordMaximum";
const char key_RecordMinimum[] PROGMEM = "RecordMinimum";

const char key_Widget[] PROGMEM = "Widget";
const char key_Type[] PROGMEM = "Type";
const char key_Src[] PROGMEM = "Src";
const char key_Disp[] PROGMEM = "Disp";
const char key_GaugeMin[] PROGMEM = "GaugeMin";
const char key_GaugeMax[] PROGMEM = "GaugeMax";

const char key_Notification[] PROGMEM = "Notification";
extern const char key_Enabled[] PROGMEM = "Enabled";
//const char key_Switch[] PROGMEM = "Switch";
const char key_Tone[] PROGMEM = "Tone";
const char key_Text[] PROGMEM = "Text";

//=================================================================================================

char* findStringInIdStr(const id_string_t *idStr_P, int8_t searchId)
{
  static char retStr[MAX_STR_SIZE];
  *retStr = '\0';
  uint8_t i = 0;
  while(1)
  {
    if(pgm_read_byte(&idStr_P[i].str) == '\0' || i == 0xff)
      break;
    if(pgm_read_byte(&idStr_P[i].id) == (uint8_t)searchId)
    {
      strlcpy_P(retStr, (const char*)&idStr_P[i].str, sizeof(retStr));
      break;
    }
    i++;
  }
  return retStr;
}

//=================================================================================================

template <typename T> void findIdInIdStr(const id_string_t *idStr_P, const char *searchStr, T &val) 
{
  uint8_t i = 0;
  bool found = false;
  while(1)
  {
    if(pgm_read_byte(&idStr_P[i].str) == '\0' || i == 0xff)
      break;
    if(MATCH_P(searchStr, (const char*)&idStr_P[i].str))
    {
      found = true;
      val = pgm_read_byte(&idStr_P[i].id);
      break;
    }
    i++;
  }
  if(!found)
    idNotFoundInIdStr = true;
}

// Explicit instantiation for the types we want to use, otherwise linking fails with 'undefined reference to..'
// Add more instantiations as needed
template void findIdInIdStr<uint8_t>(const id_string_t *idStr_P, const char *searchStr, uint8_t&);
template void findIdInIdStr<int8_t>(const id_string_t *idStr_P, const char *searchStr, int8_t&);
template void findIdInIdStr<uint16_t>(const id_string_t *idStr_P, const char *searchStr, uint16_t&);
template void findIdInIdStr<int16_t>(const id_string_t *idStr_P, const char *searchStr, int16_t&);

//=================================================================================================

void getSrcName(char* buff, uint8_t idx, uint8_t lenBuff)
{
  //For mixer sources, limit to 5 characters max otherwise text could overflow or get clipped in the UI.
  
  buff[0] = '\0';
  
  switch(idx)
  {
    case SRC_NONE: strlcpy_P(buff, PSTR("None"), lenBuff); break;
    case SRC_100PERC: strlcpy_P(buff, PSTR("Max"), lenBuff); break;
    case SRC_THR: strlcpy_P(buff, PSTR("Thr"), lenBuff); break;
    case SRC_AIL: strlcpy_P(buff, Model.type == MODEL_TYPE_MULTICOPTER ? PSTR("Roll") : PSTR("Ail"), lenBuff); break;
    case SRC_ELE: strlcpy_P(buff, Model.type == MODEL_TYPE_MULTICOPTER ? PSTR("Pitch") : PSTR("Ele"), lenBuff); break;
    case SRC_RUD: strlcpy_P(buff, Model.type == MODEL_TYPE_MULTICOPTER ? PSTR("Yaw") : PSTR("Rud"), lenBuff); break;
    case SRC_X1_TRIM: strlcpy_P(buff, PSTR("X1\xA9"), lenBuff); break;
    case SRC_Y1_TRIM: strlcpy_P(buff, PSTR("Y1\xAA"), lenBuff); break;
    case SRC_X2_TRIM: strlcpy_P(buff, PSTR("X2\xA9"), lenBuff); break;
    case SRC_Y2_TRIM: strlcpy_P(buff, PSTR("Y2\xAA"), lenBuff); break;
    
    default:
    {
      if(idx >= SRC_STICK_AXIS_FIRST && idx <= SRC_STICK_AXIS_LAST)
      {
        strlcpy(buff, findStringInIdStr(enum_StickAxisName, idx), lenBuff);
        break;
      }

      char suffix[5]; //large enough to hold the range we want to convert
      memset(suffix, 0, sizeof(suffix));
      if(idx >= SRC_KNOB_FIRST && idx <= SRC_KNOB_LAST)
      {
        strlcpy_P(buff, PSTR("Knob"), lenBuff);
        suffix[0] = 'A' + (idx - SRC_KNOB_FIRST);
        strlcat(buff, suffix, lenBuff);
      }
      else if(idx >= SRC_SW_PHYSICAL_FIRST && idx <= SRC_SW_PHYSICAL_LAST)
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
      else if(idx >= SRC_COUNTER_FIRST && idx <= SRC_COUNTER_LAST)
      {
        strlcpy_P(buff, PSTR("Counter"), lenBuff);
        itoa((idx - SRC_COUNTER_FIRST) + 1, suffix, 10);
        strlcat(buff, suffix, lenBuff);
      }
      else if(idx >= SRC_TIMER_FIRST && idx <= SRC_TIMER_LAST)
      {
        strlcpy_P(buff, PSTR("Timer"), lenBuff);
        itoa((idx - SRC_TIMER_FIRST) + 1, suffix, 10);
        strlcat(buff, suffix, lenBuff);
      }
      else if(idx >= SRC_TELEMETRY_FIRST && idx <= SRC_TELEMETRY_LAST)
      {
        strlcpy(buff, Model.Telemetry[idx - SRC_TELEMETRY_FIRST].name, lenBuff);
      }
      else if(idx == SRC_INACTIVITY_TIMER)
      {
        strlcpy_P(buff, PSTR("Inactvty"), lenBuff);
      }
      else if(idx == SRC_TX_BATTERY_VOLTAGE)
      {
        strlcpy_P(buff, PSTR("Battery"), lenBuff);
      }
    }
  }
}

//=================================================================================================

void getSrcName_Clean(char* buff, uint8_t idx, uint8_t lenBuff)
{
  //This function substitutes non printable characters and in the src name 
  //so that we can write it to a human readable text file format

  getSrcName(buff, idx, lenBuff);

  if(idx == SRC_X1_TRIM || idx == SRC_Y1_TRIM || idx == SRC_X2_TRIM || idx == SRC_Y2_TRIM)
  {
    for(uint8_t i = 0; i < lenBuff - 1; i++)
    {
      uint8_t c = *(buff + i);
      if(c == 0xA9 || c == 0xAA) { strlcpy_P(buff + i, PSTR("Trim"), lenBuff - i); break; }
    }
  }
}

//=================================================================================================

void getControlSwitchName(char* buff, uint8_t idx, uint8_t lenBuff)
{
  //Limit to 5 characters max, otherwise text could overflow or get clipped in the UI.

  buff[0] = '\0';
  
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
  else if(idx >= CTRL_SW_FMD_FIRST && idx <= CTRL_SW_FMD_LAST_INVERT)
  {
    uint8_t i = idx - CTRL_SW_FMD_FIRST;
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

//=================================================================================================

void getControlSwitchName_Clean(char* buff, uint8_t idx, uint8_t lenBuff)
{
  //This function substitutes non printable characters and the "--" 
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
      if(c == 0x18) { strlcpy_P(buff + i, PSTR("_Up"), lenBuff - i); break; }
      else if(c == '-')  { strlcpy_P(buff + i, PSTR("_Mid"), lenBuff - i); break; }
      else if(c == 0x19) { strlcpy_P(buff + i, PSTR("_Down"), lenBuff - i); break; }
    }
  }
}

