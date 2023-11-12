
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>


int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int32_t divRoundClosest(int32_t n, int32_t d)
{
  if(d == 0)
    return 0;
  else
    return ((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d);
}

int main()
{
  int32_t period = 500;
  int16_t result = 0;
  
  printf("TimeInstance\tResult\tResult/5\tRound(Result/5)\n");
  
  for(int32_t timeInstance = 0; timeInstance < period; timeInstance++)
  {
    
    //-------- sawtooth --------
    /*    
    if(timeInstance < (period/2))
      result = map(timeInstance, 0, (period/2) - 1, 0, 500);
    else
      result = map(timeInstance, period/2, period - 1, -500, 0); 
    */
    
    if(timeInstance < (period/2))
      result = map(timeInstance, 0, (period/2), 0, 500);
    else
      result = map(timeInstance, period/2, period, -500, 0);
    
    //-------- triangle -------
    /* if(timeInstance < (period/4))
      result = map(timeInstance, 0, period/4, 0, 500);
    else if(timeInstance < ((3 * period)/4))
      result = map(timeInstance, period/4, (3 * period)/4, 500, -500);
    else
      result = map(timeInstance, (3 * period)/4, period, -500, 0); */
    
    printf("%d\t%d\t%d\t%d\n", timeInstance, result, result/5, divRoundClosest(result, 5));
  }

  return 0;
}