#include "Arduino.h"

#include "../config.h"
#include "common.h"
#include "inputs.h"

int16_t deadzoneAndMap(int16_t input, int16_t minVal, int16_t centreVal, int16_t maxVal, int16_t deadzn, int16_t mapMin, int16_t mapMax);

const uint8_t swPin[MAX_NUM_PHYSICAL_SWITCHES][2] = {
  {PIN_SWA_UP, PIN_SWA_DN},
  {PIN_SWB_UP, PIN_SWB_DN},
  {PIN_SWC_UP, PIN_SWC_DN},
  {PIN_SWD_UP, PIN_SWD_DN},
  {PIN_SWE_UP, PIN_SWE_DN},
  {PIN_SWF_UP, PIN_SWF_DN},
  {PIN_SWG_UP, PIN_SWG_DN},
  {PIN_SWH_UP, PIN_SWH_DN}
};

//==================================================================================================

void initialiseSwitches()
{
  for(uint8_t i = 0; i < MAX_NUM_PHYSICAL_SWITCHES; i++)
  {
    pinMode(swPin[i][0], INPUT_PULLUP);
    pinMode(swPin[i][1], INPUT_PULLUP);
  }
}

//==================================================================================================

void readSwitchesAndButtons()
{
  //--Read switches--
  //These are read on every other call to the function, as an extra debounce measure,
  //at the expense of slight additional delay/lag.
  static bool read;
  read = !read;
  if(read)
  {
    for(uint8_t i = 0; i < MAX_NUM_PHYSICAL_SWITCHES; i++)
    {
      swState[i] = SWUPPERPOS;
      if(Sys.swType[i] == SW_2POS) 
      {
        if(!digitalRead(swPin[i][1])) 
          swState[i] = SWLOWERPOS;
      }
      else if(Sys.swType[i] == SW_3POS) 
      {
        swState[i] = SWMIDPOS;
        if(!digitalRead(swPin[i][0]))      
          swState[i] = SWUPPERPOS;
        else if(!digitalRead(swPin[i][1])) 
          swState[i] = SWLOWERPOS;
      }
    }
  }

  //--Read trims and ui buttons--
  //Only one button can be pressed at a time
  
  buttonCode = 0;

  if(!(PINx_TRIMS & 0x80)) buttonCode = KEY_X2_TRIM_UP;
  if(!(PINx_TRIMS & 0x40)) buttonCode = KEY_X2_TRIM_DOWN;
  if(!(PINx_TRIMS & 0x20)) buttonCode = KEY_Y2_TRIM_UP;
  if(!(PINx_TRIMS & 0x10)) buttonCode = KEY_Y2_TRIM_DOWN;
  if(!(PINx_TRIMS & 0x08)) buttonCode = KEY_Y1_TRIM_UP;
  if(!(PINx_TRIMS & 0x04)) buttonCode = KEY_Y1_TRIM_DOWN;
  if(!(PINx_TRIMS & 0x02)) buttonCode = KEY_X1_TRIM_UP;
  if(!(PINx_TRIMS & 0x01)) buttonCode = KEY_X1_TRIM_DOWN;

  if(!digitalRead(PIN_KEY_SELECT)) buttonCode = KEY_SELECT;
  if(!digitalRead(PIN_KEY_UP))     buttonCode = KEY_UP;
  if(!digitalRead(PIN_KEY_DOWN))   buttonCode = KEY_DOWN;

  if(buttonCode != 0)
    inputsLastMoved = millis();
  
  //-- play audio when switches are moved --
  uint8_t switchesSum = 0;
  for(uint8_t i = 0; i < MAX_NUM_PHYSICAL_SWITCHES; i++)
  {
    switchesSum += swState[i];
  }
  static uint8_t lastSwitchesSum = switchesSum;
  if(switchesSum != lastSwitchesSum)
  { 
    if(Sys.soundSwitches) 
      audioToPlay = AUDIO_SWITCH_MOVED;
    
    lastSwitchesSum = switchesSum;
    inputsLastMoved = millis();
  }
  
}

