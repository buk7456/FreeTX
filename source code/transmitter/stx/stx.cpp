#include "Arduino.h"

#include "config.h"
#include "crc.h"
#include "common.h"
#include "eestore.h"
#include "rfComm.h"

void getSerialData();
void sendSerialData();

#define UART_FIXED_PACKET_SIZE 48

enum {
  MESSAGE_TYPE_NONE = 0x00,
  
  //mtx to stx
  MESSAGE_TYPE_RC_DATA = 0x01, 
  MESSAGE_TYPE_ENTER_BIND = 0X02,
  MESSAGE_TYPE_GET_RECEIVER_CONFIG = 0x03,
  MESSAGE_TYPE_WRITE_RECEIVER_CONFIG = 0x04,
  
  //stx to mtx
  MESSAGE_TYPE_BIND_STATUS_CODE = 0x10,
  MESSAGE_TYPE_RECEIVER_CONFIG = 0x11,
  MESSAGE_TYPE_RECEIVER_CONFIG_STATUS_CODE = 0x12,
  MESSAGE_TYPE_TELEMETRY_RF_LINK_PACKET_RATE = 0x13,
  MESSAGE_TYPE_TELEMETRY_GENERAL = 0x14,
  MESSAGE_TYPE_TELEMETRY_GNSS = 0x15,
};

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
  getSerialData();
  doRfCommunication();
  sendSerialData();
}

//==================================================================================================

void getSerialData()
{
  uint8_t buffer[UART_FIXED_PACKET_SIZE];
  memset(buffer, 0, sizeof(buffer));

  if(Serial.available() < UART_FIXED_PACKET_SIZE) 
    return;

  uint8_t cntr = 0;
  while(Serial.available() > 0)
  {
    if(cntr < sizeof(buffer)) 
    {
      buffer[cntr] = Serial.read();
      cntr++;
    }
    else //Discard any extra data
      Serial.read();
  }

  //exit if busy, to prevent modifications to transmitPayloadBuffer
  if(hasPendingRCData || isRequestingBind || isRequestingOutputChConfig || isSendOutputChConfig)
    return;

  //check CRC and extract data
  uint8_t dataLength = buffer[4];
  uint8_t receivedCRC = buffer[5 + dataLength];
  uint8_t computedCRC = crc8(buffer, 5 + dataLength);
  if(computedCRC != receivedCRC)
    return;

  //get message type, extract the data
  uint8_t messageType = buffer[3];
  switch(messageType)
  {
    case MESSAGE_TYPE_RC_DATA:
      {
        hasPendingRCData = true;
        
        //read flags
        uint8_t flagsIdx = 5 + dataLength - 1;
        isRequestingTelemetry = ((buffer[flagsIdx] >> 3) & 0x01);
        rfPower = buffer[flagsIdx] & 0x07;

        //Skip this rc data if we were previously requesting for telemetry
        //to allow enough time to listen. 
        static bool wasRequestingTelemetry = false;
        if(wasRequestingTelemetry)
          hasPendingRCData = false;
        wasRequestingTelemetry = isRequestingTelemetry;

        //copy to transmitPayloadBuffer
        if(hasPendingRCData)
        {
          memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));
          if(dataLength > sizeof(transmitPayloadBuffer))
          {
            //buffer is not large enough, do not send partial data
            hasPendingRCData = false;
            break;
          }
          for(uint8_t i = 0; i < dataLength; i++)
          {
            transmitPayloadBuffer[i] = buffer[5 + i];
          }
          transmitPayloadLength = dataLength;
        }
      }
      break;

    case MESSAGE_TYPE_ENTER_BIND:
      {
        isRequestingBind = true;
        isMainReceiver = buffer[5] & 0x01;
      }
      break;
    
    case MESSAGE_TYPE_GET_RECEIVER_CONFIG:
      {
        isRequestingOutputChConfig = true;
        //copy to transmitPayloadBuffer
        memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));
        if(dataLength > sizeof(transmitPayloadBuffer))
        {
          //buffer is not large enough, do not send partial data
          isRequestingOutputChConfig = false;
          break;
        }
        for(uint8_t i = 0; i < dataLength; i++)
        {
          transmitPayloadBuffer[i] = buffer[5 + i];
        }
        transmitPayloadLength = dataLength;
      }
      break;

    case MESSAGE_TYPE_WRITE_RECEIVER_CONFIG:
      {
        isSendOutputChConfig = true;
        //copy to transmitPayloadBuffer
        memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));
        if(dataLength > sizeof(transmitPayloadBuffer))
        {
          //buffer is not large enough, do not send partial data
          isSendOutputChConfig = false;
          break;
        }
        for(uint8_t i = 0; i < dataLength; i++)
        {
          transmitPayloadBuffer[i] = buffer[5 + i];
        }
        transmitPayloadLength = dataLength;
      }
      break;
  }

}

