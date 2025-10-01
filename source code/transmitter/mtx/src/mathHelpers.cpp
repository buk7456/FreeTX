#include "Arduino.h"

#include "mathHelpers.h"

int32_t calcTangent(int16_t xValues[], int16_t yValues[], uint8_t numValues, uint8_t i);

//--------------------------------------------------------------------------------------------------

int32_t divRoundClosest(int32_t n, int32_t d)
{
  if(d == 0)
    return 0;
  else
    return ((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d);
}

//--------------------------------------------------------------------------------------------------

//Implementation of linear Interpolation using integers
//The x values must be ordered in ascending
//Formula used is y = y0 + ((x - x0)*(y1 - y0))/(x1 - x0);
//For infinite slope we simply return the immediate left y value if x is in the first half
//or the immediate right y value if x is in the second half

int16_t linearInterpolate(int16_t xValues[], int16_t yValues[], uint8_t numValues, int16_t xVal)
{
  int32_t y = 0;
  if(xVal < xValues[0]) 
    y = yValues[0]; 
  else if(xVal > xValues[numValues - 1])
    y = yValues[numValues - 1];
  else  //is in range, interpolate
  { 
    int16_t xMid = (xValues[0] + xValues[numValues - 1]) / 2;
    
    for(uint8_t i = 0; i < numValues - 1; i++)
    {
      if(xVal >= xValues[i] && xVal <= xValues[i + 1])
      {
        int32_t x0 = xValues[i];
        int32_t x1 = xValues[i + 1];
        int32_t y0 = yValues[i];
        int32_t y1 = yValues[i + 1];
        int32_t x = xVal;
        if(x0 == x1) //infinite slope
        {
          if(x <= xMid) 
            y = y0;
          else 
            y = y1;
        }
        else //interpolate
          y = (((x - x0) * (y1 - y0)) + (y0 * (x1 - x0))) / (x1 - x0);
        
        if(x <= xMid)
          break; 
      }
    }
  }
  return (int16_t) y; 
}

//--------------------------------------------------------------------------------------------------

// Implementation of hermite cubic spline interpolation.
// The x values must be ordered in ascending.
// See the following Wikipedia articles for the functions used.
// https://en.wikipedia.org/wiki/Cubic_Hermite_spline
// https://en.wikipedia.org/wiki/Monotone_cubic_interpolation
// For our implementation as we are using integer math, we need to use some multiplier to scale
// up the math to avoid losing precision. A multiplier of say 1000 is sufficient for our application.
// We then scale back the result after calculations.
// For infinite slope we simply return the immediate left y value if x is in the first half
// or the immediate right y value if x is in the second half

int16_t cubicHermiteInterpolate(int16_t xValues[], int16_t yValues[], uint8_t numValues, int16_t xVal)
{
  int32_t y = 0;
  if(xVal < xValues[0]) 
    y = yValues[0]; 
  else if(xVal > xValues[numValues - 1])
    y = yValues[numValues - 1];
  else  //is in range, interpolate
  {
    int16_t xMid = (xValues[0] + xValues[numValues - 1]) / 2;
    
    for(uint8_t i = 0; i < numValues - 1; i++)
    {
      if(xVal >= xValues[i] && xVal <= xValues[i + 1])
      {
        int16_t x0 = xValues[i];
        int16_t x1 = xValues[i + 1];
        int16_t y0 = yValues[i];
        int16_t y1 = yValues[i + 1];
        int16_t x = xVal;
        
        if(x1 == x0) //infinite slope
        {
          if(x <= xMid) 
            y = y0;
          else 
            y = y1;
        }
        else //interpolate
        {
          const int32_t multiplier = 1024; 
        
          int32_t dx = x1 - x0;
          int32_t t = (multiplier * (x - x0)) / dx;
          int32_t tpow2 = (t * t) / multiplier;
          int32_t tpow3 = (tpow2 * t) / multiplier;
          
          //hermite basis functions
          int32_t h00 = 2*tpow3 - 3*tpow2 + multiplier;
          int32_t h10 = tpow3 - 2*tpow2 + t;
          int32_t h01 = -2*tpow3 + 3*tpow2;
          int32_t h11 = tpow3 - tpow2;
          
          //tangents
          int32_t m0 = calcTangent(xValues, yValues, numValues, i);
          int32_t m1 = calcTangent(xValues, yValues, numValues, i + 1);
          
          //calculate y
          y = h00*y0 + ((h10*m0)/multiplier)*dx + h01*y1 + ((h11*m1)/multiplier)*dx;
          y /= multiplier;
        }
        
        if(x <= xMid)
          break; 
      }
    }
  }
  return (int16_t) y;
}

int32_t calcTangent(int16_t xValues[], int16_t yValues[], uint8_t numValues, uint8_t i)
{
  const int32_t multiplier = 1024;
  
  // Compute the slopes of the secant lines between successive points. 
  // For the endpoints, use one-sided differences.
  
  int32_t m = 0;
  if(i == 0) //endpoint
  {
    int16_t x0 = xValues[0];
    int16_t x1 = xValues[1];
    int16_t y0 = yValues[0];
    int16_t y1 = yValues[1];
    m = (multiplier * (y1 - y0)) / ((x1 != x0) ? (x1 - x0) : 1);
  }
  else if(i == numValues - 1) //endpoint
  {
    int16_t x0 = xValues[numValues - 2];
    int16_t x1 = xValues[numValues - 1];
    int16_t y0 = yValues[numValues - 2];
    int16_t y1 = yValues[numValues - 1];
    m = (multiplier * (y1 - y0)) / ((x1 != x0) ? (x1 - x0) : 1);
  }
  else //interior points
  {
    int16_t x0 = xValues[i - 1];
    int16_t x1 = xValues[i];
    int16_t x2 = xValues[i + 1];
    int16_t y0 = yValues[i - 1];
    int16_t y1 = yValues[i];
    int16_t y2 = yValues[i + 1];
    
    //calculate slopes of secant lines
    int32_t d0 = (multiplier * (y1 - y0)) / ((x1 != x0) ? (x1 - x0) : 1);
    int32_t d1 = (multiplier * (y2 - y1)) / ((x2 != x1) ? (x2 - x1) : 1);

    //initialise tangent as the average of the secants
    m = (d0 + d1) / 2;
    
    //apply rules for monotonicity
    if(d0 == 0 || d1 == 0 || (d0 > 0 && d1 < 0) || (d0 < 0 && d1 > 0))
      m = 0;
    else
    {
      /* 
      int32_t a = multiplier * m / d0;
      int32_t b = multiplier * m / d1;
      if(a > (multiplier * 3))
        m = 3 * d0;
      else if(b > (multiplier * 3))
        m = 3 * d1;  
      */
      //slightly faster implementation 
      int32_t p = d0 * 3;
      int32_t q = d1 * 3;
      if((p > 0 && m > p) || (p < 0 && m < p)) 
        m = p;
      else if((q > 0 && m > q) || (q < 0 && m < q))
        m = q;
    }
  }
  return m;
}


//Taken from the TinyGPS++ library.

double distanceBetween(double lat1, double long1, double lat2, double long2)
{
  // returns distance in meters between two positions, both specified
  // as signed decimal-degrees latitude and longitude. Uses great-circle
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  double delta = radians(long1-long2);
  double sdlong = sin(delta);
  double cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double slat1 = sin(lat1);
  double clat1 = cos(lat1);
  double slat2 = sin(lat2);
  double clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6372795;
}