//==================================================================================================

static bool buttonActive = false;
static uint8_t lastButtonCode = 0;
static bool buttonEventsOverridden = false;

void determineButtonEvent()
{
  /* 
  Modifies the pressedButton, clickedButton and heldButton variables, buttonStartTime 
  and buttonReleaseTime.
  Events
  - pressedButton is triggered once when the button goes down
  - clickedButton is triggered when the button is released before heldButton event
  - heldButton is triggered when button is held down long enough  
  */
  
  if(buttonEventsOverridden)
  {
    if(buttonActive)
    {
      buttonActive = false;
      buttonReleaseTime = millis();
    }
    //suppress until buttons are released
    if(buttonCode == 0)
      buttonEventsOverridden = false;
    else
      return;
  }

  //clear pressed and clicked events
  pressedButton = 0;
  clickedButton = 0;
  
  //button just went down
  if(buttonCode > 0 && !buttonActive && (millis() - buttonReleaseTime > 100)) 
  {
    buttonActive = true;
    buttonStartTime = millis();
    pressedButton = buttonCode;  //event
    lastButtonCode = buttonCode; //store the buttonCode
    if(Sys.soundKeys)
      audioToPlay = AUDIO_KEY_PRESSED;
  }
  
  //button is down long enough, trigger heldButton event
  static bool heldButtonActive = false;
  if(buttonActive && (millis() - buttonStartTime > LONGPRESSDELAY))
  {
    heldButton = lastButtonCode; //event
    if(!heldButtonActive)
    {
      heldButtonActive = true;
      heldButtonEntryLoopNum = thisLoopNum; //record the loop number at which it first occured
    }
  }
  
  //button has just been released
  if(buttonCode == 0 && buttonActive)
  {
    buttonActive = false;
    buttonReleaseTime = millis();
    
    //trigger clickedButton event if there was no heldButton
    if(heldButton == 0) 
      clickedButton = lastButtonCode; //event
    
    //clear heldButton event and associated variables
    heldButton = 0; 
    heldButtonActive = false;
  }
}

//==================================================================================================

void killButtonEvents()
{
  pressedButton = 0;
  clickedButton = 0;
  heldButton = 0;
  buttonEventsOverridden = true;
}

//==================================================================================================

