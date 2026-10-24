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
#define ISM_433_MHZ
// #define ISM_915_MHZ

//--- SPI frequency
// Overrides the default SPI frequency.
// Some logic level converters cannot support high speeds such as 8 MHz, so a lower SPI frequency can be selected.
// Note that setting it too low (e.g below 1 MHz) may cause sync issues between the transmitter and receiver. 
#define RF_MODULE_SPI_FREQUENCY 2E6


#endif
