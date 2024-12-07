#include "Arduino.h"

#include "common.h" 
#include "GNSS.h"

#define MAX_FIELDS 16
#define MAX_FIELD_SIZE 16
char fields[MAX_FIELDS][MAX_FIELD_SIZE];

typedef struct {
  char latitude[MAX_FIELD_SIZE];
  char lat_dir[2];   // N/S indicator
  char longitude[MAX_FIELD_SIZE];
  char lon_dir[2];   // E/W indicator
  char speed[10];
  char course[10];
  char fix_indicator[2];
  char satellites_used[5];
  char satellites_in_view_gps[5];
  char satellites_in_view_glonass[5];
  char satellites_in_view_beidou[5];
  char satellites_in_view_galileo[5];
  char hdop[10];
  char msl_altitude[10];
} gnss_data_t;

gnss_data_t GNSSInfo;

void tokenize(const char *sentence, char tokens[][MAX_FIELD_SIZE], uint8_t *count, uint8_t maxNumTokens) 
{
  *count = 0;
  const char *start = sentence;
  const char *end = strchr(start, ',');
  
  while(end != NULL) 
  {
    if(*count >= maxNumTokens - 1) //prevent buffer overflow
      break;
    
    uint8_t length = end - start;
    if(length > 0 && length < (MAX_FIELD_SIZE - 1))
    {
      strncpy(tokens[*count], start, length);
      tokens[*count][length] = '\0'; // Null-terminate
    }
    else
    {
      tokens[*count][0] = '\0'; // Empty field
    }
    (*count)++;
    start = end + 1;
    end = strchr(start, ',');
  }

  // Handle the last token (after the last comma or the entire string if no commas)
  if(*start) 
  {
    strncpy(tokens[*count], start, MAX_FIELD_SIZE - 1);
    tokens[*count][MAX_FIELD_SIZE - 1] = '\0';
    (*count)++;
  }
}

void parseRMC(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 10) // Ensure enough fields exist
  {
    strlcpy(GNSSInfo.latitude, fields[3], sizeof(GNSSInfo.latitude));
    strlcpy(GNSSInfo.lat_dir, fields[4], sizeof(GNSSInfo.lat_dir));
    strlcpy(GNSSInfo.longitude, fields[5], sizeof(GNSSInfo.longitude));
    strlcpy(GNSSInfo.lon_dir, fields[6], sizeof(GNSSInfo.lon_dir));
    strlcpy(GNSSInfo.speed, fields[7], sizeof(GNSSInfo.speed));
    strlcpy(GNSSInfo.course, fields[8], sizeof(GNSSInfo.course));
  }
}

void parseGGA(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 10) // Ensure enough fields exist
  {
    // strlcpy(GNSSInfo.latitude, fields[2], sizeof(GNSSInfo.latitude));
    // strlcpy(GNSSInfo.lat_dir, fields[3], sizeof(GNSSInfo.lat_dir));
    // strlcpy(GNSSInfo.longitude, fields[4], sizeof(GNSSInfo.longitude));
    // strlcpy(GNSSInfo.lon_dir, fields[5], sizeof(GNSSInfo.lon_dir));
    strlcpy(GNSSInfo.fix_indicator, fields[6], sizeof(GNSSInfo.fix_indicator));
    strlcpy(GNSSInfo.satellites_used, fields[7], sizeof(GNSSInfo.satellites_used));
    strlcpy(GNSSInfo.hdop, fields[8], sizeof(GNSSInfo.hdop));
    strlcpy(GNSSInfo.msl_altitude, fields[9], sizeof(GNSSInfo.msl_altitude));
  }
}

void parseGSV_GPS(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 3) // Ensure enough fields exist
  { 
    strlcpy(GNSSInfo.satellites_in_view_gps, fields[3], sizeof(GNSSInfo.satellites_in_view_gps));
  }
}

void parseGSV_GLONASS(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 3) // Ensure enough fields exist
  { 
    strlcpy(GNSSInfo.satellites_in_view_glonass, fields[3], sizeof(GNSSInfo.satellites_in_view_glonass));
  }
}

