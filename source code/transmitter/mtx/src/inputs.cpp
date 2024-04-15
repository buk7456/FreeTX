#include "Arduino.h"

#include "../config.h"
#include "common.h"
#include "inputs.h"

int16_t deadzoneAndMap(int16_t input, int16_t minVal, int16_t centreVal, int16_t maxVal, int16_t deadzn, int16_t mapMin, int16_t mapMax);

const uint8_t swPin[NUM_PHYSICAL_SWITCHES][2] = {
  {PIN_SWA_UP, PIN_SWA_DN},
  {PIN_SWB_UP, PIN_SWB_DN},
  {PIN_SWC_UP, PIN_SWC_DN},
  {PIN_SWD_UP, PIN_SWD_DN},
  {PIN_SWE_UP, PIN_SWE_DN},
  {PIN_SWF_UP, PIN_SWF_DN},
  {PIN_SWG_UP, PIN_SWG_DN},
  {PIN_SWH_UP, PIN_SWH_DN}
};

const uint8_t stickAxisPin[NUM_STICK_AXES] = {
  PIN_X1_AXIS,
  PIN_Y1_AXIS,
  PIN_Z1_AXIS,
  PIN_X2_AXIS,
  PIN_Y2_AXIS,
  PIN_Z2_AXIS,
  PIN_X3_AXIS,
  PIN_Y3_AXIS,
  PIN_X4_AXIS,
  PIN_Y4_AXIS
};

//==================================================================================================

void initialiseSwitches()
{
  for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
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
    for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
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
  for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
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

  //Read stick axes

  for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
  {
    stick_axis_params_t *axis = &Sys.StickAxis[i];
    if(axis->type == STICK_AXIS_ABSENT)
    {
      stickAxisIn[i] = 0;
    }
    else
    {
      stickAxisIn[i] = analogRead(stickAxisPin[i]);
      if(axis->type == STICK_AXIS_SELF_CENTERING)
      {
        stickAxisIn[i] = deadzoneAndMap(stickAxisIn[i], axis->minVal, axis->centerVal, axis->maxVal, 
                                        axis->deadzone, -500, 500);
      }
      else if(axis->type == STICK_AXIS_NON_CENTERING)
      {
        stickAxisIn[i] = map(stickAxisIn[i], axis->minVal, axis->maxVal, -500, 500);
        stickAxisIn[i] = constrain(stickAxisIn[i], -500, 500);
      }
    }
  }

  //Read knobs, add deadband at extremes to prevent jitter

  knobAIn = analogRead(PIN_KNOB_A);
  knobAIn = map(knobAIn, 20, 1003, -500, 500); 
  knobAIn = constrain(knobAIn, -500, 500);

  knobBIn = analogRead(PIN_KNOB_B);
  knobBIn = map(knobBIn, 20, 1003, -500, 500); 
  knobBIn = constrain(knobBIn, -500, 500);

  //play audio whenever knob crosses center 

  enum {
    NEG_SIDE,
    CENTER, 
    POS_SIDE, 
  };

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
  
  //Detect inactivity
  static int16_t lastSum = 0;
  int16_t sum = 0;
  for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
    sum += stickAxisIn[i];
  sum += knobAIn;
  sum += knobBIn;
  if(abs(sum - lastSum) > 50) //5% of 1000
  {
    lastSum = sum;
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
      //set min to highest and max to lowest
      for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
      {
        Sys.StickAxis[i].minVal = 1023;
        Sys.StickAxis[i].maxVal = 0;
      }
    }
    break;
    
    case CALIBRATE_MOVE:
    {
      //get min, max, center
      int16_t reading;
      for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
      {
        if(Sys.StickAxis[i].type == STICK_AXIS_ABSENT)
          continue;
        reading = analogRead(stickAxisPin[i]);
        Sys.StickAxis[i].centerVal = reading;
        if(reading < Sys.StickAxis[i].minVal)
          Sys.StickAxis[i].minVal = reading;
        else if(reading > Sys.StickAxis[i].maxVal)
          Sys.StickAxis[i].maxVal = reading;
      }
    }
    break;
    
    case CALIBRATE_DEADBAND:
    {
      //Add slight deadband(about 1.5%) at each stick ends to stabilise readings at ends
      //For a range of 0 to 5V, min max are 0.07V and 4.92V
      int16_t deadBand; 
      for(uint8_t i = 0; i < NUM_STICK_AXES; i++)
      {
        deadBand = (Sys.StickAxis[i].maxVal -Sys.StickAxis[i].minVal) / 64;
        Sys.StickAxis[i].maxVal -= deadBand;
        Sys.StickAxis[i].minVal += deadBand;
      }
    }
    break;
  }
}

