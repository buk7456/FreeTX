/*
  Adapted by Buk, from Adafruit GFX lib.
  
  This library has been tailored to this project. Some features have been removed or 
  modified for lower flash usage, reduced overhead, and improved speed.
  Major changes made:
    - Using uint8_t types where necessary.
    - Reimplemented drawHLine, drawVLine, drawRect, fillRect
    - Removed unused features like custom font support, Xbitmaps, etc.
    - Changed some member functions to be overridable.
    
  Original copyright notice
  --------------------------
  Copyright (c) 2013 Adafruit Industries.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "Arduino.h"
#include "GFX.h"
#include "font.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

GFX::GFX(uint8_t w, uint8_t h) : WIDTH(w), HEIGHT(h)
{
  _width = WIDTH;
  _height = HEIGHT;
  rotation = 0;
  cursor_y = cursor_x = 0;
  textcolor = 0xFF;
  wrap = true;
}

void GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
{
  // Bresenham's algorithm - thx Wikipedia
  
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if(steep) 
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if(x0 > x1) 
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if(y0 < y1) 
    ystep = 1;
  else 
   ystep = -1;

  for(; x0<=x1; x0++) 
  {
    if(steep) 
      drawPixel(y0, x0, color);
    else 
      drawPixel(x0, y0, color);
    err -= dy;
    if(err < 0) 
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void GFX::drawHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color)
{
  for(uint8_t i = 0; i < w; i++)
    drawPixel(x + i, y, color);
}

void GFX::drawVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color)
{
  for(uint8_t i = 0; i < h; i++)
    drawPixel(x, y+i, color);
}

void GFX::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
  drawHLine(x, y, w, color);
  drawHLine(x, y+h-1, w, color);
  drawVLine(x, y, h, color);
  drawVLine(x+w-1, y, h, color);
}

void GFX::fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
  //Here we choose the faster method
  if(w > 4*h)
  {
    for(uint8_t i = 0; i < h; i++)
      drawHLine(x, y + i, w, color);
  }
  else
  {
    for(uint8_t i = 0; i < w; i++)
      drawVLine(x + i, y, h, color);
  }
}

void GFX::drawRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color) 
{
  if((2*r) >= w || (2*r) >= h)
    return;

  drawHLine(x+r  , y    , w-2*r, color); // Top
  drawHLine(x+r  , y+h-1, w-2*r, color); // Bottom
  drawVLine(x    , y+r  , h-2*r, color); // Left
  drawVLine(x+w-1, y+r  , h-2*r, color); // Right
  // draw four corners
  drawCircleHelper(x+r    , y+r    , r, 1, color);
  drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

void GFX::fillRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color) 
{
  if((2*r) >= w || (2*r) >= h)
    return;
  
  fillRect(x+r, y, w-2*r, h, color);
  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

void GFX::drawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) 
{
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  drawPixel(x0  , y0+r, color);
  drawPixel(x0  , y0-r, color);
  drawPixel(x0+r, y0  , color);
  drawPixel(x0-r, y0  , color);

  while(x < y) 
  {
    if(f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

void GFX::fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
  drawVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

void GFX::drawCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t cornername, uint8_t color) 
{
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  while(x < y) 
  {
    if(f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if(cornername & 0x4) 
    {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if(cornername & 0x2) 
    {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if(cornername & 0x8) 
    {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if(cornername & 0x1) 
    {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void GFX::fillCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t cornername, uint8_t delta, uint8_t color) 
{
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  while(x < y) 
  {
    if(f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if(cornername & 0x1) 
    {
      drawVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if(cornername & 0x2) 
    {
      drawVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

// Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
// using the specified foreground color (unset bits are transparent).
void GFX::drawBitmap(uint8_t x, uint8_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint8_t color)
{
  uint8_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;

  for(uint8_t j = 0; j < h; j++, y++)
  {
    for(uint8_t i = 0; i < w; i++)
    {
      if(i & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      if(byte & 0x80)
        drawPixel(x + i, y, color);
    }
  }
}

// Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
// using the specified foreground (for set bits) and background (unset bits) colors
void GFX::drawBitmap(uint8_t x, uint8_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint8_t color, uint8_t bg)
{
  uint8_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;

  for(uint8_t j = 0; j < h; j++, y++)
  {
    for(uint8_t i = 0; i < w; i++)
    {
      if(i & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      if(byte & 0x80)
        drawPixel(x + i, y, color);
      else
        drawPixel(x + i, y, bg);
    }
  }
}

// Draw a character
void GFX::drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t color)
{
  for(uint8_t i = 0; i < 5; i++)
  { 
    // Char bitmap is 5 columns
    uint16_t idx =  (uint16_t) 5 * c + i;
    uint8_t line = pgm_read_byte(&font[idx]);
    for(uint8_t j = 0; j < 8; j++, line >>= 1)
    {
      if(line & 1)
        drawPixel(x + i, y + j, color);
    }
  }
}

size_t GFX::write(uint8_t c)
{
  if(c == '\n')
  { // Newline?
    cursor_x  = 0;  // Reset x to zero,
    cursor_y += 8;  // advance y one line
  }
  else if(c != '\r')
  { // Ignore carriage returns
    if(wrap && ((cursor_x + 6) > _width))
    { // Off right?
      cursor_x  = 0; // Reset x to zero,
      cursor_y += 8; // advance y one line
    }
    drawChar(cursor_x, cursor_y, c, textcolor);
    cursor_x += 6;   // Advance x one char
  }
  return 1;
}

void GFX::setCursor(uint8_t x, uint8_t y)
{
  cursor_x = x;
  cursor_y = y;
}

uint8_t GFX::getCursorX(void) const
{
  return cursor_x;
}

uint8_t GFX::getCursorY(void) const
{
  return cursor_y;
}

void GFX::setTextColor(uint8_t c)
{
  textcolor = c;
}

uint8_t GFX::getTextColor(void) const
{
  return textcolor;
}

void GFX::setTextWrap(boolean w)
{
  wrap = w;
}

uint8_t GFX::width(void) const
{
  return _width;
}

uint8_t GFX::height(void) const
{
  return _height;
}

