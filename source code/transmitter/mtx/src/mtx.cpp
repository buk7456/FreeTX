
#include "Arduino.h"

#include "../config.h"
#include "common.h"
#include "crc.h"
#include "inputs.h"
#include "mathHelpers.h"
#include "mixer.h"
#include "ee/eestore.h"
#include "sd/sdStore.h"
#include "ui/ui.h"
#include "mtx.h"

uint32_t loopStartTime; //in microseconds

#define UART_FIXED_PACKET_SIZE 48

enum {
  MESSAGE_TYPE_NONE = 0x00,
  
  //mtx to stx
  MESSAGE_TYPE_RC_DATA = 0x01, 
  MESSAGE_TYPE_ENTER_BIND = 0X02,
  MESSAGE_TYPE_GET_RECEIVER_CONFIG = 0x03,
  MESSAGE_TYPE_WRITE_RECEIVER_CONFIG = 0x04,
  
  //stx to mtx
  MESSAGE_TYPE_BIND_STATUS_CODE = 0x10,
  MESSAGE_TYPE_RECEIVER_CONFIG = 0x11,
  MESSAGE_TYPE_RECEIVER_CONFIG_STATUS_CODE = 0x12,
  MESSAGE_TYPE_TELEMETRY_RF_LINK_PACKET_RATE = 0x13,
  MESSAGE_TYPE_TELEMETRY_GENERAL = 0x14,
  MESSAGE_TYPE_TELEMETRY_GNSS = 0x15,
};

//---------- Function declarations ---------------

void controlBacklight();
void doSerialCommunication();
void handleTelemetry();

//==================================================================================================

void setup()
{
  //set up ports and pins
  DDRx_TRIMS  = 0x00; //set port as input
  PORTx_TRIMS = 0xFF; //activate internal pullup resistors
  
  pinMode(PIN_KEY_SELECT, INPUT_PULLUP);
  pinMode(PIN_KEY_UP, INPUT_PULLUP);
  pinMode(PIN_KEY_DOWN, INPUT_PULLUP);
  
  pinMode(PIN_BUZZER, OUTPUT);
  
  pinMode(PIN_POWER_OFF_SENSE, INPUT);
  pinMode(PIN_POWER_LATCH, OUTPUT);
  digitalWrite(PIN_POWER_LATCH, HIGH); //latch power
  
  initialiseSwitches();

  //init serial ports 
  Serial1.begin(115200); 
  
  //delay a bit to allow time for other devices to be ready
  delay(100);

  //init display
  initialiseDisplay();
  
  //set defaults
  resetSystemParams();
  resetModelName();
  resetModelParams();
  
  //start in silent mode if the Down key is held on boot
  bool isSilentMode = false;
  readSwitchesAndButtons();
  while(buttonCode == KEY_DOWN)
  {
    isSilentMode = true;
    turnOnBacklight();
    showMuteMessage();
    readSwitchesAndButtons();
    delay(30);
  }
  
  //Initialise eeprom storage
  eeStoreInit();  //blocking
  
  //Load data from eeprom
  eeReadSysConfig();
  eeReadModelData(Sys.activeModelIdx);
  
  if(isSilentMode)
    Sys.soundEnabled = false;
  
  //backlight
  if(Sys.backlightEnabled)
    analogWrite(PIN_LCD_BACKLIGHT, ((uint16_t) 255 * Sys.backlightLevel) / 100);
  
  //Initialise battery reading
  battVoltsNow = Sys.battVoltsMax; 
  for(uint8_t i = 0; i < 100; i++)
  {
    checkBattery();
    delay(1);
  }

  //Welcome message
  if(Sys.showWelcomeMessage)
    showMessage(PSTR("Welcome"));
  
  uint32_t tt = millis();
  
  //Initialise sd card storage
  //moved here as it implicitly blocks for about 2seconds when there is no sd card
  sdStoreInit();
  
  if(Sys.showWelcomeMessage)
  {
    tt = millis() - tt;
    if(tt < 1000)
      delay(1000 - tt);
  }
  
  //splash screen
  if(Sys.showSplashScreen)
    sdShowSplashScreen();
    
  //Warnings
  handleSafetyWarnUI(); //blocking
  
  //Initialise timers
  resetTimerRegisters();
  restoreTimerRegisters();
  
  //Initialise counters
  resetCounterRegisters();
  restoreCounterRegisters();
  
  //Other initialisations
  randomSeed(battVoltsNow);
  reinitialiseMixerCalculations();
  loopStartTime = micros();

}

