#ifndef _CONFIG_H_
#define _CONFIG_H_

#define RECEIVER_HARDWARE_VERSION_2
// #define RECEIVER_HARDWARE_VERSION_1
// #define LEGACY_HARDWARE

//------ Pins ---

#if defined RECEIVER_HARDWARE_VERSION_2
  #define PIN_CH1          A2
  #define PIN_CH2          2
  #define PIN_CH3          3
  #define PIN_CH4          4
  #define PIN_CH5          5
  #define PIN_CH6          6
  #define PIN_CH7          7
  #define PIN_CH8          8
  #define PIN_CH9          A1
  #define PIN_CH10         A0
  #define PIN_LORA_SS      10
  #define PIN_LORA_RESET   9
  #define PIN_EXT_V_SENSE  A3

#elif defined RECEIVER_HARDWARE_VERSION_1
  #define PIN_CH1          A2
  #define PIN_CH2          2
  #define PIN_CH3          3
  #define PIN_CH4          4
  #define PIN_CH5          5
  #define PIN_CH6          6
  #define PIN_CH7          7
  #define PIN_CH8          8
  #define PIN_CH9          A1
  #define PIN_CH10         A0
  #define PIN_LORA_SS      10
  #define PIN_LORA_RESET   9
  #define PIN_EXT_V_SENSE  A6
  #define PIN_LED          A3

#elif defined LEGACY_HARDWARE
  #define PIN_CH1          2
  #define PIN_CH2          5
  #define PIN_CH3          3
  #define PIN_CH4          4
  #define PIN_CH5          A5
  #define PIN_CH6          A4
  #define PIN_CH7          A3
  #define PIN_CH8          A2
  #define PIN_CH9          A1
  #define PIN_CH10         A0
  #define PIN_LORA_SS      10
  #define PIN_LORA_RESET   8
  #define PIN_EXT_V_SENSE  A6
  #define PIN_LED          6
  #define PIN_PWR_LED      7
  
#endif

//--- External voltage
const int16_t externalVfactor = 1041;  //calibration factor

//--- Radio frequency, select only one
#define ISM_433MHZ
// #define ISM_915MHZ

#endif
