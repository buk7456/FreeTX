#include "Arduino.h"

#include "../config.h"
#include "tonePlayer.h"
#include "common.h"

//--- Note defines ---
#define NOTE_REST 0
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951

//--- Sounds ---

// batteryLow:d=4,o=5,b=290:4p,4c6,32p,4a#,32p,4g.
const uint16_t batteryLowSound[] PROGMEM  = {
  290,NOTE_C6,4,NOTE_REST,32,NOTE_AS5,4,NOTE_REST,32,NOTE_G5,4
}; 

// warning:d=4,o=4,b=120:16p,8e5
const uint16_t warningSound[] PROGMEM = {
  120,NOTE_REST,16,NOTE_E5,8
};

// shortBeep:d=4,o=4,b=250:16d5
const uint16_t shortBeepSound[] PROGMEM = {
  250,NOTE_D5,16
}; 

// timerElapsed:d=4,o=5,b=210:16p,16b6,16p,8b6
const uint16_t timerElapsedSound[] PROGMEM = {
  210,NOTE_REST,16,NOTE_B6,16,NOTE_REST,16,NOTE_B6,8
}; 

// idle:d=4,o=5,b=90:8p,32c5,32g4,32c5
const uint16_t inactivitySound[] PROGMEM = {
  90,NOTE_REST,8,NOTE_C5,32,NOTE_G4,32,NOTE_C5,32
}; 

// telemWarning:d=4,o=5,b=120:8p,4a#4
const uint16_t telemWarningSound[] PROGMEM = {
  120,NOTE_REST,8,NOTE_AS4,4
}; 

// telemMute:d=4,o=4,b=160:8p,16c5,16e5
const uint16_t telemMuteSound[] PROGMEM = {
  160,NOTE_REST,8,NOTE_C5,16,NOTE_E5,16
}; 

// bind:d=4,o=5,b=75:32d#5,32g5,32a#5,16d6
const uint16_t bindSound[] PROGMEM = {
  75,NOTE_DS5,32,NOTE_G5,32,NOTE_AS5,32,NOTE_D6,16
}; 

// trimMoved:d=4,o=4,b=250:16a#7
const uint16_t trimMovedSound[] PROGMEM = {
  250,NOTE_AS7,16
}; 

// trimCentered:d=4,o=4,b=250:8a#7
const uint16_t trimCenteredSound[] PROGMEM = {
  250,NOTE_AS7,8
}; 

//trimEntered:d=4,o=5,b=120:32d#5,32g5,32a5
const uint16_t trimEnteredSound[] PROGMEM = {
  120, NOTE_DS5, 32, NOTE_G5, 32, NOTE_A5, 32
};

//trimExited:d=4,o=5,b=120:32a5,32g5,32d#5
const uint16_t trimExitedSound[] PROGMEM = {
  120, NOTE_A5, 32, NOTE_G5, 32, NOTE_DS5, 32
};

//trimX1:d=4,o=4,b=160:16a5
const uint16_t trimX1Sound[] PROGMEM = {
  160, NOTE_A5, 16
};

//trimY1:d=4,o=4,b=160:16b5
const uint16_t trimY1Sound[] PROGMEM = {
  160, NOTE_B5, 16
};

//trimX2:d=4,o=4,b=160:16c6
const uint16_t trimX2Sound[] PROGMEM = {
  160, NOTE_C6, 16
};

//trimY2:d=4,o=4,b=160:16d6
const uint16_t trimY2Sound[] PROGMEM = {
  160, NOTE_D6, 16
};

// screenCapture:d=4,o=5,b=125:32p,32c#4,32d4
const uint16_t screenshotSound[] PROGMEM = {
  125, NOTE_REST, 32, NOTE_CS4, 32, NOTE_D4, 32
};

