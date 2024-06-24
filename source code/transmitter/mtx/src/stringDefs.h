#ifndef _STRING_DEFS_H_
#define _STRING_DEFS_H_

#define MAX_STR_SIZE 21  //including null terminator

#define MATCH_P(str1, str2) (strcasecmp_P((str1), (str2)) == 0)
#define MATCH(str1, str2)   (strcasecmp((str1), (str2)) == 0)

typedef struct {
  int8_t id;
  const char str[MAX_STR_SIZE];
} id_string_t; 

char* findStringInIdStr(const id_string_t *idStr_P, int8_t searchId);
template <typename T> void findIdInIdStr(const id_string_t *idStr_P, const char *searchStr, T &val);

extern bool idNotFoundInIdStr; //for basic error detection only

void getSrcName(char* buff, uint8_t idx, uint8_t lenBuff);
void getControlSwitchName(char* buff, uint8_t idx, uint8_t lenBuff);
void getControlSwitchName_Clean(char* buff, uint8_t idx, uint8_t lenBuff);

//---- Strings for the enumerations ----
//Arrays of structs in PROGMEM

//system related
extern const id_string_t enum_RFpower[] PROGMEM;
extern const id_string_t enum_SwitchType[] PROGMEM;
extern const id_string_t enum_BacklightWakeup[] PROGMEM;
extern const id_string_t enum_BacklightTimeout[] PROGMEM;
extern const id_string_t enum_StickMode[] PROGMEM;
extern const id_string_t enum_StickAxisType[] PROGMEM;
extern const id_string_t enum_StickAxisName[] PROGMEM;
extern const id_string_t enum_KnobType[] PROGMEM;

//model related
extern const id_string_t enum_ModelType[] PROGMEM;
extern const id_string_t enum_TrimState[] PROGMEM;
extern const id_string_t enum_FuncgenWaveform[] PROGMEM;
extern const id_string_t enum_FuncgenPeriodMode[] PROGMEM;
extern const id_string_t enum_FuncgenWidthMode[] PROGMEM;
extern const id_string_t enum_FuncgenPhaseMode[] PROGMEM;
extern const id_string_t enum_CounterDirection[] PROGMEM;
extern const id_string_t enum_MixerOperation[] PROGMEM;
extern const id_string_t enum_MixerCurveType[] PROGMEM;
extern const id_string_t enum_MixerCurveType_Func[] PROGMEM;
extern const id_string_t enum_LogicalSwitch_Func[] PROGMEM;
extern const id_string_t enum_DirectionOfChange[] PROGMEM;
extern const id_string_t enum_ChannelFailsafe[] PROGMEM;
extern const id_string_t enum_TelemetryAlarmCondition[] PROGMEM;
extern const id_string_t enum_WidgetType[] PROGMEM;
extern const id_string_t enum_WidgetSource[] PROGMEM;
extern const id_string_t enum_WidgetDisplay[] PROGMEM;
extern const id_string_t enum_SwitchWarn[] PROGMEM;
extern const id_string_t enum_ClockEdge[] PROGMEM;

//---- Strings for the keys -----
//Key descriptors should be unique, but can also be shared.
//Repeated keys should be commented out, they are here only for convenience.

extern const char key_ModelName[] PROGMEM;
extern const char key_ModelType[] PROGMEM;

extern const char key_RudSrc[] PROGMEM;
extern const char key_ThrSrc[] PROGMEM;
extern const char key_AilSrc[] PROGMEM;
extern const char key_EleSrc[] PROGMEM;

extern const char key_YawSrc[] PROGMEM;
// extern const char key_ThrSrc[] PROGMEM;
extern const char key_RollSrc[] PROGMEM;
extern const char key_PitchSrc[] PROGMEM;

extern const char key_Number[] PROGMEM;

extern const char key_Timer[] PROGMEM;
extern const char key_ResetSwitch[] PROGMEM;
extern const char key_InitialMinutes[] PROGMEM;
extern const char key_IsPersistent[] PROGMEM;
extern const char key_PersistVal[] PROGMEM;

extern const char key_X1Trim[] PROGMEM;
extern const char key_Y1Trim[] PROGMEM;
extern const char key_X2Trim[] PROGMEM;
extern const char key_Y2Trim[] PROGMEM;
extern const char key_TrimState[] PROGMEM;
extern const char key_CommonTrim[] PROGMEM;

extern const char key_RudDualRate[] PROGMEM;
extern const char key_AilDualRate[] PROGMEM;
extern const char key_EleDualRate[] PROGMEM;

extern const char key_YawDualRate[] PROGMEM;
extern const char key_RollDualRate[] PROGMEM;
extern const char key_PitchDualRate[] PROGMEM;

extern const char key_Rate1[] PROGMEM;
extern const char key_Rate2[] PROGMEM;
extern const char key_Expo1[] PROGMEM;
extern const char key_Expo2[] PROGMEM;
extern const char key_Switch[] PROGMEM;

