#ifndef _CONFIG_H_
#define _CONFIG_H_

#define STX_HARDWARE_VERSION_2
// #define STX_HARDWARE_VERSION_1
// #define LEGACY_HARDWARE

//------ Pins ---

#if defined STX_HARDWARE_VERSION_2
  #define PIN_LORA_SS      10
  #define PIN_LORA_RESET   9
  
#elif defined STX_HARDWARE_VERSION_1
  #define PIN_LORA_SS      10
  #define PIN_LORA_RESET   9

#elif defined LEGACY_HARDWARE
  #define PIN_LORA_SS      10
  #define PIN_LORA_RESET   8
  
#endif

//--- Radio frequency, select only one
#define ISM_433MHZ
// #define ISM_915MHZ


#endif
