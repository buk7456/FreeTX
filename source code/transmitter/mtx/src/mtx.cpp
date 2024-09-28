
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
    showMuteMsg();
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
  if(Sys.showWelcomeMsg)
    showMsg(PSTR("Welcome"));
  
  uint32_t tt = millis();
  
  //Initialise sd card storage
  //moved here as it implicitly blocks for about 2seconds when there is no sd card
  sdStoreInit();
  
  if(Sys.showWelcomeMsg)
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
  
  uint8_t val = ((uint16_t) 255 * Sys.backlightLevel) / 100;
  uint32_t elapsed = 0;
  if(buttonCode == 0)
    elapsed = millis() - buttonReleaseTime;
  if(Sys.backlightWakeup == BACKLIGHT_WAKEUP_ACTIVITY)
    elapsed = millis() - inputsLastMoved;
  
  static const uint16_t durTable[] PROGMEM = {5, 15, 60, 120, 300, 600, 1800, 0};
  uint32_t duration = (uint32_t) 1000 * pgm_read_word(&durTable[Sys.backlightTimeout]);

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
  //See the 'protocol_over_uart.txt' in the doc folder for information about the formats used.
  
  ///------------- SEND TO SECONDARY MCU --------------

  enum {
    FLAG_RF_ENABLED       = 0x08,
    FLAG_WRITE_RX_CONFIG  = 0x10,
    FLAG_GET_RX_CONFIG    = 0x20,
    FLAG_GET_TELEMETRY    = 0x40,
    FLAG_ENTER_BIND       = 0x80,
  };
  
  enum {
    FLAG_FAILSAFE_DATA    = 0x01,
    FLAG_MAIN_RECEIVER    = 0x02
  };
  
  const uint8_t NUM_CRC_BYTES = 1;
  
  uint8_t dataToSend[2 + (2 * NUM_RC_CHANNELS) + NUM_CRC_BYTES];
  memset(dataToSend, 0, sizeof(dataToSend));

  uint8_t status0 = 0;
  uint8_t status1 = 0;
  
  //rf power level
  status0 |= (Sys.rfPower & 0x07);
  
  //rf enabled
  if(Sys.rfEnabled)
    status0 |= FLAG_RF_ENABLED;
  
  //bind
  if(isRequestingBind)
  {
    status0 |= FLAG_ENTER_BIND;
    isRequestingBind = false;
  }
  
  //alternately send failsafe or request telemetry
  const uint16_t interval = 740; //the LCM of this value and 1000 should be fairly large
  if(thisLoopNum % (interval/fixedLoopTime) == 1) 
    status1 |= FLAG_FAILSAFE_DATA;
  else if(thisLoopNum % (interval/fixedLoopTime) == interval/(2*fixedLoopTime) + 1)
  {
    //only request telemetry when necessary i.e. if there are configured sensors
    //or we are forcing telemetry request
    if(telemetryForceRequest)
      status0 |= FLAG_GET_TELEMETRY;
    else
    {
      for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
      {
        if(!isEmptyStr(Model.Telemetry[i].name, sizeof(Model.Telemetry[0].name))) 
        {
          status0 |= FLAG_GET_TELEMETRY;
          break;
        }
      }
    }
  }

  //requesting receiver configuration
  if(isRequestingOutputChConfig)
  {
    status0 |= FLAG_GET_RX_CONFIG;
    isRequestingOutputChConfig = false;
    //unset other flags
    status0 &= ~FLAG_GET_TELEMETRY; 
  }
  
  //sending receiver configuration
  if(isSendOutputChConfig)
  {
    status0 |= FLAG_WRITE_RX_CONFIG;
    isSendOutputChConfig = false;
    //unset other flags
    status0 &= ~FLAG_GET_TELEMETRY;    
    status1 &= ~FLAG_FAILSAFE_DATA;
  }
  
  //Indicate which receiver it is. Only matters if bind or receiver configuration
  if(isMainReceiver)
    status1 |= FLAG_MAIN_RECEIVER;

  //copy
  dataToSend[0] = status0;
  dataToSend[1] = status1;

  //general data
  if(status0 & FLAG_WRITE_RX_CONFIG) //receiver config data
  {
    for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
      dataToSend[2 + i] = outputChConfig[i];
  }
  else if(status1 & FLAG_FAILSAFE_DATA) //failsafe data
  {
    for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
    {
      if(Model.Channel[i].failsafe == -102) //failsafe mode Hold, send 1023
      {
        dataToSend[2 + i*2] = 0x03;
        dataToSend[3 + i*2] = 0xFF;
      }
      else if(Model.Channel[i].failsafe == -101) //failsafe mode No pulse, send 1022
      {
        dataToSend[2 + i*2] = 0x03;
        dataToSend[3 + i*2] = 0xFE;
      }
      else //failsafe is custom value
      {
        int16_t fsf = 5 * Model.Channel[i].failsafe;
        fsf = constrain(fsf, 5 * Model.Channel[i].endpointL, 5 * Model.Channel[i].endpointR);
        uint16_t val = (fsf + 500) & 0xFFFF;
        dataToSend[2 + i*2] = (val >> 8) & 0xFF;
        dataToSend[3 + i*2] = val & 0xFF;
      }
    }
  }
  else //real time RC data
  {
    for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
    {
      uint16_t val = (channelOut[i] + 500) & 0xFFFF; 
      dataToSend[2 + i*2] = (val >> 8) & 0xFF;
      dataToSend[3 + i*2] = val & 0xFF;
    }
  }
  
  //add a crc
  dataToSend[sizeof(dataToSend) - NUM_CRC_BYTES] = crc8(dataToSend, sizeof(dataToSend) - NUM_CRC_BYTES);
  
  //send
  Serial1.write(dataToSend, sizeof(dataToSend));
  
  
  ///------------- GET FROM SECONDARY MCU -------------

  //--- read into temp buff
  const uint8_t msgLength = 3 + NUM_RC_CHANNELS + NUM_CRC_BYTES;
  if(Serial1.available() < msgLength)
    return;
  
  uint8_t tmpBuff[msgLength]; 
  memset(tmpBuff, 0, msgLength);

  uint8_t cntr = 0;
  while(Serial1.available() > 0)
  {
    if(cntr < msgLength) 
    {
      tmpBuff[cntr] = Serial1.read();
      cntr++;
    }
    else //Discard any extra data
      Serial1.read();
  }
  
  //check crc and extract data
  
  if(tmpBuff[msgLength - 1] != crc8(tmpBuff, msgLength - 1))
    return;

  bindStatusCode = tmpBuff[0] & 0x03;
  receiverConfigStatusCode = (tmpBuff[0] >> 2) & 0x03;
  gotOutputChConfig = (tmpBuff[0] >> 4) & 0x01;
  
  transmitterPacketRate = tmpBuff[1];
  receiverPacketRate    = tmpBuff[2];
  
  if(gotOutputChConfig) //output configuration
  {
    for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
      outputChConfig[i] = tmpBuff[3 + i];
  }
  else //telemetry
  {
    const uint8_t MAX_FIELDS = NUM_RC_CHANNELS / 3; // there are 3 bytes per telemetry field
    for(uint8_t i = 0; i < MAX_FIELDS; i++)
    {
      uint8_t buffIdx = 3 + (i * 3);
      uint8_t sensorID = tmpBuff[buffIdx];
      //check against configured Ids and copy to telemetryReceivedValue 
      for(uint8_t idx = 0; idx < NUM_CUSTOM_TELEMETRY; idx++)
      {
        if(sensorID == Model.Telemetry[idx].identifier)
        {
          telemetryReceivedValue[idx] = joinBytes(tmpBuff[buffIdx + 1], tmpBuff[buffIdx + 2]);
          if(telemetryReceivedValue[idx] != TELEMETRY_NO_DATA)
          {
            telemetryLastReceivedValue[idx] = telemetryReceivedValue[idx];
            telemetryLastReceivedTime[idx] = millis();
          }
        }
      }
    }

    //Calculate the Link Quality indicator
    int16_t lqi = divRoundClosest(((int16_t) receiverPacketRate * 100), transmitterPacketRate);
    if(lqi > 100) lqi = 100;
    else if(lqi == 0) lqi = TELEMETRY_NO_DATA;
    //check against configured Ids and write
    for(uint8_t idx = 0; idx < NUM_CUSTOM_TELEMETRY; idx++) 
    {
      if(Model.Telemetry[idx].identifier == 0x70)
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
}

//==================================================================================================

void handleTelemetry()
{
  //-- simulated telemetry
  if(Sys.DBG_simulateTelemetry)
  {
    //check against configured Ids and copy to telemetryReceivedValue 
    for(uint8_t idx = 0; idx < NUM_CUSTOM_TELEMETRY; idx++)
    {
      if(Model.Telemetry[idx].identifier == 0x30)
      {
        telemetryReceivedValue[idx] = mixSources[SRC_VIRTUAL_FIRST] / 5;
        telemetryLastReceivedValue[idx] = telemetryReceivedValue[idx];
        telemetryLastReceivedTime[idx] = millis();
      }
    }
  }
  
  //-- stats
  for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
  {
    //-- get stats
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
    if(millis() - telemetryLastReceivedTime[i] > 2500)
      telemetryReceivedValue[i] = TELEMETRY_NO_DATA; 
  }
  
  //-- reset all telemetry on model change
  static uint8_t lastModelIdx = 0xff;
  if(Sys.activeModelIdx != lastModelIdx)
  {
    lastModelIdx = Sys.activeModelIdx;
    for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
    {
      telemetryLastReceivedTime[i] = millis();
      telemetryLastReceivedValue[i] = TELEMETRY_NO_DATA;
      telemetryReceivedValue[i] = TELEMETRY_NO_DATA;
      telemetryMaxReceivedValue[i] = TELEMETRY_NO_DATA;
      telemetryMinReceivedValue[i] = TELEMETRY_NO_DATA;
    }
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
    showMsg(PSTR("Shutting down"));
    if(Sys.backlightEnabled)
      analogWrite(PIN_LCD_BACKLIGHT, ((uint16_t) 255 * Sys.backlightLevel) / 100);
    delay(1000);
    //save changes
    if(eeStoreIsInitialised())
    {
      eeSaveSysConfig();
      eeSaveModelData(Sys.activeModelIdx);
    }
    showMsg(PSTR(""));
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

