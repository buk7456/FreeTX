#ifndef _INPUTS_H_
#define _INPUTS_H_

void initialiseSwitches();
void readSwitchesAndButtons(); 
void determineButtonEvent();
void killButtonEvents();
void readSticks();
void calibrateSticks(uint8_t stage);   
void calibrateKnobs(uint8_t stage);   

#endif