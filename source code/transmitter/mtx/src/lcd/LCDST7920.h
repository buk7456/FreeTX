
#ifndef _ST7920_H
#define _ST7920_H

#define BLACK 1
#define WHITE 0

#define LCDWIDTH 128
#define LCDHEIGHT 64

typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;

class LCDST7920 : public GFX
{
  public:
    LCDST7920(int8_t rs, int8_t en);

    void begin();
    void clearDisplay();
    void display();
    
    void setInterlace(bool enabled);

    void drawPixel(uint8_t x, uint8_t y, uint8_t color);
    uint8_t getPixel(uint8_t x, uint8_t y);
    
    void drawHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color);
    void drawVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color);
    void drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t color);

  private:
    int8_t _rs, _en;
    
    bool isInterlacedScan;

    // The memory buffer for holding the data to be sent to the LCD
    uint8_t dispBuffer[LCDWIDTH * LCDHEIGHT / 8];

    volatile PortReg *rsPort, *enPort;
    PortMask rsPinMask, enPinMask;

    void lcdCommand(uint8_t command);
};

#endif
