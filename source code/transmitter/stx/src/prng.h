#ifndef _PRNG_H_
#define _PRNG_H_

// Pseudo Random Number Generator

// Returns a random signed 16-bit value.
int16_t myRandom(); 

// Returns a random value in [0, max], inclusive.
int16_t myRandom(int16_t max); 

// Returns a random value in [min, max], inclusive.
int16_t myRandom(int16_t min, int16_t max); 

// Initialises the PRNG with the seed value.
void myRandomSeed(int16_t seed); 

#endif
