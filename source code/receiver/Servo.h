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
  Servo.h - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
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

/* 
  A servo is activated by creating an instance of the Servo class passing 
  the desired pin to the attach() method.
  The servos are pulsed in the background using the value most recently written.

  Note that analogWrite of PWM on pins associated with the timer are 
  disabled when the first servo is attached.

  The methods are:

    Servo - Class for manipulating servo motors connected to Arduino pins.

    attach(pin)  - Attaches a servo motor to an I/O pin.
    attach(pin, min, max  ) - Attaches to a pin setting min and max values in microseconds
    writeMicroseconds() - Sets the servo pulse width in microseconds 
    detach()    - Stops an attached servos from pulsing its I/O pin. 
*/

#ifndef Servo_h
#define Servo_h

#include <inttypes.h>

// Say which 16 bit timers can be used and in what order
#define _useTimer1
typedef enum { _timer1, _Nbr_16timers } timer16_Sequence_t;


#define Servo_VERSION           2     // software version of this library

#define MIN_PULSE_WIDTH       500     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      2500     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define REFRESH_INTERVAL    20000     // minimum time to refresh servos in microseconds 

#define SERVOS_PER_TIMER       12     // the maximum number of servos controlled by one timer 
#define MAX_SERVOS   (_Nbr_16timers  * SERVOS_PER_TIMER)

#define INVALID_SERVO         255     // flag indicating an invalid servo index

typedef struct  {
  uint8_t nbr        :6 ;             // a pin number from 0 to 63
  uint8_t isActive   :1 ;             // true if this channel is enabled, pin not pulsed if false 
} ServoPin_t   ;  

typedef struct {
  ServoPin_t Pin;
  volatile uint16_t ticks;
} servo_t;

class Servo
{
public:
  Servo();
  uint8_t attach(int16_t pin);       // attach the given pin to the next free channel, sets pinMode, returns channel number or 0 if failure
  uint8_t attach(int16_t pin, int16_t min, int16_t max); // as above but also sets min and max values for writes. 
  void detach();
  void writeMicroseconds(int16_t value); 
private:
   uint8_t servoIndex;  // index into the channel data for this servo
   int16_t min;         // minimum is this value times 4 added to MIN_PULSE_WIDTH    
   int16_t max;         // maximum is this value times 4 added to MAX_PULSE_WIDTH   
};


#endif
