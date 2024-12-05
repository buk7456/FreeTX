#include "Arduino.h"

#include "common.h" 
#include "GPS.h"

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
  char satellites_in_view[5];
  char hdop[10];
  char msl_altitude[10];
} gps_data_t;

gps_data_t GPSInfo;

void tokenize(const char *sentence, char tokens[][MAX_FIELD_SIZE], uint8_t *count, size_t maxNumTokens) 
{
  *count = 0;
  const char *start = sentence;
  const char *end = strchr(start, ',');
  
  while(end != NULL) 
  {
    if(*count >= maxNumTokens - 1) //prevent buffer overflow
      break;
    
    size_t length = end - start;
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

void parseGPRMC(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 10) // Ensure enough fields exist
  {
    strlcpy(GPSInfo.latitude, fields[3], sizeof(GPSInfo.latitude));
    strlcpy(GPSInfo.lat_dir, fields[4], sizeof(GPSInfo.lat_dir));
    strlcpy(GPSInfo.longitude, fields[5], sizeof(GPSInfo.longitude));
    strlcpy(GPSInfo.lon_dir, fields[6], sizeof(GPSInfo.lon_dir));
    strlcpy(GPSInfo.speed, fields[7], sizeof(GPSInfo.speed));
    strlcpy(GPSInfo.course, fields[8], sizeof(GPSInfo.course));
  }
}

void parseGPGGA(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 10) // Ensure enough fields exist
  {
    // strlcpy(GPSInfo.latitude, fields[2], sizeof(GPSInfo.latitude));
    // strlcpy(GPSInfo.lat_dir, fields[3], sizeof(GPSInfo.lat_dir));
    // strlcpy(GPSInfo.longitude, fields[4], sizeof(GPSInfo.longitude));
    // strlcpy(GPSInfo.lon_dir, fields[5], sizeof(GPSInfo.lon_dir));
    strlcpy(GPSInfo.fix_indicator, fields[6], sizeof(GPSInfo.fix_indicator));
    strlcpy(GPSInfo.satellites_used, fields[7], sizeof(GPSInfo.satellites_used));
    strlcpy(GPSInfo.hdop, fields[8], sizeof(GPSInfo.hdop));
    strlcpy(GPSInfo.msl_altitude, fields[9], sizeof(GPSInfo.msl_altitude));
  }
}

void parseGPGSV(const char *sentence) 
{
  uint8_t count;
  tokenize(sentence, fields, &count, sizeof(fields)/sizeof(fields[0]));
  if(count > 3) // Ensure enough fields exist
  { 
    strlcpy(GPSInfo.satellites_in_view, fields[3], sizeof(GPSInfo.satellites_in_view));
  }
}

void parseNMEA(const char *sentence) 
{
  if(strncmp(sentence, "$GPRMC", 6) == 0) 
    parseGPRMC(sentence);
  else if(strncmp(sentence, "$GPGGA", 6) == 0) 
    parseGPGGA(sentence);
  else if(strncmp(sentence, "$GPGSV", 6) == 0) 
    parseGPGSV(sentence);
}

void convertGPSData()
{
  float fval;
  
  //latitude ddmm.mmmmm
  if(*GPSInfo.latitude)
  {
    char latDegStr[3];
    char latMinStr[9];
    strlcpy(latDegStr, GPSInfo.latitude, sizeof(latDegStr));
    strlcpy(latMinStr, GPSInfo.latitude + 2, sizeof(latMinStr));
    fval = atof(latDegStr);
    fval += (atof(latMinStr) / 60.0);
    fval *= 100000.0;
    if(*GPSInfo.lat_dir == 'S')
      fval = -fval;
    GNSSTelemetryData.latitude = (int32_t) fval;
  }
  else
    GNSSTelemetryData.latitude = 0;
    
  //longitude dddmm.mmmmm
  if(*GPSInfo.longitude)
  {
    char lngDegStr[4];
    char lngMinStr[9];
    strlcpy(lngDegStr, GPSInfo.longitude, sizeof(lngDegStr));
    strlcpy(lngMinStr, GPSInfo.longitude + 3, sizeof(lngMinStr));
    fval = atof(lngDegStr);
    fval += (atof(lngMinStr) / 60.0);
    fval *= 100000.0;
    if(*GPSInfo.lon_dir == 'W')
      fval = -fval;
    GNSSTelemetryData.longitude = (int32_t) fval;
  }
  else
    GNSSTelemetryData.longitude = 0;
    
  //hdop
  
  //satellites in use
  GNSSTelemetryData.satellitesInUse = atoi(GPSInfo.satellites_used);
  
  //satellites in view
  GNSSTelemetryData.satellitesInView = atoi(GPSInfo.satellites_in_view);
  
  //fix indicator
  GNSSTelemetryData.positionFix = atoi(GPSInfo.fix_indicator);
  
  //speed
  fval = atof(GPSInfo.speed) * 0.51444;
  fval *= 10.0;
  GNSSTelemetryData.speed = (uint16_t) fval;
  
  //course
  fval = atof(GPSInfo.course);
  fval *= 10.0;
  GNSSTelemetryData.course = (uint16_t) fval;
  
  //msl altitude
  fval = atof(GPSInfo.msl_altitude);
  GNSSTelemetryData.altitude = (int16_t) fval;
}