//==================================================================================================

void loop()
{
  thisLoopNum++;

  ///--- SWITCHES, BUTTONS
  readSwitchesAndButtons();
  determineButtonEvent();
  
  ///--- STICKS 
  readSticks();
  
  ///--- COMPUTE OUTPUTS
  computeChannelOutputs();
  
  ///--- HANDLE MAIN INTERFACE
  handleMainUI();
  
  ///--- LAZY SAVE MODEL DATA TO EEPROM
  //Limit calling to about 10x per second to prolong the eeprom life.
  //Saving say 2400bytes takes approximately 240seconds (4minutes) before starting over.
  //Without the limit, the same would take approximately 48seconds, so if we let the system run
  //for 6hours, we would be making 450 writes to a certain eeprom cell, compared to 90 when limited.
  if(thisLoopNum % (100 / fixedLoopTime) == 0)
    eeLazyWriteModelData(Sys.activeModelIdx);

  ///--- LAZY SAVE SYSTEM DATA TO EEPROM
  //The cycle time set to about 5 minutes
  const uint32_t intervalMillis = 300000UL / sizeof(Sys); 
  if(thisLoopNum % (intervalMillis / fixedLoopTime) == 0)
    eeLazyWriteSysConfig();
  
  ///--- LIMIT MAX RATE OF LOOP
  //This is done here for a more consistent timing of communications.
  //Code section changed to use micros() instead of millis().
  //Rollover isn't a problem here. 
  uint32_t loopTime = micros() - loopStartTime;
  if(Sys.DBG_showLoopTime) //debug
    DBG_loopTime = loopTime;
  if(loopTime < (fixedLoopTime * 1000)) 
    delayMicroseconds((fixedLoopTime * 1000) - loopTime);
  loopStartTime = micros(); 

  ///--- COMMUNICATIONS
  doSerialCommunication();

  ///--- TELEMETRY
  handleTelemetry();
  
  ///--- CHECK BATTERY
  checkBattery();
  
  ///--- BACKLIGHT
  controlBacklight();
  
  ///--- HANDLE POWER OFF 
  handlePowerOff();
}

//==================================================================================================

void checkBattery()
{
  //Low pass filtered using exponential smoothing
  //As the implementation here uses integer math and the technique is recursive, 
  //there is loss of precision but this doesn't matter much here.
  const int16_t smoothFactor = 5; //>0,<100
  int32_t sample = ((int32_t)analogRead(PIN_BATTVOLTS) * Sys.battVfactor) / 100;
  battVoltsNow = ((sample * smoothFactor) + ((100 - smoothFactor) * (int32_t)battVoltsNow)) / 100;
  if(battVoltsNow <= Sys.battVoltsMin)
    battState = BATTLOW;
  else if(battVoltsNow > (Sys.battVoltsMin + 100)) //100mV hysteris
    battState = BATTHEALTY; 
}

//==================================================================================================

void turnOnBacklight()
{
  //Force the backlight to be on, irrespective of system settings
  analogWrite(PIN_LCD_BACKLIGHT, 76); //30% level
  backlightIsOn = true;
}

//==================================================================================================

