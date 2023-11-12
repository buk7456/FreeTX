#include "Arduino.h"

#include "config.h"
#include "crc.h"
#include "common.h"
#include "eestore.h"
#include "rfComm.h"

void doSerialCommunication();

//==================================================================================================

void setup()
{
  //--- init serial port
  Serial.begin(115200);  
  
  //--- delay
  delay(100);
  
  //--- set defaults
  Sys.transmitterID = 0x01;
  Sys.receiverID = 0x00;
  for(uint8_t i = 0; i< sizeof(Sys.fhss_schema)/sizeof(Sys.fhss_schema[0]); i++)
    Sys.fhss_schema[i] = i;
  
  //--- eeprom 
  eeStoreInit();
  eeReadSysConfig();
 
  //--- initialise radio module
  initialiseRfModule();
  
}

//==================================================================================================

void loop()
{
  //--- SERIAL COMMUNICATION
  doSerialCommunication();
  
  //--- RF COMMUNICATION
  doRfCommunication();
  
}

//==================================================================================================

void doSerialCommunication()
{
  //See the 'protocol_over_uart.txt' in the doc folder for information about the formats used.
  
  ///------------- GET FROM MAIN MCU ------------------
  
  //Read into temp buff
 
  const uint8_t NUM_CRC_BYTES = 1;
  const uint8_t msgLength = 2 + (2 * NUM_RC_CHANNELS) + NUM_CRC_BYTES;
  
  uint8_t tmpBuff[msgLength];
  memset(tmpBuff, 0, sizeof(tmpBuff));
  
  if(Serial.available() < msgLength)
    return;
  
  uint8_t cntr = 0;
  while (Serial.available() > 0)
  {
    if(cntr < msgLength) 
    {
      tmpBuff[cntr] = Serial.read();
      cntr++;
    }
    else //Discard any extra data
      Serial.read();
  }
  
  //Check crc and extract data
  
  if(tmpBuff[msgLength - 1] != crc8(tmpBuff, msgLength - 1))
    return;
  
  //status byte 0
  uint8_t status0 = tmpBuff[0];
  rfPower = status0 & 0x07;
  rfEnabled = (status0 >> 3) & 0x01;
  isSendOutputChConfig = (status0 >> 4) & 0x01;
  isRequestingOutputChConfig = (status0 >> 5) & 0x01;
  isRequestingTelemetry = (status0 >> 6) & 0x01; 
  isRequestingBind = (status0 >> 7) & 0x01;
  
  //status byte 1
  uint8_t status1 = tmpBuff[1];
  isFailsafeData = status1 & 0x01;
  isMainReceiver = (status1 >> 1) & 0x01;
  
  //general data
  if(isSendOutputChConfig)
  {
    for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
      chConfigData[i] = tmpBuff[2 + i];
  }
  else
  {
    hasPendingRCData = true;
    for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
      chData[i] = joinBytes(tmpBuff[2 + i * 2], tmpBuff[3 + i * 2]);
    //Skip this rc data if we were previously requesting for telemetry
    //to allow enough time to listen. 
    static bool wasRequestingTelemetry = false;
    if(wasRequestingTelemetry)
      hasPendingRCData = false;
    wasRequestingTelemetry = isRequestingTelemetry;
  }
  
  ///------------- REPLY TO MAIN MCU ------------------
  
  uint8_t dataToSend[3 + NUM_RC_CHANNELS + NUM_CRC_BYTES];
  memset(dataToSend, 0, sizeof(dataToSend));

  dataToSend[0] |= (gotOutputChConfig & 0x01) << 4;
  dataToSend[0] |= (receiverConfigStatusCode & 0x03) << 2;
  dataToSend[0] |= (bindStatusCode & 0x03);
  
  dataToSend[1] = transmitterPacketRate;
  dataToSend[2] = receiverPacketRate;
  
  if(gotOutputChConfig)
  {
    gotOutputChConfig = false;
    for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
      dataToSend[3 + i] = chConfigData[i];
  }
  else //send telemetry
  {
    //The telemetry gets sent in a circular manner
    //e.g. if we have 6 fields a,b,c,d,e,f and the maximum we can send at a time is 4 fields,
    //then we would send a,b,c,d and the next time send e,f,a,b
    
    const uint8_t MAX_FIELDS = NUM_RC_CHANNELS / 3; // there are 3 bytes per telemetry field
    static uint8_t cntr = 0;
    for(uint8_t i = 0; i < MAX_FIELDS; i++)
    {
      if(i >= MAX_TELEMETRY_COUNT)
        break;
      dataToSend[3 + i * 3] = telemID[cntr];
      uint16_t _tlmVal = telemVal[cntr];
      dataToSend[4 + i * 3] = (_tlmVal >> 8) & 0xFF;
      dataToSend[5 + i * 3] = _tlmVal & 0xFF;
      //increment counter
      cntr++;
      if(cntr >= MAX_TELEMETRY_COUNT)
        cntr = 0;
    }
  }
  
  //add a CRC
  dataToSend[sizeof(dataToSend) - 1] = crc8(dataToSend, sizeof(dataToSend) - 1);
  
  //send
  Serial.write(dataToSend, sizeof(dataToSend));
  
  //reset flags
  bindStatusCode = 0;
  receiverConfigStatusCode = 0;
  gotOutputChConfig = false;
}
