#ifndef _CONFIG_H_
#define _CONFIG_H_

// #define LEGACY_HARDWARE

//--- pins ---
#ifdef LEGACY_HARDWARE
  #define PIN_LORA_SS     10
  #define PIN_LORA_RESET  8
#else
  #define PIN_LORA_SS     10
  #define PIN_LORA_RESET  9
#endif

//--- radio frequency, select only one
#define ISM_433MHZ
// #define ISM_915MHZ


#endif
