
#include "Arduino.h"
// #include <Servo.h> //replaced with modified library tailored to our application
#include "Servo.h"

#include "config.h"
#include "crc.h"
#include "common.h"
#include "eestore.h"
#include "rfComm.h"

//array of servo objects
Servo myServo[MAX_CHANNELS_PER_RECEIVER];

//output pins array
int16_t outputPin[MAX_CHANNELS_PER_RECEIVER] = {
  PIN_CH1,
  PIN_CH2,
  PIN_CH3, 
  PIN_CH4, 
  PIN_CH5, 
  PIN_CH6, 
  PIN_CH7, 
  PIN_CH8,
  PIN_CH9,
  PIN_CH10
};

//--------------- Function Declarations ----------

void writeOutputs();
void getExternalVoltage();
uint8_t getMaxSignalType(int16_t pin);

//==================================================================================================

void setup()
{ 
  //--- setup pins
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  
  //--- use analog reference internal 1.1V
  analogReference(INTERNAL);
  
  //--- initialise values
  for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; ++i)
  {
    channelOut[i] = 0;
    channelFailsafe[i] = 0;
    
    Sys.outputChConfig[i] = 0xA0;
    uint8_t maxSignalType = getMaxSignalType(outputPin[i]);
    Sys.outputChConfig[i] |= (maxSignalType << 2) & 0x0C;
    if(maxSignalType >= SIGNAL_TYPE_SERVOPWM) 
      Sys.outputChConfig[i] |= SIGNAL_TYPE_SERVOPWM & 0x03;
  }
  
  //--- delay
  delay(100);

  //--- eeprom 
  eeStoreInit();
  eeReadSysConfig();
 
  //--- initialise radio module
  initialiseRfModule();
  
  //--- enter bind mode
  isRequestingBind = true;
}

//==================================================================================================

void loop()
{
  //--- COMMUNICATION
  doRfCommunication();

  //--- RC OUTPUTS
  writeOutputs();

  //--- EXTERNAL VOLTAGE
  getExternalVoltage();
}

//==================================================================================================