//==================================================================================================

void sendSerialData()
{
  uint8_t buffer[UART_FIXED_PACKET_SIZE];
  memset(buffer, 0, sizeof(buffer));

  uint8_t messageType = MESSAGE_TYPE_NONE;

  static bool hasPendingRFLinkMessage = false;

  static uint32_t rfLinkTelemetryMessageDispatchTime;

  if(hasReceivedTelemetry)
  {
    hasReceivedTelemetry = false;
    if(receivedTelemetryType == TELEMETRY_TYPE_GNSS)
      messageType = MESSAGE_TYPE_TELEMETRY_GNSS;
    else if(receivedTelemetryType == TELEMETRY_TYPE_GENERAL)
    {
      messageType = MESSAGE_TYPE_TELEMETRY_GENERAL;
      hasPendingRFLinkMessage = true;
      rfLinkTelemetryMessageDispatchTime = millis() + 100;
    }
  }
  else if(hasPendingRFLinkMessage)
  {
    if(millis() > rfLinkTelemetryMessageDispatchTime)
    {
      hasPendingRFLinkMessage = false;
      messageType = MESSAGE_TYPE_TELEMETRY_RF_LINK_PACKET_RATE;
    }
  }

  if(gotOutputChConfig)
  {
    gotOutputChConfig = false;
    messageType = MESSAGE_TYPE_RECEIVER_CONFIG;
  }

  if(bindStatusCode != 0)
  {
    messageType = MESSAGE_TYPE_BIND_STATUS_CODE;
    buffer[5] = bindStatusCode;
    bindStatusCode = 0;
  }

  if(receiverConfigStatusCode != 0)
  {
    messageType = MESSAGE_TYPE_RECEIVER_CONFIG_STATUS_CODE;
    buffer[5] = receiverConfigStatusCode;
    receiverConfigStatusCode = 0;
  }

  switch(messageType)
  {
    case MESSAGE_TYPE_TELEMETRY_GNSS:
    case MESSAGE_TYPE_RECEIVER_CONFIG:
      {
        uint8_t dataLength = 0;
        for(uint8_t i = 0; i < receivePayloadLength; i++)
        {
          uint8_t buffIdx = 5 + i;
          if(buffIdx < sizeof(buffer) - 1)
          {
            buffer[buffIdx] = receivePayloadBuffer[i];
            dataLength++;
          }
          else
          {
            //buffer is not large enough, do not send partial data
            messageType = MESSAGE_TYPE_NONE;
            break;
          }
        }
        buffer[4] = dataLength;
      }
      break;
    
    case MESSAGE_TYPE_TELEMETRY_GENERAL:
      {
        uint8_t dataLength = 0;
        for(uint8_t i = 0; i < (receivePayloadLength - 1); i++)
        {
          uint8_t buffIdx = 5 + i;
          if(buffIdx < sizeof(buffer) - 1)
          {
            buffer[buffIdx] = receivePayloadBuffer[i + 1];
            dataLength++;
          }
          else
          {
            //buffer is not large enough, do not send partial data
            messageType = MESSAGE_TYPE_NONE;
            break;
          }
        }
        buffer[4] = dataLength;
      }
      break;

    case MESSAGE_TYPE_RECEIVER_CONFIG_STATUS_CODE:
      {
        buffer[4] = 1; //data length
        //data already handled
      }
      break;
    case MESSAGE_TYPE_BIND_STATUS_CODE: //data already handled
      {
        buffer[4] = 1; //data length
        //data already handled
      }
      break;

    case MESSAGE_TYPE_TELEMETRY_RF_LINK_PACKET_RATE:
      {
        buffer[4] = 2; //data length
        buffer[5] = transmitterPacketRate;
        buffer[6] = receiverPacketRate;
      }
      break;
  }

  if(messageType == MESSAGE_TYPE_NONE) //nothing to send, quit
    return;

  //--- Add other fields and send

  //Preamble
  buffer[0] = 0xAA;
  buffer[1] = 0xAA;
  buffer[2] = 0xAA;
  
  //Message type
  buffer[3] = messageType;
  
  //CRC
  uint8_t dataLength = buffer[4];
  buffer[5 + dataLength] = crc8(buffer, 5 + dataLength);

  //Padding bytes
  //Already set.

  //Send
  Serial.write(buffer, sizeof(buffer));
}

