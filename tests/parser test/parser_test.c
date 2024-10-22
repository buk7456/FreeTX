// Console application. 
// Parses and prints out the key-value from the specified model data file.
// Compile with gcc.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

#define MAX_STR_SIZE 21

char keyBuff[3][MAX_STR_SIZE];
char valueBuff[MAX_STR_SIZE];

bool isEndOfFile = false;

//Our own strlcpy since the standard C lib doesnt have it
size_t strlcpy(char *dest, const char *src, size_t size) 
{
  size_t src_len = 0;
  size_t i;
  // Calculate the length of the source string
  while (src[src_len] != '\0') 
    src_len++;
  // If the size is 0, return the length of the source string
  if (size == 0)
    return src_len;
  // Copy characters from src to dest, up to size - 1 characters
  for (i = 0; i < size - 1 && src[i] != '\0'; i++)
    dest[i] = src[i];
  // Null-terminate the destination string
  dest[i] = '\0';
  // Return the length of the source string
  return src_len;
}

//Trims in place the whitespace at begining and end of a the string
void trimWhiteSpace(char* buff, uint8_t lenBuff)
{
  if(buff[0] == '\0')
    return;
  
  //trim leading space
  uint8_t i = 0; 
  while(isspace(buff[i]))
  {    
    i++;
    if(i == lenBuff - 1) //all spaces
    {
      buff[0] = '\0';
      return;
    }
  }
  
  //trim trailing space
  if(lenBuff < 2)
    return;
  uint8_t j = lenBuff - 2;
  while(j > i && isspace(buff[j]))
    j--;
  
  //shift the remaining characters to the beginning of the string
  uint8_t k = 0;
  while (i <= j)
    buff[k++] = buff[i++];
  buff[k] = '\0';  //add a null terminator
}

void parser(const char* data, uint32_t dataLen)
{
  //--- Read characters one by one until a newline or end of file is encountered
  char lineBuff[MAX_STR_SIZE * 3]; //seems enough
  uint32_t count = 0;

  static uint32_t pos = 0; //move outside of function
  while(pos < dataLen)
  {
    uint8_t c = *(data + pos);
    pos++;
    
    if(c == '\r') //skip carriage return
      continue;
    if(c == '\n')
      break;
    if(count < sizeof(lineBuff) - 1)
      lineBuff[count++] = c;
  }
  
  lineBuff[count] = '\0'; //null terminate
  
  if(pos == dataLen)
    isEndOfFile = true;
  
  //--- Get the indent level
  uint8_t indentLevel = 0;
  static uint8_t prevIndentLevel = 0;
  uint8_t numLeadingSpaces = 0;
  for(uint8_t i = 0; i < sizeof(lineBuff); i++)
  {
    if(lineBuff[i] != ' ')
      break;
    numLeadingSpaces++;
  }
  indentLevel = numLeadingSpaces / 2;
  
  //--- Trim white space at beginning and end
  trimWhiteSpace(lineBuff, sizeof(lineBuff));
  
  //--- Check if the line is a comment line or empty line
  if(lineBuff[0] == '#' || lineBuff[0] == '\0')
    return;
  
  //--- Check if invalid indention
  if(indentLevel >= sizeof(keyBuff)/sizeof(keyBuff[0]) || numLeadingSpaces % 2 != 0)
    return;
  
  //--- Find and remove inline comments
  //Find the '#' character in the line.
  //If '#' is found, truncate the line at that position to remove the comment
  char *commentStart = strchr(lineBuff, '#');
  if (commentStart != NULL)
    *commentStart = '\0';
  
  //--- Get the key
  char tempKeyStr[MAX_STR_SIZE];
  //copy all characters before the ':'
  count = 0;
  for(uint8_t i = 0; i < sizeof(lineBuff); i++)
  {
    uint8_t c = lineBuff[i];
    if(c == '\0')
      break;
    if(c == ':')
    {
      lineBuff[i] = ' '; //substitute with space
      break;
    }
    if(count < sizeof(tempKeyStr) - 1)
      tempKeyStr[count++] = c;
    lineBuff[i] = ' '; //substitute with space
  }
  tempKeyStr[count] = '\0'; //null terminate
  //copy from temp into corresponding buffer
  strlcpy(keyBuff[indentLevel], tempKeyStr, sizeof(keyBuff[0]));
  //check if we have moved backwards of the tree, in which case we nullify all children
  if(indentLevel < prevIndentLevel) 
  {
    uint8_t i = prevIndentLevel;
    while(i > indentLevel)
    {
      keyBuff[i][0] = '\0';
      i--;
    }
  }
  
  //--- Get the value
  trimWhiteSpace(lineBuff, sizeof(lineBuff));
  strlcpy(valueBuff, lineBuff, sizeof(valueBuff));
  
  //--- Save the indention level
  prevIndentLevel = indentLevel;
}

int main(int argc, char *argv[])
{
  if(argc != 2) 
  {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }
  
  // Open the file in binary mode
  FILE *file = fopen(argv[1], "rb");
  if(file == NULL) 
  {
    perror("Error opening file");
    return 1;
  }
  
  //read the data into our own buffer
  uint8_t testData[131072]; //128 KiB
  memset(testData, 0, sizeof(testData));
  uint32_t bytesRead = fread(testData, 1, sizeof(testData), file);
  
  //close the file
  fclose(file);

  //--- initialise
  isEndOfFile = false;
  memset(keyBuff[0], 0, sizeof(keyBuff[0]));
  memset(keyBuff[1], 0, sizeof(keyBuff[0]));
  memset(keyBuff[2], 0, sizeof(keyBuff[0]));
  memset(valueBuff, 0, sizeof(valueBuff));
  
  //--- loop
  while(!isEndOfFile)
  {
    //clear value buffer so we dont work with stale values
    *valueBuff = '\0';

    //call parser
    parser((char *)testData, bytesRead);

    //if value is empty, continue parsing
    if(*valueBuff == '\0')
      continue;
    
    /*     
    //print the keys and values
    for(uint8_t i = 0; i < sizeof(keyBuff)/sizeof(keyBuff[0]); i++)
    {
      if(*keyBuff[i])
        printf("%s ", keyBuff[i]);
    }
    printf("= %s\n", valueBuff); 
    */
    
    //--- Print in tabulated form
    // Construct the format string dynamically using MAX_STR_SIZE
    char format[20];
    snprintf(format, sizeof(format), "%%-%d.%ds", MAX_STR_SIZE, MAX_STR_SIZE);
    // Header
    static bool hasPutHeader = false;
    if(!hasPutHeader)
    {
      hasPutHeader = true;
      
      printf("\n");
      
      char str[MAX_STR_SIZE];
      for(uint8_t i = 0; i < sizeof(keyBuff)/sizeof(keyBuff[0]); i++) 
      {
        snprintf(str, sizeof(str), "Key[%d]", i);
        printf(format, str);
      }
      printf(format, "Value");
      printf("\n");
      
      for(size_t i = 0; i < (MAX_STR_SIZE * sizeof(keyBuff)/sizeof(keyBuff[0]) + MAX_STR_SIZE); i++)
      {
        printf("-");
      }
      printf("\n");
    }
    // print the keys and values
    for(uint8_t i = 0; i < sizeof(keyBuff)/sizeof(keyBuff[0]); i++) 
    {
      printf(format, keyBuff[i]);
    }
    printf(format, valueBuff);
    printf("\n");
    
  }
  
  return 0;
}