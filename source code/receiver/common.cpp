
#include "Arduino.h"
#include "common.h"

sys_params_t Sys;

int16_t channelOut[MAX_CHANNELS_PER_RECEIVER];
int16_t channelFailsafe[MAX_CHANNELS_PER_RECEIVER];
uint8_t maxOutputChConfig[MAX_CHANNELS_PER_RECEIVER];

uint16_t externalVolts = 0;

bool failsafeEverBeenReceived = false;
bool isRequestingBind = false;
uint32_t lastRCPacketMillis = 0;