void readSticks()
{
  if(isCalibratingSticks)
    return;

  x1AxisIn = analogRead(PIN_X1_AXIS);
  y1AxisIn = analogRead(PIN_Y1_AXIS);
  x2AxisIn = analogRead(PIN_X2_AXIS);
  y2AxisIn = analogRead(PIN_Y2_AXIS);
  x3AxisIn = analogRead(PIN_X3_AXIS);
  y3AxisIn = analogRead(PIN_Y3_AXIS);
  x4AxisIn = analogRead(PIN_X4_AXIS);
  y4AxisIn = analogRead(PIN_Y4_AXIS);
  
  knobAIn = analogRead(PIN_KNOB_A);
  knobBIn = analogRead(PIN_KNOB_B);

  //add deadzone to sticks centers
  
  if(Sys.x1AxisType == STICK_AXIS_SELF_CENTERING)
    x1AxisIn  = deadzoneAndMap(x1AxisIn, Sys.x1AxisMin, Sys.x1AxisCenter, Sys.x1AxisMax, Sys.x1AxisDeadzone, -500, 500);
  else
  {
    x1AxisIn = map(x1AxisIn, Sys.x1AxisMin, Sys.x1AxisMax, -500, 500);
    x1AxisIn = constrain(x1AxisIn, -500, 500);
  }
    
  if(Sys.y1AxisType == STICK_AXIS_SELF_CENTERING)
    y1AxisIn  = deadzoneAndMap(y1AxisIn, Sys.y1AxisMin, Sys.y1AxisCenter, Sys.y1AxisMax, Sys.y1AxisDeadzone, -500, 500);
  else
  {
    y1AxisIn = map(y1AxisIn, Sys.y1AxisMin, Sys.y1AxisMax, -500, 500);
    y1AxisIn = constrain(y1AxisIn, -500, 500);
  }
    
  if(Sys.x2AxisType == STICK_AXIS_SELF_CENTERING)
    x2AxisIn = deadzoneAndMap(x2AxisIn, Sys.x2AxisMin, Sys.x2AxisCenter, Sys.x2AxisMax, Sys.x2AxisDeadzone, -500, 500);
  else
  {
    x2AxisIn = map(x2AxisIn, Sys.x2AxisMin, Sys.x2AxisMax, -500, 500);
    x2AxisIn = constrain(x2AxisIn, -500, 500);
  }
  
  if(Sys.y2AxisType == STICK_AXIS_SELF_CENTERING)
    y2AxisIn = deadzoneAndMap(y2AxisIn, Sys.y2AxisMin, Sys.y2AxisCenter, Sys.y2AxisMax, Sys.y2AxisDeadzone, -500, 500);
  else
  {
    y2AxisIn = map(y2AxisIn, Sys.y2AxisMin, Sys.y2AxisMax, -500, 500);
    y2AxisIn = constrain(y2AxisIn, -500, 500);
  }
  
  if(Sys.x3AxisType == STICK_AXIS_SELF_CENTERING)
    x3AxisIn = deadzoneAndMap(x3AxisIn, Sys.x3AxisMin, Sys.x3AxisCenter, Sys.x3AxisMax, Sys.x3AxisDeadzone, -500, 500);
  else
  {
    x3AxisIn = map(x3AxisIn, Sys.x3AxisMin, Sys.x3AxisMax, -500, 500);
    x3AxisIn = constrain(x3AxisIn, -500, 500);
  }
  
  if(Sys.y3AxisType == STICK_AXIS_SELF_CENTERING)
    y3AxisIn = deadzoneAndMap(y3AxisIn, Sys.y3AxisMin, Sys.y3AxisCenter, Sys.y3AxisMax, Sys.y3AxisDeadzone, -500, 500);
  else
  {
    y3AxisIn = map(y3AxisIn, Sys.y3AxisMin, Sys.y3AxisMax, -500, 500);
    y3AxisIn = constrain(y3AxisIn, -500, 500);
  }
  
  if(Sys.x4AxisType == STICK_AXIS_SELF_CENTERING)
    x4AxisIn = deadzoneAndMap(x4AxisIn, Sys.x4AxisMin, Sys.x4AxisCenter, Sys.x4AxisMax, Sys.x4AxisDeadzone, -500, 500);
  else
  {
    x4AxisIn = map(x4AxisIn, Sys.x4AxisMin, Sys.x4AxisMax, -500, 500);
    x4AxisIn = constrain(x4AxisIn, -500, 500);
  }
  
  if(Sys.y4AxisType == STICK_AXIS_SELF_CENTERING)
    y4AxisIn = deadzoneAndMap(y4AxisIn, Sys.y4AxisMin, Sys.y4AxisCenter, Sys.y4AxisMax, Sys.y4AxisDeadzone, -500, 500);
  else
  {
    y4AxisIn = map(y4AxisIn, Sys.y4AxisMin, Sys.y4AxisMax, -500, 500);
    y4AxisIn = constrain(y4AxisIn, -500, 500);
  }
  
  //add deadband at extremes of knobs for stability
  knobAIn = map(knobAIn, 20, 1003, -500, 500); 
  knobAIn = constrain(knobAIn, -500, 500);
  knobBIn = map(knobBIn, 20, 1003, -500, 500); 
  knobBIn = constrain(knobBIn, -500, 500);

  //play audio whenever knob crosses center 
  enum {POS_SIDE = 0, CENTER = 1, NEG_SIDE = 2};
  static uint8_t knobARegion = CENTER;
  static uint8_t knobBRegion = CENTER;
  
  if((knobAIn >= 0 && knobARegion == NEG_SIDE) || (knobAIn < 0 && knobARegion == POS_SIDE)) //crossed center
  {
    knobARegion = CENTER;
    if(Sys.soundKnobCenter)
      audioToPlay = AUDIO_SWITCH_MOVED;
  }
  else if(knobAIn > 25) knobARegion = POS_SIDE;
  else if(knobAIn < -25) knobARegion = NEG_SIDE;
  
  if((knobBIn >= 0 && knobBRegion == NEG_SIDE) || (knobBIn < 0 && knobBRegion == POS_SIDE)) //crossed center
  {
    knobBRegion = CENTER;
    if(Sys.soundKnobCenter)
      audioToPlay = AUDIO_SWITCH_MOVED;
  }
  else if(knobBIn > 25) knobBRegion = POS_SIDE;
  else if(knobBIn < -25) knobBRegion = NEG_SIDE;
  
  //detect inactivity
  static int16_t lastSticksSum = 0;
  int16_t sticksSum = 0;
  sticksSum += x1AxisIn;
  sticksSum += y1AxisIn;
  sticksSum += x2AxisIn;
  sticksSum += y2AxisIn;
  sticksSum += x3AxisIn;
  sticksSum += y3AxisIn;
  sticksSum += x4AxisIn;
  sticksSum += y4AxisIn;
  sticksSum += knobAIn;
  sticksSum += knobBIn;
  if(abs(sticksSum - lastSticksSum) > 50) //5% of 1000
  {
    lastSticksSum = sticksSum;
    inputsLastMoved = millis();
  }

}

