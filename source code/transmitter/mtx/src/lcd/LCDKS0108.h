
#ifndef _KS0108_H
#define _KS0108_H

#define BLACK 1
#define WHITE 0

#define LCDWIDTH 128
#define LCDHEIGHT 64

typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;

class LCDKS0108 : public GFX
{
  public:
    LCDKS0108(int8_t rs, int8_t en, int8_t cs1, int8_t cs2);

    void begin();
    void clearDisplay();
    void display();
    
    void setInterlace(bool enabled);

    void drawPixel(uint8_t x, uint8_t y, uint8_t color);
    uint8_t getPixel(uint8_t x, uint8_t y);
    
    void drawHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color);
    void drawVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color);
    void drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t color);
    
    void writePageColumn(uint8_t page, uint8_t column, uint8_t val); //direct access

  private:
    int8_t _rs, _en, _cs1, _cs2;
    
    bool isInterlacedScan;

    // The memory buffer for holding the data to be sent to the LCD
    uint8_t dispBuffer[LCDWIDTH * LCDHEIGHT / 8];

    volatile PortReg *rsPort, *enPort, *cs1Port, *cs2Port;
    PortMask rsPinMask, enPinMask, cs1PinMask, cs2PinMask;

    void lcdCommand(uint8_t command);
    void setPage(uint8_t page);
};


#endif
