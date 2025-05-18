
#include "Arduino.h"
#include "GFX.h"
#include "LCDST7920.h"
#include "font.h"
#include "../../config.h"

//asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n") is 10 cycles (625 ns at 16 MHz)

/*
The LCD oscillator frequency minimum is 470 kHz according to the datasheet, typical is 530 kHz.
Taking the minimum of 470 kHz, the period is (1 / (470 kHz)) = 2.128 microseconds = 2128 ns.
From empirical tests, the LCD needs at least two oscillator cycles to recognise the EN high state,
and four oscillator cycles to recognise the EN low state.
Thus the mimimum delays we need at the microcontroller side are (2/(470k)) and (4/(470k)) seconds,
which rounds to about 70 and 140 cycles respectively. ( 1 cycle is 62.5 ns at 16 MHz)
*/

#define EN_DELAY_HIGHLOW { asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); }

#define EN_DELAY_LOWHIGH { asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); \
                           asm volatile ("lpm \n" "lpm \n" "lpm \n" "nop \n"); }

//--------------------------------------------------------------------------------------------------

LCDST7920::LCDST7920(int8_t rs, int8_t en) : GFX(LCDWIDTH, LCDHEIGHT)
{
  _rs = rs;
  _en = en;
}

//--------------------------------------------------------------------------------------------------

// the most basic function, set a single pixel
void LCDST7920::drawPixel(uint8_t x, uint8_t y, uint8_t color)
{
  if(x >= LCDWIDTH || y >= LCDHEIGHT) 
    return;

  uint16_t idx = ((uint16_t) LCDWIDTH * y + x) / 8;
  if(color)
    dispBuffer[idx] |= (0x80 >> (x & 7));
  else
    dispBuffer[idx] &= ~(0x80 >> (x & 7));
}

//--------------------------------------------------------------------------------------------------

// the most basic function, get a single pixel
uint8_t LCDST7920::getPixel(uint8_t x, uint8_t y)
{
  if(x >= LCDWIDTH || y >= LCDHEIGHT) 
    return 0;
  
  uint16_t idx = ((uint16_t) LCDWIDTH * y + x) / 8;
  // return (dispBuffer[idx] >> (7 - (x & 7))) & 0x1;
  return (dispBuffer[idx] >> (~x & 7)) & 0x1;
}

//--------------------------------------------------------------------------------------------------

//Faster implementation than regular drawVLine
//Credit: cbm80amiga's ST7920 library
void LCDST7920::drawVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color)
{
  if(x >= LCDWIDTH) 
    return;
  
  if(y >= LCDHEIGHT) 
  {
    //Handle the uint8_t wrapping around.
    //For example the y position passed to the function could have been negative. In this case 
    //we draw the portion visible, instead of instead of just returning from the function.
    uint8_t p = (255 - y) + 1;
    if(p >= h || p >= 64)
      return;
    else
    {
      y = 0;
      h -= p;
    }
  }
  
  uint8_t y0 = y;
  uint8_t y1 = y0 + h - 1;
  if(y1 >= LCDHEIGHT)
    y1 = LCDHEIGHT - 1;
  
  uint8_t mask = 0x80 >> (x & 7);
  
  if(color)
  {
    for(uint8_t k = y0; k <= y1; k++) 
      dispBuffer[((uint16_t) LCDWIDTH * k + x) / 8] |= mask;
  }
  else
  {
    for(uint8_t k = y0; k <= y1; k++) 
      dispBuffer[((uint16_t) LCDWIDTH * k + x) / 8] &= ~mask;
  }
}

//--------------------------------------------------------------------------------------------------