void controlBacklight()
{
  if(!Sys.backlightEnabled)
  {
    analogWrite(PIN_LCD_BACKLIGHT, 0);
    backlightIsOn = false;
    return;
  }

  static const uint16_t durTable[] PROGMEM = {5, 15, 60, 120, 300, 600, 1800, 0};
  uint32_t duration = (uint32_t) 1000 * pgm_read_word(&durTable[Sys.backlightTimeout]);
  
  uint8_t val = ((uint16_t) 255 * Sys.backlightLevel) / 100;
  uint32_t elapsed = 0;

  if(Sys.backlightWakeup == BACKLIGHT_WAKEUP_KEYS)
  {
    //Only allow the UI navigation keys to trigger the backlight on, suppressing the trim keys.
    //If the backlight is on, treat all keys the same.
    static bool inhibit = false;
    if(buttonCode == KEY_SELECT || buttonCode == KEY_DOWN || buttonCode == KEY_UP)
      inhibit = false;
    else if(backlightIsOn && buttonCode == 0 && !inhibit)
    {
      if(millis() - buttonReleaseTime >= duration)
        inhibit = true;
    }

    if(inhibit)
      elapsed = duration;
    else
    {
      if(buttonCode == 0)
        elapsed = millis() - buttonReleaseTime;
    }
  }
  else if(Sys.backlightWakeup == BACKLIGHT_WAKEUP_ACTIVITY)
    elapsed = millis() - inputsLastMovedTime;

  if(elapsed < duration || Sys.backlightTimeout == BACKLIGHT_TIMEOUT_NEVER)
  {
    analogWrite(PIN_LCD_BACKLIGHT, val);
    backlightIsOn = true;
  }
  else
  {
    analogWrite(PIN_LCD_BACKLIGHT, 0);
    backlightIsOn = false;
  }
}

//==================================================================================================