// notifications tones
//if you add more, change NOTIFICATION_TONE_COUNT to match
/*
  tone:d=4,o=5,b=125:32p,16b6,8e7,16b7
  tone:d=4,o=5,b=125:32p,16f#7,16e7,16d#7,16c#7,16b6,16c#7,16d#7,16e7,16f#7
  tone:d=4,o=5,b=125:32p,16g6,32p,16c7
  tone:d=4,o=5,b=125:32p,16a6,16e7,16c#7
  tone:d=4,o=5,b=125:32p,8g#7,8c#7,8g#7,8c#7
*/
const uint16_t notificationTone[][32] PROGMEM = {
  {125, NOTE_REST, 32, NOTE_B6, 16, NOTE_E7, 8, NOTE_B7, 16},
  {125, NOTE_REST, 32, NOTE_FS7, 16, NOTE_E7, 16, NOTE_DS7, 16, NOTE_CS7, 16, NOTE_B6, 16, NOTE_CS7, 16, NOTE_DS7, 16, NOTE_E7, 16, NOTE_FS7, 16},
  {125, NOTE_REST, 32, NOTE_G6, 16, NOTE_REST, 32, NOTE_C7, 16},
  {125, NOTE_REST, 32, NOTE_A6, 16, NOTE_E7, 16, NOTE_CS7, 16},
  {125, NOTE_REST, 32, NOTE_GS7, 8, NOTE_CS7, 8, NOTE_GS7, 8, NOTE_CS7, 8}
};

//--- Function declarations ---

void beginTone(const uint16_t* toneArray, size_t iSize);
void beginToneTrimMoved();
void playback();
void stopPlayback();

//--- variables ----

uint32_t endTimeOfNote = 0;
bool isPlaying = false;
int16_t bpm;
uint8_t postn = 0;
size_t size = 0;

const uint16_t* songStart;

bool isTrimBeepSound = false; 

//--------------------------------------------------------------------------------------------------

void beginTone(const uint16_t* toneArray, size_t iSize)
{
  noTone(PIN_BUZZER); //stop any notes
  //init values
  size = iSize;
  endTimeOfNote = 0;
  songStart = toneArray;
  bpm = pgm_read_word(songStart);
  postn = 1;
  isPlaying = true;
  isTrimBeepSound = false;
}

//--------------------------------------------------------------------------------------------------

void beginToneTrimMoved()
{
  noTone(PIN_BUZZER); //stop any notes
  //init values
  endTimeOfNote = 0;
  bpm = 250;
  postn = 1; //TODO: clean up, this is a hack
  size = 2;  //TODO: clean up, this is a hack
  isPlaying = true;
  isTrimBeepSound = true;
}

//--------------------------------------------------------------------------------------------------

void playback()
{
  uint32_t currTime = millis();
  
  if(!isPlaying || currTime < endTimeOfNote)
    return;
  if(postn >= size) //end reached, stop
  {
    stopPlayback();
    return;
  }
  
  if(isTrimBeepSound)
  {
    int16_t freq = map(audioTrimVal, TRIM_MIN_VAL, TRIM_MAX_VAL, NOTE_AS6, NOTE_AS7);
    int16_t duration = (audioTrimVal == 0) ? 80 : 40;
    endTimeOfNote = currTime + duration;
    postn += 2;
    if(freq)
      tone(PIN_BUZZER, freq, duration);
  }
  else
  {
    noTone(PIN_BUZZER);
    int16_t note = pgm_read_word(songStart + postn);
    /* 
    int16_t wholenote = (60 * 1000L / bpm) * 4;  // time for whole note in milliseconds
    int16_t duration  = wholenote / pgm_read_word(songStart + postn + 1); 
    */
    int16_t duration = (bpm == 0) ? 0 : (240000L / (bpm * pgm_read_word(songStart + postn + 1)));
    endTimeOfNote = currTime + duration;
    postn += 2;
    if(note)
      tone(PIN_BUZZER, note, duration);
  }
}

//--------------------------------------------------------------------------------------------------

void stopPlayback()
{
  if(isPlaying)
  {
    noTone(PIN_BUZZER);
    isPlaying = false;
    isTrimBeepSound = false;
  }
}

//==================================================================================================