void parseGSV_BEIDOU(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 3) // Ensure enough fields exist
  { 
    strlcpy(GNSSInfo.satellites_in_view_beidou, fields[3], sizeof(GNSSInfo.satellites_in_view_beidou));
  }
}

void parseGSV_GALILEO(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 3) // Ensure enough fields exist
  { 
    strlcpy(GNSSInfo.satellites_in_view_galileo, fields[3], sizeof(GNSSInfo.satellites_in_view_galileo));
  }
}

void parseNMEA(const char *sentence) 
{
  if(strncmp(sentence + 3, "RMC", 3) == 0) 
    parseRMC(sentence);
  else if(strncmp(sentence + 3, "GGA", 3) == 0) 
    parseGGA(sentence);
  else if(strncmp(sentence, "$GPGSV", 6) == 0) 
    parseGSV_GPS(sentence); 
  else if(strncmp(sentence, "$GLGSV", 6) == 0) 
    parseGSV_GLONASS(sentence);
  else if(strncmp(sentence, "$GBGSV", 6) == 0) 
    parseGSV_BEIDOU(sentence); 
  else if(strncmp(sentence, "$GAGSV", 6) == 0) 
    parseGSV_GALILEO(sentence);
}

void convertGNSSData()
{
  float fval;
  
  //latitude ddmm.mmmmm
  if(*GNSSInfo.latitude)
  {
    char latDegStr[3];
    char latMinStr[9];
    strlcpy(latDegStr, GNSSInfo.latitude, sizeof(latDegStr));
    strlcpy(latMinStr, GNSSInfo.latitude + 2, sizeof(latMinStr));
    fval = atof(latDegStr);
    fval += (atof(latMinStr) / 60.0);
    fval *= 100000.0;
    if(*GNSSInfo.lat_dir == 'S')
      fval = -fval;
    GNSSTelemetryData.latitude = (int32_t) fval;
  }
  else
    GNSSTelemetryData.latitude = 0;
    
  //longitude dddmm.mmmmm
  if(*GNSSInfo.longitude)
  {
    char lngDegStr[4];
    char lngMinStr[9];
    strlcpy(lngDegStr, GNSSInfo.longitude, sizeof(lngDegStr));
    strlcpy(lngMinStr, GNSSInfo.longitude + 3, sizeof(lngMinStr));
    fval = atof(lngDegStr);
    fval += (atof(lngMinStr) / 60.0);
    fval *= 100000.0;
    if(*GNSSInfo.lon_dir == 'W')
      fval = -fval;
    GNSSTelemetryData.longitude = (int32_t) fval;
  }
  else
    GNSSTelemetryData.longitude = 0;
    
  //hdop
  
  //satellites in use
  GNSSTelemetryData.satellitesInUse = atoi(GNSSInfo.satellites_used);
  
  //satellites in view
  GNSSTelemetryData.satellitesInView = atoi(GNSSInfo.satellites_in_view_gps);
  GNSSTelemetryData.satellitesInView += atoi(GNSSInfo.satellites_in_view_glonass);
  GNSSTelemetryData.satellitesInView += atoi(GNSSInfo.satellites_in_view_beidou);
  GNSSTelemetryData.satellitesInView += atoi(GNSSInfo.satellites_in_view_galileo);
  
  if(GNSSTelemetryData.satellitesInView < GNSSTelemetryData.satellitesInUse)
    GNSSTelemetryData.satellitesInView = GNSSTelemetryData.satellitesInUse;
  
  //fix indicator
  GNSSTelemetryData.positionFix = atoi(GNSSInfo.fix_indicator);
  
  //speed
  fval = atof(GNSSInfo.speed) * 0.51444;
  fval *= 10.0;
  GNSSTelemetryData.speed = (uint16_t) fval;
  
  //course
  fval = atof(GNSSInfo.course);
  fval *= 10.0;
  GNSSTelemetryData.course = (uint16_t) fval;
  
  //msl altitude
  fval = atof(GNSSInfo.msl_altitude);
  GNSSTelemetryData.altitude = (int16_t) fval;
}