void doSerialCommunication()
{
  uint8_t buffer[UART_FIXED_PACKET_SIZE];
  memset(buffer, 0, sizeof(buffer));
  
  ///------------- SEND TO SECONDARY MCU --------------

  uint8_t messageType = Sys.rfEnabled ? MESSAGE_TYPE_RC_DATA : MESSAGE_TYPE_NONE;
  uint8_t dataLength = 0;

  if(isRequestingBind)
  {
    isRequestingBind = false;
    messageType = MESSAGE_TYPE_ENTER_BIND;
  }

  if(isRequestingOutputChConfig)
  {
    isRequestingOutputChConfig = false;
    messageType = MESSAGE_TYPE_GET_RECEIVER_CONFIG;
  }

  if(isSendOutputChConfig)
  {
    isSendOutputChConfig = false;
    messageType = MESSAGE_TYPE_WRITE_RECEIVER_CONFIG;
  }

  // Build the message body
  switch(messageType)
  {
    case MESSAGE_TYPE_ENTER_BIND:
    case MESSAGE_TYPE_GET_RECEIVER_CONFIG:
      {
        dataLength = 1;
        buffer[5] = isMainReceiver ? 1 : 0;
      }
      break;
    
    case MESSAGE_TYPE_WRITE_RECEIVER_CONFIG:
      {
        for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
        {
          buffer[5 + i] = outputChConfig[i];
        }
        uint8_t flagsIdx = 5 + MAX_CHANNELS_PER_RECEIVER;
        buffer[flagsIdx] = isMainReceiver ? 1 : 0;
        dataLength = MAX_CHANNELS_PER_RECEIVER + 1;
      }
      break;

    case MESSAGE_TYPE_RC_DATA:
      {
        bool isRequestingTelemetry = false;
        bool isFailsafeData = false;
        uint8_t qq = thisLoopNum % 64;
        if(qq == 16) //send failsafe data
          isFailsafeData = true;
        else if(qq == 0 || qq == 32) //request telemetry
        {
          //only request telemetry when necessary i.e. if there are configured sensors
          //or we are forcing telemetry request
          if(telemetryForceRequest)
            isRequestingTelemetry = true;
          else
          {
            for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
            {
              if(!isEmptyStr(Model.Telemetry[i].name, sizeof(Model.Telemetry[0].name))) 
              {
                isRequestingTelemetry = true;
                break;
              }
            }
          }
        }

        uint8_t numChannels = Model.secondaryRcvrEnabled ? NUM_RC_CHANNELS : MAX_CHANNELS_PER_RECEIVER;

        for(uint8_t chIdx = 0; chIdx < numChannels; chIdx++)
        {
          uint16_t val = 0;

          if(isFailsafeData)
          {
            if(Model.Channel[chIdx].failsafe == -102) //failsafe mode Hold, send 1023
              val = 1023;
            else if(Model.Channel[chIdx].failsafe == -101) //failsafe mode No pulse, send 1022
              val = 1022;
            else //failsafe is custom value
            {
              int16_t fsf = 5 * Model.Channel[chIdx].failsafe;
              fsf = constrain(fsf, 5 * Model.Channel[chIdx].endpointL, 5 * Model.Channel[chIdx].endpointR);
              val = fsf + 500;
            }
          }
          else //real time rc data
          {
            val = channelOut[chIdx] + 500; 
          }

          //encode into bit stream, with resolution of 10 bits
          uint8_t aIdx = chIdx + (chIdx / 4);
          uint8_t bIdx = aIdx + 1;
          uint8_t aShift = ((chIdx % 4) + 1) * 2;
          uint8_t bShift = 8 - aShift;
          buffer[5 + aIdx] |= ((val >> aShift) & 0xFF);
          buffer[5 + bIdx] |= ((val << bShift) & 0xFF);
        }

        dataLength = ((((numChannels * 10) + 7) / 8) + 1); // +1 is for the flags byte
        
        //write flags
        uint8_t flagsIdx = 5 + dataLength - 1;
        buffer[flagsIdx]  = (isFailsafeData & 0x01) << 4;
        buffer[flagsIdx] |= (isRequestingTelemetry & 0x01) << 3;
        buffer[flagsIdx] |= Sys.rfPower & 0x07;
      }
      break;
  }

  // Add other fields and send
  if(messageType != MESSAGE_TYPE_NONE || thisLoopNum == 1)
  {
    //Preamble
    buffer[0] = 0xAA;
    buffer[1] = 0xAA;
    buffer[2] = 0xAA;
  
    //Message type
    buffer[3] = messageType;

    //Data length
    buffer[4] = dataLength;

    //Data
    //Already set.
  
    //CRC
    buffer[5 + dataLength] = crc8(buffer, 5 + dataLength);

    //Padding bytes
    //Already set.

    //Send
    Serial1.write(buffer, sizeof(buffer));
  }

  ///------------- GET FROM SECONDARY MCU -------------

  memset(buffer, 0, sizeof(buffer));

  if(Serial1.available() < UART_FIXED_PACKET_SIZE)
    return;

  uint8_t cntr = 0;
  while(Serial1.available() > 0)
  {
    if(cntr < sizeof(buffer)) 
    {
      buffer[cntr] = Serial1.read();
      cntr++;
    }
    else //Discard any extra data
      Serial1.read();
  }

  //check CRC and extract data
  dataLength = buffer[4];
  uint8_t receivedCRC = buffer[5 + dataLength];
  uint8_t computedCRC = crc8(buffer, 5 + dataLength);
  if(computedCRC != receivedCRC)
    return;

  //get message type, extract the data
  messageType = buffer[3];
  switch(messageType)
  {
    case MESSAGE_TYPE_BIND_STATUS_CODE:
      {
        bindStatusCode = buffer[5];
      }
      break;

    case MESSAGE_TYPE_RECEIVER_CONFIG_STATUS_CODE:
      {
        receiverConfigStatusCode = buffer[5];
      }
      break;

    case MESSAGE_TYPE_RECEIVER_CONFIG:
      {
        gotOutputChConfig = true;
        for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
          outputChConfig[i] = buffer[5 + i];
      }
      break;

    case MESSAGE_TYPE_TELEMETRY_RF_LINK_PACKET_RATE:
      {
        transmitterPacketRate = buffer[5];
        receiverPacketRate = buffer[6];
        
        //Calculate the Link Quality indicator
        int16_t lqi = divRoundClosest(((int16_t) receiverPacketRate * 100), transmitterPacketRate);
        if(lqi > 100) lqi = 100;
        else if(lqi == 0) lqi = TELEMETRY_NO_DATA;
        //check against configured sensor IDs and write
        for(uint8_t idx = 0; idx < NUM_CUSTOM_TELEMETRY; idx++) 
        {
          if(Model.Telemetry[idx].identifier == SENSOR_ID_LINK_QLTY)
          {
            telemetryReceivedValue[idx] = lqi;
            if(telemetryReceivedValue[idx] != TELEMETRY_NO_DATA)
            {
              telemetryLastReceivedValue[idx] = telemetryReceivedValue[idx];
              telemetryLastReceivedTime[idx] = millis();
            }
          }
        }
      }
      break;

    case MESSAGE_TYPE_TELEMETRY_GENERAL:
      {
        uint8_t numFields = dataLength / 3; //there are 3 bytes per telemetry field
        for(uint8_t i = 0; i < numFields; i++)
        {
          uint8_t buffIdx = 5 + (i * 3);
          uint8_t sensorID = buffer[buffIdx];
          //check against configured Ids and copy to telemetryReceivedValue 
          for(uint8_t idx = 0; idx < NUM_CUSTOM_TELEMETRY; idx++)
          {
            if(sensorID == Model.Telemetry[idx].identifier)
            {
              telemetryReceivedValue[idx] = joinBytes(buffer[buffIdx + 1], buffer[buffIdx + 2]);
              if(telemetryReceivedValue[idx] != TELEMETRY_NO_DATA)
              {
                telemetryLastReceivedValue[idx] = telemetryReceivedValue[idx];
                telemetryLastReceivedTime[idx] = millis();
              }
            }
          }
        }
      }
      break;

    case MESSAGE_TYPE_TELEMETRY_GNSS:
      {
        //copy into the struct
        memcpy(&GNSSTelemetryData, &buffer[5], sizeof(GNSSTelemetryData));

        gnssTelemetrylastReceivedTime = millis();

        if(GNSSTelemetryData.positionFix != 0)
        {
          //calculate distance
          float lat1 = (float) Model.gnssHomeLatitude / 100000;
          float lng1 = (float) Model.gnssHomeLongitude / 100000;
          float lat2 = (float) GNSSTelemetryData.latitude / 100000;
          float lng2 = (float) GNSSTelemetryData.longitude / 100000; 
          gnssDistanceFromHome = (int32_t) distanceBetween(lat1, lng1, lat2, lng2);
          //store last know position
          Model.gnssLastKnownLatitude = GNSSTelemetryData.latitude;
          Model.gnssLastKnownLongitude = GNSSTelemetryData.longitude;
          Model.gnssLastKnownAltitude = GNSSTelemetryData.altitude;
          Model.gnssLastKnownDistanceFromHome = gnssDistanceFromHome;
        }

        //extract individual "sensors" for use in the pre-configured GNSS telemetry sensor templates
        static const uint8_t sensorIDs[] PROGMEM = {
          SENSOR_ID_GNSS_SPEED,
          SENSOR_ID_GNSS_DISTANCE,
          SENSOR_ID_GNSS_MSL_ALTITUDE,
          SENSOR_ID_GNSS_AGL_ALTITUDE
        };
        for(uint8_t i = 0; i < sizeof(sensorIDs)/sizeof(sensorIDs[0]); i++)
        {
          uint8_t id = pgm_read_byte(&sensorIDs[i]);
          //get the value
          int16_t val = TELEMETRY_NO_DATA;
          if(GNSSTelemetryData.positionFix != 0)
          {
            switch(id)
            {
              case SENSOR_ID_GNSS_SPEED:
                val = GNSSTelemetryData.speed;
                break;
              
              case SENSOR_ID_GNSS_DISTANCE:
                {
                  //Limit val to prevent overflow of 16 bit signed integer.
                  //When gnssDistanceFromHome exceeds this limit, value reverts to TELEMETRY_NO_DATA.
                  if(gnssDistanceFromHome < ((int32_t) 32767))
                    val = gnssDistanceFromHome;
                }
                break;
              
              case SENSOR_ID_GNSS_MSL_ALTITUDE:
                val = GNSSTelemetryData.altitude;
                break;
              
              case SENSOR_ID_GNSS_AGL_ALTITUDE:
                val = GNSSTelemetryData.altitude - Model.gnssAltitudeOffset;
                break;
            }
          }
          //check against configured Ids and copy to telemetryReceivedValue
          for(uint8_t idx = 0; idx < NUM_CUSTOM_TELEMETRY; idx++)
          {
            if(Model.Telemetry[idx].identifier == id)
            {
              telemetryReceivedValue[idx] = val;
              if(telemetryReceivedValue[idx] != TELEMETRY_NO_DATA)
              {
                telemetryLastReceivedValue[idx] = telemetryReceivedValue[idx];
                telemetryLastReceivedTime[idx] = millis();
              }
            }
          }
        }
      }
      break;
  }

}

