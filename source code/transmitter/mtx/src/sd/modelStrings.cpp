#include "Arduino.h"

#include "../mtx.h"
#include "modelStrings.h"

char* findStringInIdStr(const id_string_t *idStr_P, int8_t searchId)
{
  static char retStr[MAX_STR_SIZE];
  uint8_t i = 0;
  while(1)
  {
    if(pgm_read_byte(&idStr_P[i].str) == '\0' || i == 0xff)
      break;
    if(pgm_read_byte(&idStr_P[i].id) == (uint8_t)searchId)
    {
      strlcpy_P(retStr, (const char*)&idStr_P[i].str, sizeof(retStr));
      return retStr;
    }
    i++;
  }
  return NULL;
}

template <typename T> void findIdInIdStr(const id_string_t *idStr_P, const char *searchStr, T &val) 
{
  uint8_t i = 0;
  while(1)
  {
    if(pgm_read_byte(&idStr_P[i].str) == '\0' || i == 0xff)
      break;
    if(MATCH_P(searchStr, (const char*)&idStr_P[i].str))
    {
      val = pgm_read_byte(&idStr_P[i].id);
      break;
    }
    i++;
  }
}
// Explicit instantiation for the types we want to use, otherwise linking fails with 'undefined reference to..'
// Add more instantiations as needed
template void findIdInIdStr<uint8_t>(const id_string_t *idStr_P, const char *searchStr, uint8_t&);
template void findIdInIdStr<int8_t>(const id_string_t *idStr_P, const char *searchStr, int8_t&);
template void findIdInIdStr<uint16_t>(const id_string_t *idStr_P, const char *searchStr, uint16_t&);
template void findIdInIdStr<int16_t>(const id_string_t *idStr_P, const char *searchStr, int16_t&);

