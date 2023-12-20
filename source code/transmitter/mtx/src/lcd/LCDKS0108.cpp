
#include "Arduino.h"
#include "GFX.h"
#include "LCDKS0108.h"
#include "font.h"
#include "../../config.h"

#define START_LINE    0b11000000
#define SET_Y_ADDRESS 0b01000000
#define SET_PAGE      0b10111000
#define DISP_ON       0b00111111
#define DISP_OFF      0b00111110

//asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n") is 10 cycles (625 ns at 16 MHz)

#define EN_DELAY_HIGHLOW { asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); }

#define EN_DELAY_LOWHIGH { asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); }

//--------------------------------------------------------------------------------------------------

LCDKS0108::LCDKS0108(int8_t QRS, int8_t QEN, int8_t QCS1, int8_t QCS2) : GFX(LCDWIDTH, LCDHEIGHT)
{
  _qrs = QRS;
  _qen = QEN;
  _qcs1 = QCS1;
  _qcs2 = QCS2;
}

//--------------------------------------------------------------------------------------------------

// the most basic function, set a single pixel
void LCDKS0108::drawPixel(uint8_t x, uint8_t y, uint8_t color)
{
  if(x >= LCDWIDTH || y >= LCDHEIGHT) 
    return;
  uint16_t idx = (uint16_t) LCDWIDTH * (y / 8) + x;
  if(color)
    dispBuffer[idx] |= (1 << (y % 8));
  else
    dispBuffer[idx] &= ~(1 << (y % 8));
}

//--------------------------------------------------------------------------------------------------

// the most basic function, get a single pixel
uint8_t LCDKS0108::getPixel(uint8_t x, uint8_t y)
{
  if(x >= LCDWIDTH || y >= LCDHEIGHT) 
    return 0;
  uint16_t idx = (uint16_t) LCDWIDTH * (y / 8) + x;
  return (dispBuffer[idx] >> (y % 8)) & 0x1;
}

//--------------------------------------------------------------------------------------------------

//Faster implementation than regular drawVLine
//Credit: cbm80amiga's ST7567 library https://github.com/cbm80amiga/ST7567_FB
void LCDKS0108::drawVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color)
{
  if(x >= LCDWIDTH || y >= LCDHEIGHT) 
    return;
  
  uint8_t y0 = y;
  uint8_t y1 = y0 + h - 1;
  if(y1 >= LCDHEIGHT)
    y1 = LCDHEIGHT - 1;
  
  //determine the start and end page in the display buffer (0 to 7). The display buffer is divided
  //into 8 pages, just lke the LCD's RAM, with each page containg 128 bytes (columns)
  uint8_t y8s = y0/8; //start page
  uint8_t y8e = y1/8; //end end page
  
  const uint8_t ystab[8] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};
  const uint8_t yetab[8] = {0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
  
  if(color)
  {
    if(y8s==y8e) 
      dispBuffer[y8s*LCDWIDTH+x] |= (ystab[y0&7] & yetab[y1&7]);
    else 
    { 
      dispBuffer[y8s*LCDWIDTH+x] |= ystab[y0&7]; 
      dispBuffer[y8e*LCDWIDTH+x] |= yetab[y1&7]; 
      for(uint8_t page = y8s + 1; page < y8e; page++)        
        dispBuffer[page*LCDWIDTH+x] = 0xff;
    }
  }
  else
  {
    if(y8s==y8e) 
      dispBuffer[y8s*LCDWIDTH+x] &= ~(ystab[y0&7] & yetab[y1&7]);
    else 
    { 
      dispBuffer[y8s*LCDWIDTH+x] &= ~ystab[y0&7]; 
      dispBuffer[y8e*LCDWIDTH+x] &= ~yetab[y1&7]; 
      for(uint8_t page = y8s + 1; page < y8e; page++) 
        dispBuffer[page*LCDWIDTH+x] = 0x00;
    }
  }
}

//--------------------------------------------------------------------------------------------------

//Faster implementation than regular drawHLine
//Achieves speed by eliminating calls to drawPixel() in the loops.
//Credit: cbm80amiga's ST7567 library https://github.com/cbm80amiga/ST7567_FB
void LCDKS0108::drawHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color)
{
  if(x >= LCDWIDTH || y >= LCDHEIGHT) 
    return;
  
  uint8_t x0 = x;
  uint8_t x1 = x0 + w - 1;
  if(x1 >= LCDWIDTH)
    x1 = LCDWIDTH - 1;
  
  uint8_t mask = 1 << (y&7);
  if(color)
  {
    for(uint8_t k = x0; k <= x1; k++)
      dispBuffer[(y/8)*LCDWIDTH+k] |= mask;
  }
  else
  {
    for(uint8_t k = x0; k <= x1; k++)
      dispBuffer[(y/8)*LCDWIDTH+k] &= ~mask;
  }
}

