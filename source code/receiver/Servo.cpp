
/*
  Adapted by Buk, from the Arduino servo library.
  
  This library has been tailored to this project. 
  Changes made:
    - Stripped code to retain only AVR specific code that applies to the Atmega328.
    - Changed MIN_PULSE_WIDTH to 500, MAX_PULSE_WIDTH to 2500.
    - Using int16_t instead of int8_t for the private class members min and max.
    - Removed methods that we do not need for this application.
    - Replaced generic int types with fixed width types (int16_t, etc).
    
  Original copyright notice is below.
*/
  
/*
 Servo.cpp - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
 Copyright (c) 2009 Michael Margolis.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <avr/interrupt.h>
#include <Arduino.h>

#include "Servo.h"

#define usToTicks(_us)  ((clockCyclesPerMicrosecond()* _us) / 8) // converts microseconds to tick (assumes prescale of 8)
#define TRIM_DURATION   2  // compensation ticks to trim adjust for digitalWrite delays

static servo_t servos[MAX_SERVOS];  // static array of servo structures
static volatile int8_t Channel[_Nbr_16timers ];  // counter for the servo being pulsed for each timer (or -1 if refresh interval)

uint8_t ServoCount = 0; // the total number of attached servos


// convenience macros
#define SERVO_INDEX_TO_TIMER(_servo_nbr) ((timer16_Sequence_t)(_servo_nbr / SERVOS_PER_TIMER)) // returns the timer controlling this servo
#define SERVO_INDEX(_timer,_channel)  ((_timer*SERVOS_PER_TIMER) + _channel)  // macro to access servo index by timer and channel
#define SERVO(_timer,_channel)  (servos[SERVO_INDEX(_timer,_channel)])   // macro to access servo class by timer and channel

#define SERVO_MIN() (MIN_PULSE_WIDTH - this->min * 4)  // minimum value in us for this servo
#define SERVO_MAX() (MAX_PULSE_WIDTH - this->max * 4)  // maximum value in us for this servo

/************ static functions common to all instances ***********************/

static inline void handle_interrupts(timer16_Sequence_t timer, volatile uint16_t *TCNTn, volatile uint16_t* OCRnA)
{
  if(Channel[timer] < 0)
    *TCNTn = 0; // channel set to -1 indicated that refresh interval completed so reset the timer
  else
  {
    if(SERVO_INDEX(timer,Channel[timer]) < ServoCount && SERVO(timer,Channel[timer]).Pin.isActive)
      digitalWrite(SERVO(timer,Channel[timer]).Pin.nbr, LOW); // pulse this channel low if activated
  }

  Channel[timer]++;    // increment to the next channel
  if(SERVO_INDEX(timer,Channel[timer]) < ServoCount && Channel[timer] < SERVOS_PER_TIMER) 
  {
    *OCRnA = *TCNTn + SERVO(timer,Channel[timer]).ticks;
    if(SERVO(timer,Channel[timer]).Pin.isActive)     // check if activated
      digitalWrite(SERVO(timer,Channel[timer]).Pin.nbr, HIGH); // its an active channel so pulse it high
  }
  else 
  {
    // finished all channels so wait for the refresh period to expire before starting over
    if(((unsigned)*TCNTn) + 4 < usToTicks(REFRESH_INTERVAL))  // allow a few ticks to ensure the next OCR1A not missed
      *OCRnA = (uint16_t) usToTicks(REFRESH_INTERVAL);
    else
      *OCRnA = *TCNTn + 4;  // at least REFRESH_INTERVAL has elapsed
    Channel[timer] = -1; // this will get incremented at the end of the refresh period to start again at the first channel
  }
}

ISR(TIMER1_COMPA_vect)
{
  handle_interrupts(_timer1, &TCNT1, &OCR1A);
}

static void initISR(timer16_Sequence_t timer)
{
  if(timer == _timer1) 
  {
    TCCR1A = 0;              // normal counting mode
    TCCR1B = _BV(CS11);      // set prescaler of 8
    TCNT1 = 0;               // clear the timer count
    TIFR1 |= _BV(OCF1A);     // clear any pending interrupts
    TIMSK1 |=  _BV(OCIE1A); // enable the output compare interrupt
  }
}

static boolean isTimerActive(timer16_Sequence_t timer)
{
  // returns true if any servo is active on this timer
  for(uint8_t channel = 0; channel < SERVOS_PER_TIMER; channel++) 
  {
    if(SERVO(timer,channel).Pin.isActive)
      return true;
  }
  return false;
}

/****************** end of static functions ******************************/

Servo::Servo()
{
  if(ServoCount < MAX_SERVOS) 
  {
    this->servoIndex = ServoCount++;  // assign a servo index to this instance
    servos[this->servoIndex].ticks = usToTicks(DEFAULT_PULSE_WIDTH); // store default values
  }
  else
    this->servoIndex = INVALID_SERVO;  // too many servos
}

uint8_t Servo::attach(int16_t pin)
{
  return this->attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int16_t pin, int16_t min, int16_t max)
{
  if(this->servoIndex < MAX_SERVOS) 
  {
    pinMode(pin, OUTPUT);
    servos[this->servoIndex].Pin.nbr = pin;
   
    this->min  = (MIN_PULSE_WIDTH - min)/4; //resolution of min/max is 4 us
    this->max  = (MAX_PULSE_WIDTH - max)/4;
    // initialize the timer if it has not already been initialized
    timer16_Sequence_t timer = SERVO_INDEX_TO_TIMER(servoIndex);
    if(!isTimerActive(timer))
      initISR(timer);
    servos[this->servoIndex].Pin.isActive = true;
  }
  return this->servoIndex;
}

void Servo::detach()
{
  servos[this->servoIndex].Pin.isActive = false;
}

void Servo::writeMicroseconds(int16_t value)
{
  // calculate and store the values for the given channel
  uint8_t channel = this->servoIndex;
  if(channel < MAX_SERVOS)
  {
    if(value < SERVO_MIN())
      value = SERVO_MIN();
    else if(value > SERVO_MAX())
      value = SERVO_MAX();

    value = value - TRIM_DURATION;
    value = usToTicks(value);  // convert to ticks after compensating for interrupt overhead

    uint8_t oldSREG = SREG;
    cli();
    servos[channel].ticks = value;
    SREG = oldSREG;
  }
}