//Faster implementation than regular drawHLine
//Credit: cbm80amiga's ST7920 library
void LCDST7920::drawHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color)
{
  if(y >= LCDHEIGHT)
    return;

  if(x >= LCDWIDTH)
  {
    //Handle the uint8_t wrapping around.
    //For example the x position passed to the function could have been negative. In this case
    //we draw the portion visible, instead of instead of just returning from the function.
    uint8_t p = (255 - x) + 1;
    if(p >= w || p >= 64)
      return;
    else
    {
      x = 0;
      w -= p;
    }
  }

  uint8_t x0 = x;
  uint8_t x1 = x0 + w - 1;
  if(x1 >= LCDWIDTH)
    x1 = LCDWIDTH - 1;

  uint16_t yadd = (y * LCDWIDTH) / 8;

  uint8_t x8s = x0 / 8;
  uint8_t x8e = x1 / 8;

  const uint8_t xstab[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
  const uint8_t xetab[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};

  if(color)
  {
    if(x8s == x8e)
      dispBuffer[yadd + x8s] |= (xstab[x0 & 7] & xetab[x1 & 7]);
    else
    {
      dispBuffer[yadd + x8s] |= xstab[x0 & 7];
      dispBuffer[yadd + x8e] |= xetab[x1 & 7];
    }
    for (uint8_t k = x8s + 1; k < x8e; k++)
      dispBuffer[yadd + k] = 0xff;

  }
  else
  {
    if(x8s == x8e)
      dispBuffer[yadd + x8s] &= ~(xstab[x0 & 7] & xetab[x1 & 7]);
    else
    {
      dispBuffer[yadd + x8s] &= ~xstab[x0 & 7];
      dispBuffer[yadd + x8e] &= ~xetab[x1 & 7];
    }
    for (uint8_t k = x8s + 1; k < x8e; k++)
      dispBuffer[yadd + k] = 0x00;
  }
}

//--------------------------------------------------------------------------------------------------

// Faster implementation of drawChar
// 5x7 font
void LCDST7920::drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t color)
{
  if(x >= (LCDWIDTH - 5) || y >= LCDHEIGHT) 
    return;
  
  uint8_t jCount = 8;
  if(y >= LCDHEIGHT - 7)
    jCount = LCDHEIGHT - y;

  //Determine the start and end horizontal 'page'
  uint8_t x8s = x / 8;        //start
  uint8_t x8e = (x + 5) / 8;  //end
  
  //Determine how much to bit shift.
  uint8_t sShift = x % 8;
  uint8_t eShift = 8 - sShift;
  
  //Get the starting index of the character (in the font table)
  uint16_t cc = (uint16_t) 8 * c; 

  //Write to the display buffer
  if(color)
  {
    if(x8s == x8e)
    {
      for(uint8_t j = 0; j < jCount; j++)
      {
        uint8_t line = pgm_read_byte(&font_hh[cc + j]);
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8s] |= (line >> sShift);
      }
    }
    else
    {
      for(uint8_t j = 0; j < jCount; j++)
      {
        uint8_t line = pgm_read_byte(&font_hh[cc + j]);
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8s] |= (line >> sShift);
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8e] |= (line << eShift);
      }
    }
  }
  else
  {
    if(x8s == x8e)
    {
      for(uint8_t j = 0; j < jCount; j++)
      {
        uint8_t line = pgm_read_byte(&font_hh[cc + j]);
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8s] &= ~(line >> sShift);
      }
    }
    else
    {
      for(uint8_t j = 0; j < jCount; j++)
      {
        uint8_t line = pgm_read_byte(&font_hh[cc + j]);
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8s] &= ~(line >> sShift);
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8e] &= ~(line << eShift);
      }
    }
  }
}

/*
//The implementation below is actually much slower than regular drawChar(), 
//due to the conversion code for vertical to horizontal ordering.
void LCDST7920::drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t color)
{
  if(x >= (LCDWIDTH - 5) || y >= LCDHEIGHT) 
    return;
  
  uint8_t jCount = 8;
  if(y >= LCDHEIGHT - 7)
    jCount = LCDHEIGHT - y;

  //Determine the start and end horizontal 'page'
  uint8_t x8s = x / 8;        //start
  uint8_t x8e = (x + 5) / 8;  //end
  
  //Determine how much to bit shift.
  uint8_t sShift = x % 8;
  uint8_t eShift = 8 - sShift;
  
  //Get the starting index of the character (in the font table)
  uint16_t cc = (uint16_t) 5 * c; 
  
  //Read a line (horizontal) from the glyph.
  //Need to convert the character from vertical to horizontal layout.

  uint8_t line[8]; //the font has 8 horizontal lines
  memset(line, 0, sizeof(line));

  for(uint8_t i = 0; i < 5; i++)
  {
    // uint8_t b = pgm_read_byte(&font[cc + i]);
    uint8_t b = pgm_read_byte(&font[cc + 4 - i]);
    for(uint8_t j = 0; j < jCount; j++)
    {
      line[j] |= (((b >> j) & 1) << (3 + i));
    }
  }
  
  //Write to the display buffer
  if(color)
  {
    if(x8s == x8e)
    {
      for(uint8_t j = 0; j < jCount; j++)
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8s] |= (line[j] >> sShift);
    }
    else
    {
      for(uint8_t j = 0; j < jCount; j++)
      {
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8s] |= (line[j] >> sShift);
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8e] |= (line[j] << eShift);
      }
    }
  }
  else
  {
    if(x8s == x8e)
    {
      for(uint8_t j = 0; j < jCount; j++)
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8s] &= ~(line[j] >> sShift);
    }
    else
    {
      for(uint8_t j = 0; j < jCount; j++)
      {
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8s] &= ~(line[j] >> sShift);
        dispBuffer[((uint16_t)(y + j) * LCDWIDTH / 8) + x8e] &= ~(line[j] << eShift);
      }
    }
  }
}
*/