//--------------------------------------------------------------------------------------------------

// Faster implementation of drawChar
// Code page 437 character set
// 5x7 font 
void LCDKS0108::drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t color)
{
  if(x >= (LCDWIDTH - 5) || y >= LCDHEIGHT) 
    return;
  
  uint8_t y0 = y;
  uint8_t y1 = y0 + 7;
  if(y1 >= LCDHEIGHT)
    y1 = LCDHEIGHT - 1;
  
  //determine the start and end page in the display buffer (0 to 7). The display buffer is divided
  //into 8 pages, just lke the LCD's RAM, with each page containg 128 bytes (columns)
  uint8_t y8s = y0/8; //start page
  uint8_t y8e = y1/8; //end page

  //determine how much to bit shift.
  uint8_t sShift = y0 % 8;
  uint8_t eShift = 8 - sShift;
  
  //read a 'line' (vertical) from the character and draw it.
  uint16_t s = y8s * LCDWIDTH;
  uint16_t e = y8e * LCDWIDTH; 
  uint16_t cc = (uint16_t) 5 * c; 
  uint8_t line;
  for(uint8_t i = 0; i < 5; i++, x++)
  { 
    line = pgm_read_byte(&font[cc+i]);
    if(color) 
    {
      dispBuffer[s+x] |= (line << sShift);
      if(y8s != y8e)           
        dispBuffer[e+x] |= (line >> eShift); 
    }
    else
    {
      dispBuffer[s+x] &= ~(line << sShift); 
      if(y8s != y8e) 
        dispBuffer[e+x] &= ~(line >> eShift); 
    }
  }
}

//--------------------------------------------------------------------------------------------------

//This low level function directly writes to the LCD, skipping the frame buffer altogether.
//It comes in handy when we want speed.
void LCDKS0108::writePageColumn(uint8_t page, uint8_t column, uint8_t val)
{
  //Page 0 to 7, column 0 to 127
  page &= 0x07;
  column &= 0x7f;
  
  setPage(page);
  lcdCommand(SET_Y_ADDRESS | column); //set column
  
  //rs high
  *qrsport |= qrspinmask;

  if(column < 64)
  {
    //enable chip1, disable chip2
    #if defined (CS_ACTIVE_LOW)
    *qcs1port &= ~qcs1pinmask;  
    *qcs2port |= qcs2pinmask;
    #else
    *qcs1port |= qcs1pinmask;  
    *qcs2port &= ~qcs2pinmask; 
    #endif
  }
  else
  {
    //enable chip2, disable chip1
    #if defined (CS_ACTIVE_LOW)
      *qcs2port &= ~qcs2pinmask;
      *qcs1port |= qcs1pinmask;
    #else 
      *qcs2port |= qcs2pinmask; 
      *qcs1port &= ~qcs1pinmask;
    #endif
  }
  
  PORTx_LCD_DATA = val;    //write
  *qenport |= qenpinmask;  //EN high
  EN_DELAY_HIGHLOW;        //Delay 
  *qenport &= ~qenpinmask; //EN low 
  EN_DELAY_LOWHIGH;        //Delay
}

//--------------------------------------------------------------------------------------------------

void LCDKS0108::begin()
{
  pinMode(_qrs, OUTPUT);
  pinMode(_qen, OUTPUT);
  pinMode(_qcs1, OUTPUT);
  pinMode(_qcs2, OUTPUT);
  
  //set port as output port
  DDRx_LCD_DATA = 0xFF;

  //Set ports and masks
  qrsport = portOutputRegister(digitalPinToPort(_qrs));
  qrspinmask = digitalPinToBitMask(_qrs);

  qenport = portOutputRegister(digitalPinToPort(_qen));
  qenpinmask = digitalPinToBitMask(_qen);

  qcs1port = portOutputRegister(digitalPinToPort(_qcs1));
  qcs1pinmask = digitalPinToBitMask(_qcs1);
  
  qcs2port = portOutputRegister(digitalPinToPort(_qcs2));
  qcs2pinmask = digitalPinToBitMask(_qcs2);

  //initialise lcd
  lcdCommand(DISP_ON);
  lcdCommand(START_LINE);
  
  //interlacing
  isInterlacedScan = false;
}

