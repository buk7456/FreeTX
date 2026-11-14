#include "Arduino.h"

#include "prng.h"

uint32_t s = 1;   // any seed is valid, including 0

uint16_t rand16(void)
{
  s = s * 1664525u + 1013904223u;  // Implements a Linear Congruential Generator (LCG)
  return (uint16_t)(s >> 16); // We are interested in the upper 16 bits only.
}

void myRandomSeed(int16_t seed) 
{
  s = (uint16_t)seed;
}

int16_t myRandom() 
{
  return (int16_t)rand16();
}

int16_t myRandom(int16_t max) 
{
  if(max <= 0)
    return 0;
  return (int16_t)(rand16() % ((uint16_t)max + 1));
}

int16_t myRandom(int16_t min, int16_t max) 
{
  if(max <= min)
    return min;
  uint16_t range = (uint16_t)((int32_t)max - min + 1);
  return (int16_t)((int32_t)min + (rand16() % range));
}

