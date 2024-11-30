#ifndef _MIXER_H_
#define _MIXER_H_

void computeChannelOutputs();

int16_t calcRateExpo(int16_t input, int16_t rate, int16_t expo);
bool checkSwitchCondition(uint8_t sw);
void moveMix(uint8_t newPos, uint8_t oldPos);
void swapMix(uint8_t posA, uint8_t posB);
bool moveLogicalSwitch(uint8_t newPos, uint8_t oldPos);
void syncWaveform(uint8_t idx);
void reinitialiseMixerCalculations();
int16_t adjustTrim(uint8_t idx, int16_t val, uint8_t incButton, uint8_t decButton);

extern int16_t mixSources[MIX_SOURCES_COUNT]; 

#endif