//--------------------------------------------------------------------------------------------------

void LCDKS0108::setPage(uint8_t pageNo)
{
  lcdCommand(SET_PAGE | pageNo);
  lcdCommand(SET_Y_ADDRESS);
}

//--------------------------------------------------------------------------------------------------

void LCDKS0108::setInterlace(bool enabled)
{
  if(enabled)
    isInterlacedScan = true;
  else //Progressive scan
    isInterlacedScan = false; 
}

//--------------------------------------------------------------------------------------------------

void LCDKS0108::clearDisplay() //Clear frame buffer
{
  memset(dispBuffer, 0, sizeof(dispBuffer));
  cursor_y = cursor_x = 0; //These are in GFX lib
}

//--------------------------------------------------------------------------------------------------

void LCDKS0108::display()
{
  //Progressive scan
  uint8_t startPage = 0, endPage = 7, step = 1;
  uint16_t dataIdx = 0;
  
  //Interlaced scan
  /*
  With interlacing, we alternate between sending the odd and even pages.
  There are a total of 8 pages (0 to 7), but for interlace we only send half of these,
  thus we in essence halve the time taken compared to full frame (progressive scan). 
  The catch however is that the half frames are not exactly halves of the same image, 
  and they can be significantly different when there is fast motion, or when the scene changes. 
  However as long as the display() function is being called at a fast enough rate 
  say greater than 50x a second, interlacing artifacts (especially combing) 
  wont be noticeable to the eye.
  */
  if(isInterlacedScan)
  {
    static bool even;
    even = !even;
    if(even){startPage = 0; endPage = 6; step = 2; dataIdx = 0;}
    else    {startPage = 1; endPage = 7; step = 2; dataIdx = 128;}
  }

  //Output to the physical display
  for(uint8_t page = startPage; page <= endPage; page += step)
  {
    setPage(page);
    
    //rs high
    *qrsport |= qrspinmask; 

    //enable chip1, disable chip2
    #if defined (CS_ACTIVE_LOW)
    *qcs1port &= ~qcs1pinmask;  
    *qcs2port |= qcs2pinmask;
    #else
    *qcs1port |= qcs1pinmask;  
    *qcs2port &= ~qcs2pinmask; 
    #endif
    
    /*     
    for(uint8_t column = 0; column < 128; column++)
    {
      if(column == 64)
      {
        //enable chip2, disable chip1
      #if defined (CS_ACTIVE_LOW)
        *qcs2port &= ~qcs2pinmask;
        *qcs1port |= qcs1pinmask;
      #else 
        *qcs2port |= qcs2pinmask; 
        *qcs1port &= ~qcs1pinmask;
      #endif
      }
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
    }
    */

    
    //Unrolled loop version
    //this saves about 0.6ms in progressive scan mode
    //at the expense of a few bytes more flash memory usage
    for(uint8_t column = 0; column < 128; column += 8)
    {
      if(column == 64)
      {
        //enable chip2, disable chip1
      #if defined (CS_ACTIVE_LOW)
        *qcs2port &= ~qcs2pinmask;
        *qcs1port |= qcs1pinmask;
      #else 
        *qcs2port |= qcs2pinmask; 
        *qcs1port &= ~qcs1pinmask;
      #endif
      }
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
      
      PORTx_LCD_DATA = dispBuffer[dataIdx++]; //write
      *qenport |= qenpinmask;  //EN high
      EN_DELAY_HIGHLOW;        //Delay 
      *qenport &= ~qenpinmask; //EN low 
      EN_DELAY_LOWHIGH;        //Delay
    }

    if(isInterlacedScan)
      dataIdx += 128;
  }
}

//--------------------------------------------------------------------------------------------------

inline void LCDKS0108::lcdCommand(uint8_t command)
{
//enable both controllers
#if defined (CS_ACTIVE_LOW)
  *qcs1port &= ~qcs1pinmask;
  *qcs2port &= ~qcs2pinmask;
#else
  *qcs1port |= qcs1pinmask;
  *qcs2port |= qcs2pinmask;
#endif
  
  //rs low
  *qrsport &= ~qrspinmask;
 
  PORTx_LCD_DATA = command; //write
  *qenport |= qenpinmask;  //EN high
  EN_DELAY_HIGHLOW;        //Delay 
  *qenport &= ~qenpinmask; //EN low 
  EN_DELAY_LOWHIGH;        //Delay
}

