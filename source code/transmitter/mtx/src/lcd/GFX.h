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

#ifndef _GFX_H
#define _GFX_H

class GFX : public Print
{

public:
  GFX(uint8_t w, uint8_t h); // Constructor

  // This MUST be defined by the subclass:
  virtual void drawPixel(uint8_t x, uint8_t y, uint8_t color) = 0;

  // BASIC DRAW API
  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.
  virtual void
      drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color), 
      drawVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color),
      drawHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color),
      fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color),
      drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color),
      drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t color);

  // These exist only with GFX (no subclass overrides)
  void
      drawRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color), 
      fillRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color),
      drawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color),
      fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color),
      drawCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t cornername, uint8_t color), 
      fillCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t cornername, uint8_t delta, uint8_t color),
      drawBitmap(uint8_t x, uint8_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint8_t color),
      setCursor(uint8_t x, uint8_t y),
      setTextColor(uint8_t c),
      setTextWrap(boolean w);
      
  virtual size_t write(uint8_t);

  uint8_t height(void) const;
  uint8_t width(void) const;
  
  // get current cursor position
  uint8_t getCursorX(void) const;
  uint8_t getCursorY(void) const;


protected:
  
  const uint8_t
      WIDTH,
      HEIGHT; // This is the 'raw' display w/h - never changes
  uint8_t
      _width,
      _height, // Display w/h as modified by current rotation
      cursor_x, cursor_y;
  uint8_t
      textcolor;
  uint8_t
      rotation;
  boolean
      wrap;  // If set, 'wrap' text at right edge of display
};

#endif // _GFX_H
