#ifndef _COMMON_H_
#define _COMMON_H_

//====================== MISC =====================================================================

#define NUM_RC_CHANNELS  20 
#define MAX_CHANNELS_PER_RECEIVER  10

extern int16_t channelOut[MAX_CHANNELS_PER_RECEIVER];
extern int16_t channelFailsafe[MAX_CHANNELS_PER_RECEIVER];

extern uint16_t externalVolts; //in millivolts

extern bool failsafeEverBeenReceived;
extern bool isRequestingBind;
extern uint32_t lastRCPacketMillis;

#define TELEMETRY_NO_DATA  0x7FFF

#define FIXED_PAYLOAD_SIZE ((((NUM_RC_CHANNELS * 10) + 7) / 8) + 1)

#if FIXED_PAYLOAD_SIZE < 10
  #undef  FIXED_PAYLOAD_SIZE
  #define FIXED_PAYLOAD_SIZE  10
#endif

//====================== SYSTEM PARAMETERS =========================================================

#define NUM_HOP_CHANNELS  3 //for frequency hopping

typedef struct {
  uint8_t transmitterID;  //set on bind
  uint8_t receiverID;     //set on bind
  uint8_t fhss_schema[NUM_HOP_CHANNELS]; // Index in freqList. This is the hopping sequence.
  bool    isMainReceiver;
  uint8_t outputChConfig[MAX_CHANNELS_PER_RECEIVER];
} sys_params_t;

extern sys_params_t Sys;

enum {
  SIGNAL_TYPE_DIGITAL = 0,
  SIGNAL_TYPE_SERVOPWM = 1,
  SIGNAL_TYPE_PWM = 2  
};


#endif
