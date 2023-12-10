#ifndef _COMMON_H_
#define _COMMON_H_

uint16_t joinBytes(uint8_t _highByte, uint8_t _lowByte);

//====================== MISC =====================================================================

extern bool    rfEnabled;
extern uint8_t rfPower;

#define NUM_RC_CHANNELS   20    //Same value as in main mcu

extern uint16_t chData[NUM_RC_CHANNELS];

extern bool     isFailsafeData;

extern bool     isRequestingBind;
extern uint8_t  bindStatusCode;  //1 on success, 2 on fail
 
extern bool     hasPendingRCData;

extern uint8_t  transmitterPacketRate;
extern uint8_t  receiverPacketRate;


#define FIXED_PAYLOAD_SIZE ((((NUM_RC_CHANNELS * 10) + 7) / 8) + 1)

#if FIXED_PAYLOAD_SIZE < 10
  #undef  FIXED_PAYLOAD_SIZE
  #define FIXED_PAYLOAD_SIZE  10
#endif

//---- Telemetry --------------------------

#define MAX_TELEMETRY_COUNT  (FIXED_PAYLOAD_SIZE / 3)

extern uint8_t telemID[MAX_TELEMETRY_COUNT];
extern int16_t telemVal[MAX_TELEMETRY_COUNT]; 

#define TELEMETRY_NO_DATA  0x7FFF

extern bool isRequestingTelemetry; 

//---- Output channel configuration -----

extern bool    isRequestingOutputChConfig;
extern bool    isSendOutputChConfig;
extern bool    gotOutputChConfig;
extern bool    isMainReceiver;

extern uint8_t chConfigData[NUM_RC_CHANNELS]; //Lower nibble is current config, upper nibble is max config

extern uint8_t receiverConfigStatusCode; //1 on success, 2 on fail

//====================== SYSTEM PARAMETERS =========================================================

#define NUM_HOP_CHANNELS  3 //for frequency hopping

typedef struct {
  uint8_t  transmitterID;  //set on bind
  uint8_t  receiverID;     //set on bind
  uint8_t  fhss_schema[NUM_HOP_CHANNELS]; //Stores indexes in freqList for hopping. 
} sys_params_t;

extern sys_params_t Sys;


#endif