//--------------------------------------------------------------------------------------------------

void LCDST7920::begin()
{
  pinMode(_rs, OUTPUT);
  pinMode(_en, OUTPUT);

  //set port as output port
  DDRx_LCD_DATA = 0xFF;

  //Set ports and masks
  rsPort = portOutputRegister(digitalPinToPort(_rs));
  rsPinMask = digitalPinToBitMask(_rs);

  enPort = portOutputRegister(digitalPinToPort(_en));
  enPinMask = digitalPinToBitMask(_en);
  
  //initialise lcd
  
  delay(100);
  
  lcdCommand(0x30);      // set 8 bit operation and basic instruction set
  delayMicroseconds(200);
  lcdCommand(0x0C);      // display on cursor off and char blink off
  delayMicroseconds(200);   
  lcdCommand(0x06);      // entry mode set
  delayMicroseconds(200);  
  lcdCommand(0x01);      // display clear
  delay(2);        
  lcdCommand(0x34);      // Select extended instruction set
  delayMicroseconds(200);
  lcdCommand(0x36);      // Graphic display ON
  delayMicroseconds(200);
  lcdCommand(0x02);      // Graphic display ON
  delayMicroseconds(200);
  
  //interlacing
  isInterlacedScan = false;
}

//--------------------------------------------------------------------------------------------------

void LCDST7920::setInterlace(bool enabled)
{
  if(enabled)
    isInterlacedScan = true;
  else //Progressive scan
    isInterlacedScan = false; 
}

//--------------------------------------------------------------------------------------------------

void LCDST7920::clearDisplay() //Clear frame buffer
{
  memset(dispBuffer, 0, sizeof(dispBuffer));
  cursor_y = cursor_x = 0; //These are in GFX lib
}

//--------------------------------------------------------------------------------------------------

void LCDST7920::display()
{
  uint16_t dataIdx = 0;

  // progressive scan
  uint8_t jStart = 0, jEnd = 31, jStep = 1;
  
  // interlaced scan
  /*   
  if(isInterlacedScan)
  {
    static bool even;
    even = !even;
    if(even) {jStart = 0; jEnd = 30; jStep = 2;}
    else     {jStart = 1; jEnd = 31; jStep = 2;}
  } 
  */
  if(isInterlacedScan)
  {
    // Here we use an extreme form of interlacing since this LCD is relatively slow.
    // We divide the screen into four portions, and we update only one portion at every call of the display function. 

    // Method 1
    /*
    static uint8_t qq = 0;
    jStart = 0 + (qq % 4);
    jEnd = 28 + (qq % 4);
    jStep = 4;
    qq++;
    */
    
    // Method 2
    // This produces less noticeable visual artifacts (combing, etc.)
    static uint8_t qq = 0;
    jStart = 8 * (qq % 4);
    jEnd = jStart + 7;
    jStep = 1;
    qq++;
  }

  for(uint8_t j = jStart; j <= jEnd; j += jStep)
  {
    //set GDRAM address. Vertical address first, followed by horizontal address
    delayMicroseconds(100); //tried 80 and less, they do not work, display becomes glitchy.
    lcdCommand(0b10000000 | j);
    // delayMicroseconds(100); //not necessary
    lcdCommand(0b10000000 | 0);
    delayMicroseconds(100);

    dataIdx = (uint16_t) j * 16;

    for(uint8_t i = 0; i < 16; i++)
    {
      *rsPort |= rsPinMask; //rs high
      PORTx_LCD_DATA = dispBuffer[dataIdx++];
      *enPort |= enPinMask;  //EN high
      EN_DELAY_HIGHLOW;      //Delay 
      *enPort &= ~enPinMask; //EN low 
      EN_DELAY_LOWHIGH;      //Delay
    } 
    
    dataIdx = (uint16_t) (j + 32) * 16;
    
    for(uint8_t i = 0; i < 16; i++)
    {
      *rsPort |= rsPinMask; //rs high
      PORTx_LCD_DATA = dispBuffer[dataIdx++];
      *enPort |= enPinMask;  //EN high
      EN_DELAY_HIGHLOW;      //Delay 
      *enPort &= ~enPinMask; //EN low 
      EN_DELAY_LOWHIGH;      //Delay
    } 
  }
}

//--------------------------------------------------------------------------------------------------

inline void LCDST7920::lcdCommand(uint8_t command)
{
  *rsPort &= ~rsPinMask; //rs low
  PORTx_LCD_DATA = command; //write
  *enPort |= enPinMask;   //EN high
  EN_DELAY_HIGHLOW;       //Delay 
  *enPort &= ~enPinMask;  //EN low 
  EN_DELAY_LOWHIGH;       //Delay
}

