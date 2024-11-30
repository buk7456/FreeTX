#ifndef _COMMON_H_
#define _COMMON_H_

uint16_t joinBytes(uint8_t _highByte, uint8_t _lowByte);

//====================== MISC =====================================================================

#define MAX_PAYLOAD_SIZE 26
#define MAX_PACKET_SIZE  (4 + MAX_PAYLOAD_SIZE)

extern uint8_t transmitPayloadBuffer[MAX_PAYLOAD_SIZE];
extern uint8_t receivePayloadBuffer[MAX_PAYLOAD_SIZE];
extern uint8_t transmitPayloadLength;
extern uint8_t receivePayloadLength;
 
extern uint8_t rfPower;

extern bool     isRequestingBind;
extern uint8_t  bindStatusCode;  //1 on success, 2 on fail
 
extern bool     hasPendingRCData;

extern bool     hasReceivedTelemetry;

extern uint8_t  transmitterPacketRate;
extern uint8_t  receiverPacketRate;

//---- Telemetry --------------------------

extern bool isRequestingTelemetry; 

extern uint8_t receivedTelemetryType;

enum telemetry_type_e {
  TELEMETRY_TYPE_GENERAL = 0,
  TELEMETRY_TYPE_GNSS = 1,
};

//---- Output channel configuration -----

extern bool    isRequestingOutputChConfig;
extern bool    isSendOutputChConfig;
extern bool    gotOutputChConfig;
extern bool    isMainReceiver;

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
