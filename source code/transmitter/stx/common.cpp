
#include "Arduino.h"
#include "common.h"

sys_params_t Sys;

bool    rfEnabled = false;
uint8_t rfPower;

uint16_t chData[NUM_RC_CHANNELS];
bool    isFailsafeData = false;

bool    isRequestingBind = false;
uint8_t bindStatusCode;  

bool    isMainReceiver = true;

bool    hasPendingRCData = false;

uint8_t transmitterPacketRate;
uint8_t receiverPacketRate;

uint8_t telemID[MAX_TELEMETRY_COUNT];
int16_t telemVal[MAX_TELEMETRY_COUNT];

bool    isRequestingTelemetry = false;  

bool    isRequestingOutputChConfig = false;
bool    isSendOutputChConfig = false;
bool    gotOutputChConfig = false;

uint8_t outputChConfig[NUM_RC_CHANNELS]; 

uint8_t receiverConfigStatusCode;

//--------------------------------------------------------------------------------------------------

uint16_t joinBytes(uint8_t _highByte, uint8_t _lowByte)
{
  uint16_t rslt;
  rslt = (uint16_t) _highByte;
  rslt <<= 8;
  rslt |= (uint16_t) _lowByte;
  return rslt;
}

