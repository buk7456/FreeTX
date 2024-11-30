#ifndef _MATH_HELPERS_H_
#define _MATH_HELPERS_H_

int32_t divRoundClosest(int32_t n, int32_t d);
int16_t linearInterpolate(int16_t xValues[], int16_t yValues[], uint8_t numValues, int16_t xVal);
int16_t cubicHermiteInterpolate(int16_t xValues[], int16_t yValues[], uint8_t numValues, int16_t xVal);

double distanceBetween(double lat1, double long1, double lat2, double long2);

#endif
