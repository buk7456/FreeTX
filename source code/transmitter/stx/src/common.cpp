
#include "Arduino.h"
#include "common.h"

sys_params_t Sys;

uint8_t  transmitPayloadBuffer[MAX_PAYLOAD_SIZE];
uint8_t  receivePayloadBuffer[MAX_PAYLOAD_SIZE];
uint8_t  transmitPayloadLength;
uint8_t  receivePayloadLength;

uint8_t  rfPower;
bool     isRequestingBind = false;
uint8_t  bindStatusCode;  
bool     isMainReceiver = true;
bool     hasPendingRCData = false;
bool     hasReceivedTelemetry = false;
uint8_t  transmitterPacketRate;
uint8_t  receiverPacketRate;
uint32_t generalTelemetryLastReceiveTime;
bool     hasPendingRFLinkMessage = false;
bool     isRequestingTelemetry = false;  
bool     isRequestingOutputChConfig = false;
bool     isSendOutputChConfig = false;
bool     gotOutputChConfig = false;
uint8_t  receiverConfigStatusCode;
uint8_t  receivedTelemetryType;

//--------------------------------------------------------------------------------------------------

uint16_t joinBytes(uint8_t _highByte, uint8_t _lowByte)
{
  uint16_t rslt;
  rslt = (uint16_t) _highByte;
  rslt <<= 8;
  rslt |= (uint16_t) _lowByte;
  return rslt;
}

