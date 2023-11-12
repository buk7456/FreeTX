#ifndef _COMMON_H_
#define _COMMON_H_

#define NUM_RC_CHANNELS  20 
#define MAX_CHANNELS_PER_RECEIVER  10

extern int16_t channelOut[MAX_CHANNELS_PER_RECEIVER];
extern int16_t channelFailsafe[MAX_CHANNELS_PER_RECEIVER];
extern uint8_t maxOutputChConfig[MAX_CHANNELS_PER_RECEIVER];

extern uint16_t externalVolts; //in millivolts

extern bool failsafeEverBeenReceived;
extern bool isRequestingBind;
extern uint32_t lastRCPacketMillis;

#define TELEMETRY_NO_DATA  0x7FFF

//====================== SYSTEM PARAMETERS =========================================================

#define NUM_HOP_CHANNELS  3 //for frequency hopping

typedef struct {
  uint8_t transmitterID;  //set on bind
  uint8_t receiverID;     //set on bind
  uint8_t fhss_schema[NUM_HOP_CHANNELS]; // Index in freqList. This is the hopping sequence.
  bool    isMainReceiver;
  uint8_t outputChConfig[MAX_CHANNELS_PER_RECEIVER]; //0 digital, 1 Servo, 2 PWM
} sys_params_t;

extern sys_params_t Sys;

enum {
  CFG_DIGITAL = 0,
  CFG_SERVO,
  CFG_PWM    
};


#endif