
#include "Arduino.h"
#include <Servo.h>

#include "config.h"
#include "crc.h"
#include "common.h"
#include "eestore.h"
#include "rfComm.h"

//Declare an array of servo objects
Servo myServo[MAX_CHANNELS_PER_RECEIVER];

//Declare an output pins array
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
uint8_t getMaxOutputChConfig(int16_t pin);

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
    
    maxOutputChConfig[i] = getMaxOutputChConfig(outputPin[i]);
    if(maxOutputChConfig[i] >= CFG_SERVO) 
      Sys.outputChConfig[i] = CFG_SERVO;
    else 
      Sys.outputChConfig[i] = CFG_DIGITAL;
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
  
  //--- setup outputs. Changes in output configuration are applied on the fly
  static uint8_t prevOutputChConfig[MAX_CHANNELS_PER_RECEIVER];
  static bool initialised = false;
  if(!initialised)
  {
    for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
      prevOutputChConfig[i] = 0xFF;
    initialised = true;
  }
  for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
  {
    if(Sys.outputChConfig[i] != prevOutputChConfig[i])
    {
      if(prevOutputChConfig[i] == CFG_SERVO) //detach servo
        myServo[i].detach();
      else if(prevOutputChConfig[i] == CFG_PWM) //halt the pwm
        digitalWrite(outputPin[i], LOW);
        
      if(Sys.outputChConfig[i] == CFG_DIGITAL)
        pinMode(outputPin[i], OUTPUT);
      else if(Sys.outputChConfig[i] == CFG_SERVO)
        myServo[i].attach(outputPin[i]);

      prevOutputChConfig[i] = Sys.outputChConfig[i];
    }
  }
  
  //--- handle failsafe
  static bool failsafeActivated = false;
  static bool outputsReinitialized = true;
  if(millis() - lastRCPacketMillis > 1000)
  {
    if(!failsafeActivated)
    {
      failsafeActivated = true;
      outputsReinitialized = false;
      for(uint8_t i= 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
      {
        if(channelFailsafe[i] == 523) //Hold
        {
          //Nothing here. 
          //Receiver will continue outputing the last received value before signal was lost.
        }
        else if(channelFailsafe[i] == 522) //No Pulses
        {
          if(Sys.outputChConfig[i] == CFG_SERVO)
          {
            myServo[i].detach();
            pinMode(outputPin[i], OUTPUT);
            digitalWrite(outputPin[i], LOW);
          }
          else if(Sys.outputChConfig[i] == CFG_PWM || Sys.outputChConfig[i] == CFG_DIGITAL)
            digitalWrite(outputPin[i], LOW);
        }
        else //Custom value
          channelOut[i] = channelFailsafe[i];
      }
    }
  }
  else
  {
    failsafeActivated = false;
    if(!outputsReinitialized)
    {
      outputsReinitialized = true;
      for(uint8_t i= 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
      {
        if(Sys.outputChConfig[i] == CFG_SERVO)
        {
          if(channelFailsafe[i] == 522) //only attach again for those with 'no pulses' specified
            myServo[i].attach(outputPin[i]);
        }
        else if(Sys.outputChConfig[i] == CFG_PWM || Sys.outputChConfig[i] == CFG_DIGITAL)
        {
          //Nothing here, No need.
        } 
      }
    }
  }
  
  //--- write outputs
  for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
  {
    if(failsafeActivated && channelFailsafe[i] == 522) //No pulse. Already handled
      continue;

    if(Sys.outputChConfig[i] == CFG_DIGITAL)
    {
      //range -500 to -250 is LOW, -250 to 250 is ignored, 250 to 500 is HIGH
      if(channelOut[i] <= -250)
        digitalWrite(outputPin[i], LOW);
      else if(channelOut[i] >= 250)
        digitalWrite(outputPin[i], HIGH);
    }
    else if(Sys.outputChConfig[i] == CFG_SERVO)
    {
      int16_t val = map(channelOut[i], -500, 500, 1000, 2000);
      val = constrain(val, 1000, 2000);
      myServo[i].writeMicroseconds(val);
    }
    else if(Sys.outputChConfig[i] == CFG_PWM)
    {
      int16_t val = map(channelOut[i], -500, 500, 0, 255);
      val = constrain(val, 0, 255);
      analogWrite(outputPin[i], val);
    }
  }
}

//==================================================================================================

uint8_t getMaxOutputChConfig(int16_t pin)
{
  uint8_t rslt = CFG_SERVO; //assumed //### TODO Check within servo objects
  
  int16_t pwmPins[] = {5, 6, 3, 11}; //on arduino uno. Pins 9 and 10 omitted due to servo lib
  //search through array
  for(uint8_t i = 0; i < (sizeof(pwmPins)/sizeof(pwmPins[0])); i++)
  {
    if(pwmPins[i] == pin)
    {
      rslt = CFG_PWM;
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