extern const char key_ThrottleCurve[] PROGMEM;
extern const char key_CustomCurve[] PROGMEM;
extern const char key_Name[] PROGMEM;
extern const char key_NumPoints[] PROGMEM;
extern const char key_XVal[] PROGMEM;
extern const char key_YVal[] PROGMEM;
extern const char key_Smooth[] PROGMEM;

extern const char key_Funcgen[] PROGMEM;
extern const char key_Waveform[] PROGMEM;
extern const char key_PeriodMode[] PROGMEM;
extern const char key_Period1[] PROGMEM;
extern const char key_Period2[] PROGMEM;
extern const char key_ModulatorSrc[] PROGMEM;
extern const char key_ReverseModulator[] PROGMEM;
extern const char key_PhaseMode[] PROGMEM;
extern const char key_Phase[] PROGMEM;
extern const char key_WidthMode[] PROGMEM;
extern const char key_Width[] PROGMEM;
extern const char key_Period[] PROGMEM;

extern const char key_Mixer[] PROGMEM;
// extern const char key_Name[] PROGMEM;
extern const char key_Output[] PROGMEM;
extern const char key_Operation[] PROGMEM;
// extern const char key_Switch[] PROGMEM;
extern const char key_Input[] PROGMEM;
extern const char key_Weight[] PROGMEM;
extern const char key_Offset[] PROGMEM;
extern const char key_CurveType[] PROGMEM;
extern const char key_CurveVal[] PROGMEM;
extern const char key_TrimEnabled[] PROGMEM;
extern const char key_FlightMode[] PROGMEM;
extern const char key_DelayUp[] PROGMEM;
extern const char key_DelayDown[] PROGMEM;
extern const char key_SlowUp[] PROGMEM;
extern const char key_SlowDown[] PROGMEM;

extern const char key_LogicalSwitch[] PROGMEM;
extern const char key_Func[] PROGMEM;
extern const char key_Val1[] PROGMEM;
extern const char key_Val2[] PROGMEM;
extern const char key_Val3[] PROGMEM;
extern const char key_Val4[] PROGMEM;

extern const char key_Counter[] PROGMEM;
// extern const char key_Name[] PROGMEM;
extern const char key_Clock[] PROGMEM;
extern const char key_Edge[] PROGMEM;
extern const char key_Clear[] PROGMEM;
extern const char key_Modulus[] PROGMEM;
extern const char key_Direction[] PROGMEM;
// extern const char key_IsPersistent[] PROGMEM;
// extern const char key_PersistVal[] PROGMEM;

// extern const char key_Name[] PROGMEM;
// extern const char key_Switch[] PROGMEM;
// extern const char key_X1Trim[] PROGMEM;
// extern const char key_Y1Trim[] PROGMEM;
// extern const char key_X2Trim[] PROGMEM;
// extern const char key_Y2Trim[] PROGMEM;
extern const char key_TransitionTime[] PROGMEM;

extern const char key_CheckThrottle[] PROGMEM;
extern const char key_SwitchWarn[] PROGMEM;

extern const char key_Channel[] PROGMEM;
// extern const char key_Name[] PROGMEM;
extern const char key_Reverse[] PROGMEM;
extern const char key_Subtrim[] PROGMEM;
extern const char key_OverrideSwitch[] PROGMEM;
extern const char key_OverrideVal[] PROGMEM;
extern const char key_Failsafe[] PROGMEM;
extern const char key_EndpointL[] PROGMEM;
extern const char key_EndpointR[] PROGMEM;

extern const char key_Telemetry[] PROGMEM;
// extern const char key_Name[] PROGMEM;
extern const char key_UnitsName[] PROGMEM;
extern const char key_Identifier[] PROGMEM;
extern const char key_Multiplier[] PROGMEM;
extern const char key_Factor10[] PROGMEM;
// extern const char key_Offset[] PROGMEM;
extern const char key_LogEnabled[] PROGMEM;
extern const char key_AlarmCondition[] PROGMEM;
extern const char key_AlarmThreshold[] PROGMEM;
extern const char key_AlarmMelody[] PROGMEM;
extern const char key_ShowOnHome[] PROGMEM;
extern const char key_RecordMaximum[] PROGMEM;
extern const char key_RecordMinimum[] PROGMEM;

extern const char key_Widget[] PROGMEM;
extern const char key_Type[] PROGMEM;
extern const char key_Src[] PROGMEM;
extern const char key_Disp[] PROGMEM;
extern const char key_GaugeMin[] PROGMEM;
extern const char key_GaugeMax[] PROGMEM;

extern const char key_Notification[] PROGMEM;
// extern const char key_Switch[] PROGMEM;
extern const char key_Tone[] PROGMEM;
extern const char key_Text[] PROGMEM;

#endif
