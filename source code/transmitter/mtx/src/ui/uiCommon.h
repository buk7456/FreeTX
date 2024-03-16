#ifndef _UICOMMON_H_
#define _UICOMMON_H_

extern bool isEditMode;

enum {
  INCDEC_WRAP = true, 
  INCDEC_NOWRAP = false,
  
  INCDEC_PRESSED, 
  INCDEC_SLOW,
  INCDEC_SLOW_START,
  INCDEC_NORMAL,
  INCDEC_FAST,
  
  INCDEC_FLAG_TELEM_SRC = 0x01,
  INCDEC_FLAG_MIX_SRC = 0x02,
  INCDEC_FLAG_COUNTER_SRC = 0x04,
  INCDEC_FLAG_TIMER_SRC = 0x08,
  
  INCDEC_FLAG_PHY_SW  = 0X01,
  INCDEC_FLAG_LGC_SW  = 0x02,
  INCDEC_FLAG_FMODE   = 0x04,
}; 

extern uint32_t timerLastElapsedTime[NUM_TIMERS];
extern uint32_t timerLastPaused[NUM_TIMERS];
extern bool     timerIsRunning[NUM_TIMERS];
extern bool     timerForceRun[NUM_TIMERS];

extern const char* toastText;
extern uint32_t toastEndTime;
extern uint32_t toastStartTime;
extern bool     toastExpired;

//------------- functions ----------------

void resetTimerRegisters();
void resetTimerRegister(uint8_t idx);

void resetCounterRegisters();
void resetCounterRegister(uint8_t idx);

uint8_t getMovedSource();
uint8_t getMovedControlSwitch();
uint8_t procMovedSource(uint8_t src);

void telemetryAlarmHandler();
void inactivityAlarmHandler();
void timerHandler();

void notificationHandler();

uint8_t getLSFuncGroup(uint8_t func);

bool hasEnoughMixSlots(uint8_t start, uint8_t numRequired);
bool hasOccupiedMixSlots(uint8_t start, uint8_t numRequired);

void calcNewCurvePts(custom_curve_t *crv, uint8_t numOldPts);

void makeToast(const char* text, uint16_t duration, uint16_t dly);

int16_t incDec(int16_t val, int16_t lowerLimit, int16_t upperLimit, bool wrapEnabled, uint8_t state);
uint8_t incDecSource(uint8_t val, uint8_t flag);
uint8_t incDecControlSwitch(uint8_t val, uint8_t flag);

void drawNotificationOverlay(uint8_t idx, uint32_t startTime, uint32_t endTime); //implemented in the resolution specific UI file

void screenshotHandler();


#endif
