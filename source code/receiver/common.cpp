
#include "Arduino.h"
#include "common.h"

sys_params_t Sys;

int16_t channelOut[MAX_CHANNELS_PER_RECEIVER];
int16_t channelFailsafe[MAX_CHANNELS_PER_RECEIVER];

uint16_t externalVolts = 0;

bool failsafeEverBeenReceived[MAX_CHANNELS_PER_RECEIVER];
bool isRequestingBind = false;
uint32_t lastRCPacketMillis = 0;

gnss_telemetry_data_t GNSSTelemetryData;

bool hasGNSSModule = false;

uint8_t telemetryType;