//==================================================================================================

void handleTelemetry()
{
  uint32_t currMillis = millis();

  //-- simulated telemetry
  if(Sys.DBG_simulateTelemetry)
  {
    //check against configured Ids and copy to telemetryReceivedValue 
    for(uint8_t idx = 0; idx < NUM_CUSTOM_TELEMETRY; idx++)
    {
      if(Model.Telemetry[idx].identifier == SENSOR_ID_SIMULATED)
      {
        telemetryReceivedValue[idx] = mixSources[SRC_VIRTUAL_FIRST] / 5;
        telemetryLastReceivedValue[idx] = telemetryReceivedValue[idx];
        telemetryLastReceivedTime[idx] = currMillis;
      }
    }
  }
  
  //-- statistics
  for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
  {
    //-- get statistics
    if(telemetryReceivedValue[i] != TELEMETRY_NO_DATA)
    {
      //max
      if(telemetryReceivedValue[i] > telemetryMaxReceivedValue[i] || telemetryMaxReceivedValue[i] == TELEMETRY_NO_DATA)   
        telemetryMaxReceivedValue[i] = telemetryReceivedValue[i];
      //min
      if(telemetryReceivedValue[i] < telemetryMinReceivedValue[i] || telemetryMinReceivedValue[i] == TELEMETRY_NO_DATA)   
        telemetryMinReceivedValue[i] = telemetryReceivedValue[i];
    }
    //-- reset received telemetry if timeout
    if(currMillis - telemetryLastReceivedTime[i] > 3000)
      telemetryReceivedValue[i] = TELEMETRY_NO_DATA; 
  }
  
  //-- reset telemetry on model change
  static uint8_t lastModelIdx = 0xff;
  if(Sys.activeModelIdx != lastModelIdx)
  {
    lastModelIdx = Sys.activeModelIdx;
    for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
    {
      telemetryLastReceivedTime[i] = currMillis;
      telemetryLastReceivedValue[i] = TELEMETRY_NO_DATA;
      telemetryReceivedValue[i] = TELEMETRY_NO_DATA;
      telemetryMaxReceivedValue[i] = TELEMETRY_NO_DATA;
      telemetryMinReceivedValue[i] = TELEMETRY_NO_DATA;
    }
  }

  //-- reset RF link statistics if RF is disabled
  if(!Sys.rfEnabled)
  {
    transmitterPacketRate = 0;
    receiverPacketRate = 0;
  }

  //reset gnss specific data
  if(currMillis - gnssTelemetrylastReceivedTime > 3000)
  {
    GNSSTelemetryData.satellitesInUse = 0;
    GNSSTelemetryData.satellitesInView = 0;
    GNSSTelemetryData.positionFix = 0;
  }
}

//==================================================================================================

void handlePowerOff()
{ 
  static uint8_t _counter = 0;
  if(digitalRead(PIN_POWER_OFF_SENSE) == HIGH) 
    _counter++;
  else if(_counter > 0)
    _counter--;
  if(_counter > 1000/fixedLoopTime) //power off
  {
    showMessage(PSTR("Shutting down"));
    if(Sys.backlightEnabled)
      analogWrite(PIN_LCD_BACKLIGHT, ((uint16_t) 255 * Sys.backlightLevel) / 100);
    delay(1000);
    //save changes
    if(eeStoreIsInitialised())
    {
      eeSaveSysConfig();
      eeSaveModelData(Sys.activeModelIdx);
    }
    showMessage(PSTR(""));
    //power off
    delay(100);
    pinMode(PIN_POWER_LATCH, INPUT);
    while(1){}
  } 
}

//==================================================================================================

int16_t getFreeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  int freeRam = (int) &v - (__brkval == 0 ? (int) &__heap_start: (int) __brkval);
  
  return freeRam;
}

