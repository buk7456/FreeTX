/*
 * config.h
 *
 * Basic settings such as:
 * - LCD controller
 * - UI to use
 * - Pin mapping
 * - External EEPROM 
 *
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define _FIRMWARE_VERSION "1.4.1"

//===========================================================================
//============================= LCD and UI ==================================
//===========================================================================

#define UI_128X64
// #define UI_84X48  //not implemented

#define DISPLAY_KS0108
// #define DISPLAY_ST7920
// #define DISPLAY_PCD8544 //not implemented

//===========================================================================
//============================= Pins and ports ==============================
//===========================================================================

//Name                 Arduino Pin

#define PIN_X1_AXIS      A3
#define PIN_Y1_AXIS      A2
#define PIN_X2_AXIS      A0
#define PIN_Y2_AXIS      A1
#define PIN_X3_AXIS      A11
#define PIN_Y3_AXIS      A10
#define PIN_X4_AXIS      A13
#define PIN_Y4_AXIS      A12

#define PIN_Z1_AXIS      A9
#define PIN_Z2_AXIS      A8

#define PIN_KNOB_A       A4
#define PIN_KNOB_B       A5

#define PIN_BATTERY_VOLTS    A6

#define PIN_LCD_BACKLIGHT   2
#define PIN_BUZZER          3

#define PIN_POWER_OFF_SENSE 12
#define PIN_POWER_LATCH     13

#define PIN_KEY_SELECT   8
#define PIN_KEY_UP       9
#define PIN_KEY_DOWN     7

#define PIN_KS_EN        15
#define PIN_KS_RS        14
#define PIN_KS_CS1       4
#define PIN_KS_CS2       39

#define PIN_ST_EN        15
#define PIN_ST_RS        14

//--- Direct port manipulation

#define DDRx_LCD_DATA    DDRA
#define PORTx_LCD_DATA   PORTA

#define DDRx_TRIMS   DDRL
#define PORTx_TRIMS  PORTL
#define PINx_TRIMS   PINL

//--- Physical switches

//Name             Arduino Pin
#define PIN_SWA_UP   37
#define PIN_SWA_DN   36

#define PIN_SWB_UP   35
#define PIN_SWB_DN   34

#define PIN_SWC_UP   33
#define PIN_SWC_DN   32

#define PIN_SWD_UP   31
#define PIN_SWD_DN   30

#define PIN_SWE_UP   10
#define PIN_SWE_DN   11

#define PIN_SWF_UP   41
#define PIN_SWF_DN   40

#define PIN_SWG_UP   61
#define PIN_SWG_DN   69

#define PIN_SWH_UP   5
#define PIN_SWH_DN   38

//--- sd card

#define PIN_SD_CS    53

//===========================================================================
//============================= External EEPROM =============================
//===========================================================================
//Maximum supported is a 512kbit eeprom

/* 
//--- 24LC512
//Total bytes (512*1024)/8 => 65536
#define EXTERNAL_EEPROM_TOTAL_BYTES     65536
#define EXTERNAL_EEPROM_PAGE_BYTES      32
#define EXTERNAL_EEPROM_DEVICE_ADDRESS  0x0
 */

//--- 24LC256
//Total bytes (256*1024)/8 => 32768
#define EXTERNAL_EEPROM_TOTAL_BYTES     32768
#define EXTERNAL_EEPROM_PAGE_BYTES      32
#define EXTERNAL_EEPROM_DEVICE_ADDRESS  0x0

#endif