//====================================Helpers=======================================================

int16_t deadzoneAndMap(int16_t input, int16_t minVal, int16_t centreVal, int16_t maxVal, int16_t deadzn, int16_t mapMin, int16_t mapMax)
{
  //Formula is ((maxVal-minVal)*deadzn/100)/2. We divide by 2 as its applied about center
  int16_t deadznVal = ((int32_t)(maxVal - minVal) * deadzn) / 200;
  int16_t threshR = centreVal + deadznVal;
  int16_t threshL = centreVal - deadznVal;
  int16_t mapCenter = (mapMin + mapMax) / 2;
  int16_t output = mapCenter;
  if(input > threshR)
    output = map(input, threshR, maxVal, mapCenter + 1, mapMax);
  else if(input < threshL)
    output = map(input, minVal, threshL, mapMin, mapCenter - 1);
  output = constrain(output, mapMin, mapMax);
  return output;
}

//==================================================================================================

void calibrateSticks(uint8_t stage)
{
  switch(stage)
  {
    case CALIBRATE_INIT:
    {
      //set values to lowest
      Sys.x1AxisMin = 1023; Sys.x1AxisMax = 0;
      Sys.y1AxisMin = 1023; Sys.y1AxisMax = 0;
      Sys.x2AxisMin = 1023; Sys.x2AxisMax = 0;
      Sys.y2AxisMin = 1023; Sys.y2AxisMax = 0;
      Sys.x3AxisMin = 1023; Sys.x3AxisMax = 0;
      Sys.y3AxisMin = 1023; Sys.y3AxisMax = 0;
      Sys.x4AxisMin = 1023; Sys.x4AxisMax = 0;
      Sys.y4AxisMin = 1023; Sys.y4AxisMax = 0;
    }
    break;
    
    case CALIBRATE_MOVE:
    {
      int16_t reading;
      
      //---- get min, max, center
      
      reading = analogRead(PIN_X1_AXIS);
      Sys.x1AxisCenter = reading;
      if(reading < Sys.x1AxisMin)
        Sys.x1AxisMin = reading;
      else if(reading > Sys.x1AxisMax)
        Sys.x1AxisMax = reading;
      
      reading = analogRead(PIN_Y1_AXIS);
      Sys.y1AxisCenter = reading;
      if(reading < Sys.y1AxisMin)
        Sys.y1AxisMin = reading;
      else if(reading > Sys.y1AxisMax)
        Sys.y1AxisMax = reading;
      
      reading = analogRead(PIN_X2_AXIS);
      Sys.x2AxisCenter  = reading;
      if(reading < Sys.x2AxisMin)
        Sys.x2AxisMin = reading;
      else if(reading > Sys.x2AxisMax)
        Sys.x2AxisMax = reading;
      
      reading  = analogRead(PIN_Y2_AXIS);
      Sys.y2AxisCenter = reading;
      if(reading < Sys.y2AxisMin)
        Sys.y2AxisMin = reading;
      else if(reading > Sys.y2AxisMax)
        Sys.y2AxisMax = reading;
      
      reading = analogRead(PIN_X3_AXIS);
      Sys.x3AxisCenter  = reading;
      if(reading < Sys.x3AxisMin)
        Sys.x3AxisMin = reading;
      else if(reading > Sys.x3AxisMax)
        Sys.x3AxisMax = reading;
      
      reading  = analogRead(PIN_Y3_AXIS);
      Sys.y3AxisCenter = reading;
      if(reading < Sys.y3AxisMin)
        Sys.y3AxisMin = reading;
      else if(reading > Sys.y3AxisMax)
        Sys.y3AxisMax = reading;
      
      reading = analogRead(PIN_X4_AXIS);
      Sys.x4AxisCenter  = reading;
      if(reading < Sys.x4AxisMin)
        Sys.x4AxisMin = reading;
      else if(reading > Sys.x4AxisMax)
        Sys.x4AxisMax = reading;
      
      reading  = analogRead(PIN_Y4_AXIS);
      Sys.y4AxisCenter = reading;
      if(reading < Sys.y4AxisMin)
        Sys.y4AxisMin = reading;
      else if(reading > Sys.y4AxisMax)
        Sys.y4AxisMax = reading;
    }
    break;
    
    case CALIBRATE_DEADBAND:
    {
      //Add slight deadband(about 1.5%) at each stick ends to stabilise readings at ends
      //For a range of 0 to 5V, min max are 0.07V and 4.92V
      
      int16_t deadBand; 
      
      deadBand = (Sys.x1AxisMax - Sys.x1AxisMin) / 64;
      Sys.x1AxisMax -= deadBand;
      Sys.x1AxisMin += deadBand;
      
      deadBand = (Sys.y1AxisMax - Sys.y1AxisMin) / 64;
      Sys.y1AxisMax -= deadBand;
      Sys.y1AxisMin += deadBand;

      deadBand = (Sys.x2AxisMax - Sys.x2AxisMin) / 64;
      Sys.x2AxisMax -= deadBand;
      Sys.x2AxisMin += deadBand;
      
      deadBand = (Sys.y2AxisMax - Sys.y2AxisMin) / 64;
      Sys.y2AxisMax -= deadBand;
      Sys.y2AxisMin += deadBand;
      
      deadBand = (Sys.x3AxisMax - Sys.x3AxisMin) / 64;
      Sys.x3AxisMax -= deadBand;
      Sys.x3AxisMin += deadBand;
      
      deadBand = (Sys.y3AxisMax - Sys.y3AxisMin) / 64;
      Sys.y3AxisMax -= deadBand;
      Sys.y3AxisMin += deadBand;
      
      deadBand = (Sys.x4AxisMax - Sys.x4AxisMin) / 64;
      Sys.x4AxisMax -= deadBand;
      Sys.x4AxisMin += deadBand;
      
      deadBand = (Sys.y4AxisMax - Sys.y4AxisMin) / 64;
      Sys.y4AxisMax -= deadBand;
      Sys.y4AxisMin += deadBand;
    }
    break;
  }
}

