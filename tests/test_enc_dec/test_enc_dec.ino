#include "Arduino.h"

#define NUM_CHANNELS 16

void testEncDec()
{
  int16_t channelOut[NUM_CHANNELS];
  //fill with random data
  randomSeed(micros());
  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    channelOut[i] = random(-500, 500);
  }
  
  //--------- encode -----------
  uint32_t encTimeQQ = micros();
  uint8_t dataBuff[21]; //always fixed at 21 bytes
  memset(dataBuff, 0, sizeof(dataBuff));
  for(uint8_t chIdx = 0; chIdx < NUM_CHANNELS && chIdx < 16; chIdx++)
  {
    uint16_t chVal = (channelOut[chIdx] + 500) & 0xFFFF; 
    //encode into 10bit resolution bitstream
    uint8_t aIdx = chIdx + (chIdx / 4);
    uint8_t bIdx = aIdx + 1;
    uint8_t aShift = ((chIdx % 4) + 1) * 2;
    uint8_t bShift = 8 - aShift;
    dataBuff[aIdx] |= ((chVal >> aShift) & 0xFF);
    dataBuff[bIdx] |= ((chVal << bShift) & 0xFF);
  }
  encTimeQQ = micros() - encTimeQQ;

  //-------- Decode -----------
  uint32_t decTimeQQ = micros();
  uint16_t chData[NUM_CHANNELS];
  for(uint8_t chIdx = 0; chIdx < NUM_CHANNELS && chIdx < 16; chIdx++)
  {
    uint8_t aIdx = chIdx + (chIdx / 4);
    uint8_t bIdx = aIdx + 1;
    uint8_t aShift = ((chIdx % 4) + 1) * 2;
    uint8_t bShift = 8 - aShift;
    uint16_t aMask = ((uint16_t)0x0400) - ((uint16_t)1 << aShift);
    uint8_t  bMask = ((uint16_t)1 << (8-bShift)) - 1;
    
    chData[chIdx] = (((uint16_t)dataBuff[aIdx] << aShift) & aMask) | (((uint16_t)dataBuff[bIdx] >> bShift) & bMask);
  }
  decTimeQQ = micros() - decTimeQQ;
  
  //-------- Print the input output ---------
  Serial.print("\r\nEncode time:");
  Serial.print(encTimeQQ);
  Serial.print("us\r\n");
  Serial.print("Decode time:");
  Serial.print(decTimeQQ);
  Serial.print("us\r\n");

  Serial.print("In, Out\r\n");
  for(uint8_t chIdx = 0; chIdx < NUM_CHANNELS; chIdx++)
  {
    Serial.print("Ch");
    Serial.print(chIdx);
    Serial.print(": ");
    Serial.print(channelOut[chIdx]);
    Serial.print(", ");
    int16_t val = chData[chIdx];
    val -= 500;
    Serial.print(val);
    Serial.println();
  }
}

void setup()
{
  Serial.begin(9600);
  delay(1000);
}

void loop()
{
  Serial.print("\r\n======== 10bit encode decode test ===========\r\n");
  for(uint8_t cnt = 0; cnt < 10; cnt++)
  {
    Serial.print("\r\n----- Run ");
    Serial.print(cnt + 1);
    Serial.print(" of 10 ------\r\n");
    
    testEncDec();
    delay(2000);
  }
  
  Serial.print("\r\n------Tests complete --------\r\n");
  
  while(1)
  {
  }
}