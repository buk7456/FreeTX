#ifndef _COMMON_H_
#define _COMMON_H_

//====================== MISC =====================================================================

#define NUM_RC_CHANNELS  20 
#define MAX_CHANNELS_PER_RECEIVER  10

extern int16_t channelOut[MAX_CHANNELS_PER_RECEIVER];
extern int16_t channelFailsafe[MAX_CHANNELS_PER_RECEIVER];

extern uint16_t externalVolts; //in millivolts

extern bool failsafeEverBeenReceived[MAX_CHANNELS_PER_RECEIVER];
extern bool isRequestingBind;
extern uint32_t lastRCPacketMillis;

#define TELEMETRY_NO_DATA  0x7FFF

//--- GNSS telemetry

typedef struct {
  int32_t  latitude;     //in degrees, fixed point representation; 5 decimal places of precision 
  int32_t  longitude;    //in degrees, fixed point representation; 5 decimal places of precision
  int16_t  altitude;     //Mean Sea Level altitude in meters,  fixed point representation; 0 decimal places of precision
  uint16_t speed;        //speed over ground in m/s, fixed point representation; 1 decimal places of precision
  uint16_t course;       //course over ground in degrees, fixed point representation; 1 decimal places of precision
  uint8_t  positionFix;  //position fix indicator
  uint8_t  satellitesInUse;  //number of satellites in use
  uint8_t  satellitesInView; //number of satellites in view
} gnss_telemetry_data_t;

extern gnss_telemetry_data_t GNSSTelemetryData;

extern bool hasGNSSModule;

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

enum signal_type_e {
  SIGNAL_TYPE_DIGITAL = 0,
  SIGNAL_TYPE_SERVOPWM = 1,
  SIGNAL_TYPE_PWM = 2  
};

enum telemetry_type_e {
  TELEMETRY_TYPE_GENERAL = 0,
  TELEMETRY_TYPE_GNSS = 1,
};

extern uint8_t telemetryType;

#endif