void writeOutputs()
{ 
  if(!failsafeEverBeenReceived)
    return;
  
  static uint8_t prevSignalType[MAX_CHANNELS_PER_RECEIVER];
  static uint8_t prevServoPWMRangeIdx[MAX_CHANNELS_PER_RECEIVER];
  static bool    initialised[MAX_CHANNELS_PER_RECEIVER];
  static bool    failsafeActivated[MAX_CHANNELS_PER_RECEIVER];
  static bool    outputReinitialised[MAX_CHANNELS_PER_RECEIVER];
  
  bool activateFailsafe = ((millis() - lastRCPacketMillis) > 1000);
  
  for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
  {
    //--- SETUP OUTPUTS
    
    if(!initialised[i])
    {
      initialised[i] = true;
      prevSignalType[i] = 0xFF;
      prevServoPWMRangeIdx[i] = 0xFF;
      failsafeActivated[i] = false;
      outputReinitialised[i] = true;
    }
    
    uint8_t signalType = Sys.outputChConfig[i] & 0x03;
    
    uint8_t servoPWMRangeIdx = (Sys.outputChConfig[i] >> 4) & 0x0F;
    int16_t minMicroseconds = 500 + (servoPWMRangeIdx * 50);
    int16_t maxMicroseconds = 2500 - (servoPWMRangeIdx * 50);
    
    if(signalType != prevSignalType[i])
    {
      if(prevSignalType[i] == SIGNAL_TYPE_SERVOPWM)
        myServo[i].detach();
      else if(prevSignalType[i] == SIGNAL_TYPE_PWM)
        digitalWrite(outputPin[i], LOW);
        
      if(signalType == SIGNAL_TYPE_DIGITAL)
        pinMode(outputPin[i], OUTPUT);
      else if(signalType == SIGNAL_TYPE_SERVOPWM)
        myServo[i].attach(outputPin[i], minMicroseconds, maxMicroseconds);

      prevSignalType[i] = signalType;
    }
    
    if(signalType == SIGNAL_TYPE_SERVOPWM && servoPWMRangeIdx != prevServoPWMRangeIdx[i])
    {
      myServo[i].detach();
      myServo[i].attach(outputPin[i], minMicroseconds, maxMicroseconds);
      prevServoPWMRangeIdx[i] = servoPWMRangeIdx;
    }
    
    //--- HANDLE FAILSAFE
    
    if(activateFailsafe)
    {
      if(!failsafeActivated[i])
      {
        failsafeActivated[i] = true;
        outputReinitialised[i] = false;
        
        if(channelFailsafe[i] == 523) //Hold
        {
          //Nothing here. 
          //Receiver will continue outputing the last received value before signal was lost.
        }
        else if(channelFailsafe[i] == 522) //No Pulses
        {
          if(signalType == SIGNAL_TYPE_SERVOPWM)
          {
            myServo[i].detach();
            pinMode(outputPin[i], OUTPUT);
            digitalWrite(outputPin[i], LOW);
          }
          else if(signalType == SIGNAL_TYPE_PWM || signalType == SIGNAL_TYPE_DIGITAL)
            digitalWrite(outputPin[i], LOW);
        }
        else //Custom value
          channelOut[i] = channelFailsafe[i];
      }
    }
    else
    {
      failsafeActivated[i] = false;
      if(!outputReinitialised[i])
      {
        outputReinitialised[i] = true;
        if(signalType == SIGNAL_TYPE_SERVOPWM)
        {
          if(channelFailsafe[i] == 522) //only attach again for those with 'no pulses' specified
            myServo[i].attach(outputPin[i], minMicroseconds, maxMicroseconds);
        }
        else if(signalType == SIGNAL_TYPE_PWM || signalType == SIGNAL_TYPE_DIGITAL)
        {
          //Nothing here, No need.
        } 
      }
    }

    //--- WRITE OUTPUTS
    
    if(failsafeActivated[i] && channelFailsafe[i] == 522) //No pulse. Already handled
      continue;

    if(signalType == SIGNAL_TYPE_DIGITAL)
    {
      //range -500 to -250 is LOW, -250 to 250 is ignored, 250 to 500 is HIGH
      if(channelOut[i] <= -250)
        digitalWrite(outputPin[i], LOW);
      else if(channelOut[i] >= 250)
        digitalWrite(outputPin[i], HIGH);
    }
    else if(signalType == SIGNAL_TYPE_SERVOPWM)
    {
      int16_t val = map(channelOut[i], -500, 500, minMicroseconds, maxMicroseconds);
      val = constrain(val, minMicroseconds, maxMicroseconds);
      myServo[i].writeMicroseconds(val);
    }
    else if(signalType == SIGNAL_TYPE_PWM)
    {
      int16_t val = map(channelOut[i], -500, 500, 0, 255);
      val = constrain(val, 0, 255);
      analogWrite(outputPin[i], val);
    }
  }
}

//==================================================================================================

uint8_t getMaxSignalType(int16_t pin)
{
  uint8_t rslt = SIGNAL_TYPE_SERVOPWM; //assumed //### TODO Check within servo objects
  
  int16_t pwmPins[] = {5, 6, 3, 11}; //on arduino uno. Pins 9 and 10 omitted due to servo lib
  //search through array
  for(uint8_t i = 0; i < (sizeof(pwmPins)/sizeof(pwmPins[0])); i++)
  {
    if(pwmPins[i] == pin)
    {
      rslt = SIGNAL_TYPE_PWM;
      break;
    }
  }
  return rslt;
}

//==================================================================================================

void getExternalVoltage()
{
  /* 
  Apply smoothing to measurement using exponential recursive smoothing
  It works by subtracting out the mean each time, and adding in a new point. 
  _NUM_SAMPLES parameter defines number of samples to average over. Higher value results in slower
  response.
  Formula x = x - x/n + a/n  
  */
  static uint32_t lastMillis = 0;
  if(millis() - lastMillis < 10)
    return;

  lastMillis = millis();
  
  const int16_t _NUM_SAMPLES = 30;
  int32_t millivolts = ((int32_t)analogRead(PIN_EXTV_SENSE) * battVfactor) / 100;
  millivolts = ((int32_t)externalVolts * (_NUM_SAMPLES - 1) + millivolts) / _NUM_SAMPLES; 
  externalVolts = int16_t(millivolts); 
}

