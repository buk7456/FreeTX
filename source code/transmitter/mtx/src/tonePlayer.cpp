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

// "txbattlow:d=4,o=5,b=290:4p,4c6,32p,4a#,32p,4g."
static const uint16_t PROGMEM battLowSound[]  = {
  290,NOTE_C6,4,NOTE_REST,32,NOTE_AS5,4,NOTE_REST,32,NOTE_G5,4
}; 

// "warn:d=4,o=4,b=120:16p,8e5"
static const uint16_t PROGMEM warnSound[] = {
  120,NOTE_REST,16,NOTE_E5,8
};

// "shortBeep:d=4,o=4,b=250:16d5"
static const uint16_t PROGMEM shortBeepSound[] = {
  250,NOTE_D5,16
}; 

// "timerElapsed:d=4,o=5,b=210:16p,16b6,16p,8b6" 
static const uint16_t PROGMEM timerElapsedSound[] = {
  210,NOTE_REST,16,NOTE_B6,16,NOTE_REST,16,NOTE_B6,8
}; 

// "idle:d=4,o=5,b=90:8p,32c5,32g4,32c5"
static const uint16_t PROGMEM inactivitySound[] = {
  90,NOTE_REST,8,NOTE_C5,32,NOTE_G4,32,NOTE_C5,32
}; 

// "telemWarn:d=4,o=5,b=120:8p,4a#4"
static const uint16_t PROGMEM telemWarnSound[] = {
  120,NOTE_REST,8,NOTE_AS4,4
}; 

// "telemMute:d=4,o=4,b=160:8p,16c5,16e5"
static const uint16_t PROGMEM telemMuteSound[] = {
  160,NOTE_REST,8,NOTE_C5,16,NOTE_E5,16
}; 

// "bind:d=4,o=5,b=75:32d#5,32g5,32a#5,16d6"
static const uint16_t PROGMEM bindSound[] = {
  75,NOTE_DS5,32,NOTE_G5,32,NOTE_AS5,32,NOTE_D6,16
}; 

// "trimMove:d=4,o=4,b=250:16a#7"
static const uint16_t PROGMEM trimMovedSound[] = {
  250,NOTE_AS7,16
}; 

//--- Function declarations ---

void beginTone(int16_t iPin, const uint16_t* toneArray, size_t iSize);
void playback();
void stopPlayback();

//--- variables ----

uint32_t endTimeOfNote = 0;
bool isPlaying = false;
int16_t bpm;
int16_t pin = -1;
uint8_t postn = 0;
size_t size = 0;

const uint16_t* songStart;

//--------------------------------------------------------------------------------------------------

void beginTone(int16_t iPin, const uint16_t* toneArray, size_t iSize)
{
  //init values
  pin = iPin;
  isPlaying = true;
  endTimeOfNote = 0;
  size = iSize;
  songStart = toneArray;
  noTone(pin); //stop any notes
  bpm = pgm_read_word(songStart);
  postn = 1;
}

//--------------------------------------------------------------------------------------------------

void playback()
{
  if(!isPlaying || millis() < endTimeOfNote)
    return;

  if(postn >= size) //end reached, stop
    stopPlayback();
  else //play
  {
    noTone(pin);
    int32_t note = pgm_read_word(songStart + postn);
    int32_t wholenote = (60 * 1000L / bpm) * 4;  // time for whole note in milliseconds
    int32_t duration  = wholenote / pgm_read_word(songStart + postn + 1);
    endTimeOfNote = millis() + duration;
    postn += 2;
    if(note)
      tone(pin, note, duration);
  }
}

//--------------------------------------------------------------------------------------------------

void stopPlayback()
{
  if(isPlaying)
  {
    noTone(pin);
    isPlaying = false;
  }
}

//==================================================================================================

void playTones()
{
  if(!Sys.soundEnabled)
    audioToPlay = AUDIO_NONE; 

  static uint8_t lastAudioToPlay = AUDIO_NONE;
  if(audioToPlay != lastAudioToPlay) //init playback with the specified audio
  {
    lastAudioToPlay = audioToPlay;
    switch(audioToPlay)
    {
      case AUDIO_SAFETY_WARN:   
        beginTone(PIN_BUZZER, warnSound, sizeof(warnSound)/sizeof(warnSound[0])); 
        break;
        
      case AUDIO_BATTERY_WARN:    
        beginTone(PIN_BUZZER, battLowSound, sizeof(battLowSound)/sizeof(battLowSound[0])); 
        break;
        
      case AUDIO_TIMER_ELAPSED:   
        beginTone(PIN_BUZZER, timerElapsedSound, sizeof(timerElapsedSound)/sizeof(timerElapsedSound[0])); 
        break;
        
      case AUDIO_KEY_PRESSED: 
      case AUDIO_SWITCH_MOVED:    
        beginTone(PIN_BUZZER, shortBeepSound, sizeof(shortBeepSound)/sizeof(shortBeepSound[0])); 
        break;
        
      case AUDIO_INACTIVITY:
        beginTone(PIN_BUZZER, inactivitySound, sizeof(inactivitySound)/sizeof(inactivitySound[0]));
        break;
        
      case AUDIO_TELEM_WARN:
        beginTone(PIN_BUZZER, telemWarnSound, sizeof(telemWarnSound)/sizeof(telemWarnSound[0]));
        break;
        
      case AUDIO_TELEM_MUTE_CHANGED:
        beginTone(PIN_BUZZER, telemMuteSound, sizeof(telemMuteSound)/sizeof(telemMuteSound[0]));
        break;
        
      case AUDIO_BIND_SUCCESS:
        beginTone(PIN_BUZZER, bindSound, sizeof(bindSound)/sizeof(bindSound[0]));
        break;
        
      case AUDIO_TRIM_MOVED:
        beginTone(PIN_BUZZER, trimMovedSound, sizeof(trimMovedSound)/sizeof(trimMovedSound[0]));
        break;
    }
  }
  else //Playback. Automatically stops once all notes have been played
    playback();
    
  audioToPlay = AUDIO_NONE;
}

//==================================================================================================

void stopTones()
{
  stopPlayback();
}