void playTones()
{
  if(!Sys.soundEnabled)
    audioToPlay = AUDIO_NONE;

  static uint8_t lastAudioToPlay = AUDIO_NONE;
  if(audioToPlay != lastAudioToPlay) //initialise playback with the specified audio
  {
    lastAudioToPlay = audioToPlay;
    
    switch(audioToPlay)
    {
      case AUDIO_SAFETY_WARNING:
        beginTone(warningSound, sizeof(warningSound)/sizeof(warningSound[0]));
        break;
        
      case AUDIO_BATTERY_WARNING:
        beginTone(batteryLowSound, sizeof(batteryLowSound)/sizeof(batteryLowSound[0])); 
        break;
        
      case AUDIO_TIMER_ELAPSED:
        beginTone(timerElapsedSound, sizeof(timerElapsedSound)/sizeof(timerElapsedSound[0]));
        break;
      
      case AUDIO_KEY_PRESSED:
      case AUDIO_SWITCH_MOVED:
      case AUDIO_KNOB_CENTERED:
        beginTone(shortBeepSound, sizeof(shortBeepSound)/sizeof(shortBeepSound[0])); 
        break;
        
      case AUDIO_INACTIVITY:
        beginTone(inactivitySound, sizeof(inactivitySound)/sizeof(inactivitySound[0]));
        break;
        
      case AUDIO_TELEM_WARNING: 
        beginTone(telemWarningSound, sizeof(telemWarningSound)/sizeof(telemWarningSound[0]));
        break;
        
      case AUDIO_TELEM_MUTE_CHANGED: 
        beginTone(telemMuteSound, sizeof(telemMuteSound)/sizeof(telemMuteSound[0]));
        break;
        
      case AUDIO_BIND_SUCCESS:
        beginTone(bindSound, sizeof(bindSound)/sizeof(bindSound[0]));
        break;
        
      case AUDIO_SCREENSHOT_CAPTURED: 
        beginTone(screenshotSound, sizeof(screenshotSound)/sizeof(screenshotSound[0]));
        break;
        
      case AUDIO_TRIM_MOVED:
      case AUDIO_TRIM_CENTERED:
        {
          if(Sys.trimToneFreqMode == TRIM_TONE_FREQ_FIXED)
          {
            if(audioToPlay == AUDIO_TRIM_MOVED)
              beginTone(trimMovedSound, sizeof(trimMovedSound)/sizeof(trimMovedSound[0]));
            if(audioToPlay == AUDIO_TRIM_CENTERED)
              beginTone(trimCenteredSound, sizeof(trimCenteredSound)/sizeof(trimCenteredSound[0]));
          }
          else if(Sys.trimToneFreqMode == TRIM_TONE_FREQ_VARIABLE)
            beginToneTrimMoved();
        }
        break;

      case AUDIO_TRIM_MODE_ENTERED: 
        beginTone(trimEnteredSound, sizeof(trimEnteredSound)/sizeof(trimEnteredSound[0]));
        break;
        
      case AUDIO_TRIM_MODE_EXITED: 
        beginTone(trimExitedSound, sizeof(trimExitedSound)/sizeof(trimExitedSound[0]));
        break;
        
      case AUDIO_TRIM_MODE_X1:
        beginTone(trimX1Sound, sizeof(trimX1Sound)/sizeof(trimX1Sound[0]));
        break;
        
      case AUDIO_TRIM_MODE_Y1:
        beginTone(trimY1Sound, sizeof(trimY1Sound)/sizeof(trimY1Sound[0]));
        break;
        
      case AUDIO_TRIM_MODE_X2: 
        beginTone(trimX2Sound, sizeof(trimX2Sound)/sizeof(trimX2Sound[0]));
        break;
        
      case AUDIO_TRIM_MODE_Y2: 
        beginTone(trimY2Sound, sizeof(trimY2Sound)/sizeof(trimY2Sound[0]));
        break;
      
      default:
        {
          if(audioToPlay >= AUDIO_NOTIFICATION_TONE_FIRST && audioToPlay <= AUDIO_NOTIFICATION_TONE_LAST)
          {
            uint8_t i = audioToPlay - AUDIO_NOTIFICATION_TONE_FIRST;
            beginTone(notificationTone[i], sizeof(notificationTone[0])/sizeof(notificationTone[0][0]));
          }
        }
        break;
    }
  }
  else //Playback. Automatically stops once all notes have been played
  {
    playback();
  }
    
  audioToPlay = AUDIO_NONE;
}

//==================================================================================================

void stopTones()
{
  stopPlayback();
}
