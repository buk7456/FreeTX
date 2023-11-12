
#ifndef _KS0108_H
#define _KS0108_H

#define CS_ACTIVE_LOW  //uncomment this if the chip select pins are active low

#define BLACK 1
#define WHITE 0

#define LCDWIDTH 128
#define LCDHEIGHT 64

typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;

class LCDKS0108 : public GFX
{
  public:
    LCDKS0108(int8_t QRS, int8_t QEN, int8_t QCS1, int8_t QCS2);

    void begin();
    void clearDisplay();
    void display();
    
    void setInterlace(bool enabled);

    void drawPixel(uint8_t x, uint8_t y, uint8_t color);
    uint8_t getPixel(uint8_t x, uint8_t y);
    
    void drawVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color);
    void drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t color);
    
    void writePageColumn(uint8_t page, uint8_t column, uint8_t val); //direct access

  private:
    int8_t _qrs, _qen, _qcs1, _qcs2;
    
    bool isInterlacedScan;

    // The memory buffer for holding the data to be sent to the LCD
    uint8_t dispBuffer[LCDWIDTH * LCDHEIGHT / 8];

    volatile PortReg *qrsport, *qenport, *qcs1port, *qcs2port;
    PortMask qrspinmask, qenpinmask, qcs1pinmask, qcs2pinmask;

    void lcdCommand(uint8_t command);
    void setPage(uint8_t pageNo);
};

#endif
