
#include "Arduino.h"
#include <SPI.h>

#include "LoRa.h"
#include "config.h"
#include "common.h"
#include "crc.h"
#include "eestore.h"
#include "rfComm.h"

//--------------- Freq allocation --------------------
/* 
  LPD433 Band ITU region 1
  The frequencies in this UHF band lie between 433.05Mhz and 434.790Mhz with 25kHz separation for a
  total of 69 freq channels. Channel_1 is 433.075 Mhz and Channel_69 is 434.775Mhz. 
  All our communications have to occur on any of these 69 channels. 
  
  915MHZ Band ITU region 2
  The frequencies in this UHF band lie between 902Mhz and 928Mhz
*/

#if defined (ISM_433MHZ)
  // separation here is 525kHz (500kHz total lora bw +  25kHz guard band.
  #define NUM_FREQ 4
  uint32_t freqList[NUM_FREQ] = {
    433150000, 433675000, 434200000, 434725000
  };
  
#elif defined (ISM_915MHZ)
  // separation here is 550kHz (500kHz total Lora bw + 50kHz guard band
  // frequencies here are in range 915MHz to 925MHz to avoid clashing with the GSM900 frequencies
  #define NUM_FREQ 15
  uint32_t freqList[NUM_FREQ] = {
    916100000, 916650000, 917200000, 917750000, 918300000,
    918850000, 919400000, 919950000, 920500000, 921050000,
    921600000, 922150000, 922700000, 923250000, 923800000
  };
#endif

#if NUM_HOP_CHANNELS > NUM_FREQ
  #error Number of hop channels cannot exceed number of available frequencies
#endif 

//------------------------------------------------

#if NUM_HOP_CHANNELS > (MAX_PAYLOAD_SIZE - 2)
  #error Number of hop channels exceeds allowable value
#endif 

uint8_t transmitPacketBuffer[MAX_PACKET_SIZE];
uint8_t receivePacketBuffer[MAX_PACKET_SIZE];
uint8_t transmitPacketLength;
uint8_t receivePacketLength;

enum {
  PACKET_BIND = 0,
  PACKET_ACK_BIND = 1,
  PACKET_READ_OUTPUT_CH_CONFIG = 2,
  PACKET_SET_OUTPUT_CH_CONFIG = 3,
  PACKET_ACK_OUTPUT_CH_CONFIG = 4,
  PACKET_RC_DATA = 5,
  PACKET_TELEMETRY_GENERAL = 6,
  PACKET_TELEMETRY_GNSS = 7,

  PACKET_INVALID = 0xFF
};

bool     radioInitialised = false;
uint32_t totalPacketsSent = 0;

bool isListeningForTelemetry = false;

//function declarations
void setRfPower(uint8_t dBm);
void hop();
void bind();
void transmitRCdata();
void transmitReceiverConfig();
void getReceiverConfig();
void getTelemetry();
void buildPacket(uint8_t sourceID, uint8_t destinationID, uint8_t dataIdentifier, uint8_t *dataBuffer, uint8_t dataLength);
void readReceivedPacket();
uint8_t checkReceivedPacket(uint8_t sourceID, uint8_t destinationID);

//==================================================================================================

void initialiseRfModule()
{
  //setup lora module
  LoRa.setPins(PIN_LORA_SS, PIN_LORA_RESET); 
  if(LoRa.begin(freqList[0]))
  {
    LoRa.setSpreadingFactor(7); 
    LoRa.setCodingRate4(5);
    LoRa.setSignalBandwidth(500E3);
    delay(20);
    //start in low power level
    LoRa.sleep();
    LoRa.setTxPower(3); //3dBm
    LoRa.idle();
    
    radioInitialised = true;
  }
  else
    radioInitialised = false;
}

//==================================================================================================

void doRfCommunication()
{
  if(!radioInitialised)
    return;

  if(isRequestingBind)
  {
    bind();
  }
  else if(isRequestingOutputChConfig)
  {
    getReceiverConfig();
  }
  else if(isSendOutputChConfig)
  {
    transmitReceiverConfig();
  }
  else if(hasPendingRCData)
  {
    if(isListeningForTelemetry)
    {
      //In this case, we forcibly kick out of 'listening for telemetry' mode
      isListeningForTelemetry = false;
      isRequestingTelemetry = false;
      hasPendingRFLinkMessage = true;
      hop();
    }

    //transmit
    transmitRCdata();

    //calculate transmitted packets per second
    static uint32_t lastTotalPacketsSent = 0;
    static uint32_t prevMillis = 0;
    uint32_t ttElapsed = millis() - prevMillis;
    if(ttElapsed >= 1000)
    {
      prevMillis = millis();
      uint32_t pps = (totalPacketsSent - lastTotalPacketsSent) * 1000;
      pps /= ttElapsed;
      transmitterPacketRate = pps & 0xFF;
      lastTotalPacketsSent = totalPacketsSent;
    }
  }
  else if((isRequestingTelemetry && !hasPendingRCData) || isListeningForTelemetry)
  {
    getTelemetry();
  }

  //set RF power level
  if(!LoRa.isTransmitting())
  {
    static uint8_t power_dBm[3] = {3, 10, 17}; //2mW, 10mW, 50mW
    setRfPower(power_dBm[rfPower]);
  }

}

//============================== Helpers ===========================================================

void stopRfModule()
{
  //stop LoRa module
  if(radioInitialised)
  {
    while(LoRa.isTransmitting())
    {    
      delay(2);
    }
    LoRa.sleep();
    delay(10);
  }
}

//--------------------------------------------------------------------------------------------------

void setRfPower(uint8_t dBm)
{
  static uint8_t prev_dBm = 0xff;
  if(dBm != prev_dBm)
  {
    prev_dBm = dBm;
    LoRa.sleep();
    LoRa.setTxPower(dBm);
    LoRa.idle();
  }
}

//--------------------------------------------------------------------------------------------------

void hop()
{
  static uint8_t idx_fhss_schema = 0; 
  idx_fhss_schema++;
  if(idx_fhss_schema >= NUM_HOP_CHANNELS)
    idx_fhss_schema = 0;
  
  uint8_t idx_freq = Sys.fhss_schema[idx_fhss_schema];
  if(idx_freq < NUM_FREQ)
  {
    LoRa.sleep();
    LoRa.setFrequency(freqList[idx_freq]);
    LoRa.idle();
  }
}

//--------------------------------------------------------------------------------------------------

void bind()
{
  static bool bindInitialised = false;
  static bool transmitInitiated = false;

  static uint32_t bindModeEntryTime = 0;
  static uint32_t bindAckEntryTime = 0;
  
  static bool isListeningForAck = false;
  
  #define TIMEOUT_MODE_BIND  4500 
  #define TIMEOUT_BIND_ACK   100  //Max time waiting for receiver's ack before retransmission
  
  /// INITIALISE
  if(!bindInitialised)
  {
    bindModeEntryTime = millis();
    //save first
    eeSaveSysConfig();
    
    if(isMainReceiver)
    {
     //--- generate random Sys.transmitterID and Sys.fhss_schema
      randomSeed(millis()); //Seed PRNG
      Sys.transmitterID = random(0x01, 0x7F); //generate random ID
      memset(Sys.fhss_schema, 0xFF, sizeof(Sys.fhss_schema)); //clear schema
      //generate random unique hopping sequence
      uint8_t idx = 0;
      while (idx < NUM_HOP_CHANNELS)
      {
        uint8_t genVal = random(NUM_FREQ);
        bool unique = true;
        for(uint8_t k = 0; k < (NUM_HOP_CHANNELS); k++)
        {
          if(genVal == Sys.fhss_schema[k]) //not unique
          {
            unique = false;
            break; //exit for loop
          }
        }
        if(unique)
        {
          Sys.fhss_schema[idx] = genVal; //add to schema
          idx++; //increment index
        }
      }
    }
    
    //--- set to lowest power level
    setRfPower(2); // 2dBm

    //--- set to bind frequency
    LoRa.sleep();
    LoRa.setFrequency(freqList[0]);
    LoRa.idle();
    
    bindInitialised = true;
  }
  
  ///START TRANSMIT
  if(bindInitialised && !isListeningForAck && !transmitInitiated)
  {
    memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));
    uint8_t i;
    for(i = 0; i < NUM_HOP_CHANNELS; i++)
    {
      transmitPayloadBuffer[i] = Sys.fhss_schema[i];
    }
    transmitPayloadBuffer[i++] = isMainReceiver & 0x01;
    transmitPayloadBuffer[i++] = Sys.receiverID;
    transmitPayloadLength = i;

    buildPacket(Sys.transmitterID, 0x00, PACKET_BIND, transmitPayloadBuffer, transmitPayloadLength);
    if(LoRa.beginPacket())
    {
      LoRa.write(transmitPacketBuffer, transmitPacketLength);
      LoRa.endPacket(true); //non-blocking
      delayMicroseconds(500);
      transmitInitiated = true;
    }
  }
  
  /// ON DONE TRANSMIT, LISTEN FOR REPLY OR TIMEOUT 
  if(!LoRa.isTransmitting())
  {
    if(!isListeningForAck)
    {
      isListeningForAck = true;
      bindAckEntryTime = millis();
    }
    
    if(LoRa.parsePacket()) //received a packet
    {
      readReceivedPacket();
      if(checkReceivedPacket(0x00, Sys.transmitterID) == PACKET_ACK_BIND) 
      {
        bindStatusCode = 1; //bind success
        Sys.receiverID = receivePayloadBuffer[0];
        //Save to eeprom
        eeSaveSysConfig();
        //clear flags
        bindInitialised = false;
        isListeningForAck = false;
        transmitInitiated = false;
        //exit
        isRequestingBind = false;
        return;
      }
    }
    
    if(isListeningForAck && millis() - bindAckEntryTime > TIMEOUT_BIND_ACK) //timeout of ack
    {
      transmitInitiated = false;
      isListeningForAck = false;
    }
    
    if(millis() - bindModeEntryTime > TIMEOUT_MODE_BIND) //timeout of bind
    {
      bindStatusCode = 2; //bind failed
      //restore so that we don't unintentionally unbind a bound receiver
      eeReadSysConfig();
      //clear flags
      bindInitialised = false;
      isListeningForAck = false;
      transmitInitiated = false;
      //exit
      isRequestingBind = false;
      return;
    }
  }
}

//--------------------------------------------------------------------------------------------------

void transmitRCdata()
{
  static bool transmitInitiated = false;
  static bool hopPending = false;

  //START TRANSMIT
  if(!transmitInitiated) 
  {
    buildPacket(Sys.transmitterID, Sys.receiverID, PACKET_RC_DATA, transmitPayloadBuffer, transmitPayloadLength);
    if(LoRa.beginPacket())
    {
      LoRa.write(transmitPacketBuffer, transmitPacketLength);
      LoRa.endPacket(true); //async
      delayMicroseconds(500);
      transmitInitiated = true;
      hopPending = true;
      totalPacketsSent++;
    }
  }
  
  /// ON TRANSMIT DONE
  if(!LoRa.isTransmitting())
  {
    hasPendingRCData = false;
    transmitInitiated = false;
    if(hopPending)
    {
      hop();
      hopPending = false;
    }
  }
}

//--------------------------------------------------------------------------------------------------

void getTelemetry()
{
  isListeningForTelemetry = true;

  if(LoRa.parsePacket()) //received a packet
  {
    readReceivedPacket();
    hop();
    isListeningForTelemetry = false;
    isRequestingTelemetry = false;

    if(checkReceivedPacket(Sys.receiverID, Sys.transmitterID) == PACKET_TELEMETRY_GENERAL)
    {
      hasReceivedTelemetry = true;
      receivedTelemetryType = TELEMETRY_TYPE_GENERAL;
      receiverPacketRate = receivePayloadBuffer[0];
      generalTelemetryLastReceiveTime = millis();
    }
    else if(checkReceivedPacket(Sys.receiverID, Sys.transmitterID) == PACKET_TELEMETRY_GNSS)
    {
      hasReceivedTelemetry = true;
      receivedTelemetryType = TELEMETRY_TYPE_GNSS;
    }
  }
}

//--------------------------------------------------------------------------------------------------

void getReceiverConfig()
{
  static bool transmitInitiated = false;
  static bool isListeningForReply = false;
  
  static uint32_t listenEntryTime = 0;
  const int16_t   maxListenTime = 40;
  
  static int16_t retryCount = 0;
  const int16_t  maxRetries  = 5 * sizeof(Sys.fhss_schema) / sizeof(Sys.fhss_schema[0]);

  //Start transmit
  if(!transmitInitiated)
  {
    buildPacket(Sys.transmitterID, Sys.receiverID, PACKET_READ_OUTPUT_CH_CONFIG, transmitPayloadBuffer, transmitPayloadLength);
    if(LoRa.beginPacket())
    {
      LoRa.write(transmitPacketBuffer, transmitPacketLength);
      LoRa.endPacket(true); //async
      delayMicroseconds(500);
      transmitInitiated = true;
    }
  }
  
  //On transmit done, listen for reply
  if(!LoRa.isTransmitting())
  {
    if(!isListeningForReply)
    {
      hop();
      isListeningForReply = true;
      listenEntryTime = millis();
    }
    
    if(LoRa.parsePacket()) //received a packet
    {
      readReceivedPacket();
      hop();
      if(checkReceivedPacket(Sys.receiverID, Sys.transmitterID) == PACKET_READ_OUTPUT_CH_CONFIG)
      {
        gotOutputChConfig = true;
        isRequestingOutputChConfig = false;
        isListeningForReply = false;
        transmitInitiated = false;
        retryCount = 0;
        //exit
        return;
      }
    }
    
    //Retry if nothing was received
    if(isListeningForReply && (millis() - listenEntryTime) > maxListenTime)
    {
      isListeningForReply = false;
      transmitInitiated = false;
      ++retryCount;
      if(retryCount > maxRetries) //timeout
      {
        retryCount = 0;
        isRequestingOutputChConfig = false;
        hop();
      }
    }
  }
}

//--------------------------------------------------------------------------------------------------

void transmitReceiverConfig()
{
  static bool transmitInitiated = false;
  static bool isListeningForReply = false;
  
  static uint32_t listenEntryTime = 0;
  const int16_t maxListenTime = 40;
  
  static int16_t retryCount = 0;
  const int16_t maxRetries  = 5 * sizeof(Sys.fhss_schema) / sizeof(Sys.fhss_schema[0]);

  //Start transmit
  if(!transmitInitiated)
  {
    buildPacket(Sys.transmitterID, Sys.receiverID, PACKET_SET_OUTPUT_CH_CONFIG, transmitPayloadBuffer, transmitPayloadLength);
    if(LoRa.beginPacket())
    {
      LoRa.write(transmitPacketBuffer, transmitPacketLength);
      LoRa.endPacket(true); //async
      delayMicroseconds(500);
      transmitInitiated = true;
    }
  }
  
  //On transmit done, listen for reply
  if(!LoRa.isTransmitting())
  {
    if(!isListeningForReply)
    {
      hop();
      isListeningForReply = true;
      listenEntryTime = millis();
    }
    
    if(LoRa.parsePacket()) //received a packet
    {
      readReceivedPacket();
      hop();
      if(checkReceivedPacket(Sys.receiverID, Sys.transmitterID) == PACKET_ACK_OUTPUT_CH_CONFIG)
      {
        receiverConfigStatusCode = 1;
        isSendOutputChConfig = false;
        isListeningForReply = false;
        transmitInitiated = false;
        retryCount = 0;
        //exit
        return;
      }
    }
    
    //Retry if nothing was received
    if(isListeningForReply && (millis() - listenEntryTime) > maxListenTime)
    {
      isListeningForReply = false;
      transmitInitiated = false;
      ++retryCount;
      if(retryCount > maxRetries) //timeout
      {
        retryCount = 0;
        receiverConfigStatusCode = 2;
        isSendOutputChConfig = false;
        hop();
      }
    }
  }
}

//--------------------------------------------------------------------------------------------------

void buildPacket(uint8_t sourceID, uint8_t destinationID, uint8_t dataIdentifier, uint8_t *dataBuffer, uint8_t dataLength)
{
  memset(transmitPacketBuffer, 0, sizeof(transmitPacketBuffer));
  
  //sourceID 7 bits, destinationID 7 bits, dataIdentifier 5 bits, dataLength 5 bits
  transmitPacketBuffer[0] = sourceID << 1;
  transmitPacketBuffer[0] |= (destinationID >> 6) & 0x01;
  transmitPacketBuffer[1] = destinationID << 2;
  transmitPacketBuffer[1] |= (dataIdentifier >> 3) & 0x03;
  transmitPacketBuffer[2] = dataIdentifier << 5;
  transmitPacketBuffer[2] |= (dataLength & 0x1F);
  
  //payload
  uint8_t payloadLength = 0;
  for(uint8_t i = 0; i < dataLength; i++)
  {
    uint8_t idx = 3 + i;
    if(idx < (sizeof(transmitPacketBuffer) - 1))
    {
      transmitPacketBuffer[idx] = *dataBuffer;
      dataBuffer++;
      payloadLength++;
    }
  }
  
  //add a crc
  transmitPacketBuffer[3 + payloadLength] = crc8(transmitPacketBuffer, 3 + payloadLength);
 
  //calculate the packet length
  transmitPacketLength = 4 + payloadLength;
}

//--------------------------------------------------------------------------------------------------

void readReceivedPacket()
{
  memset(receivePacketBuffer, 0, sizeof(receivePacketBuffer));
  uint8_t cntr = 0;
  while (LoRa.available() > 0) 
  {
    if(cntr < sizeof(receivePacketBuffer))
    {
      receivePacketBuffer[cntr] = LoRa.read();
      cntr++;
    }
    else // discard any extra data
      LoRa.read(); 
  }
}

//--------------------------------------------------------------------------------------------------

uint8_t checkReceivedPacket(uint8_t sourceID, uint8_t destinationID)
{
  uint8_t _sourceID = (receivePacketBuffer[0] >> 1) & 0x7F;
  uint8_t _destinationID = ((receivePacketBuffer[0] & 0x01) << 6) | ((receivePacketBuffer[1] >> 2) & 0x3F);
  uint8_t _dataIdentifier = ((receivePacketBuffer[1] & 0x03) << 3) | ((receivePacketBuffer[2] >> 5) & 0x07);
  uint8_t _dataLength = receivePacketBuffer[2] & 0x1F;

  if(_sourceID != sourceID || _destinationID != destinationID)
    return PACKET_INVALID;

  if(_dataLength > MAX_PAYLOAD_SIZE)
    return PACKET_INVALID;

  //check crc
  uint8_t receivedCRC = receivePacketBuffer[3 + _dataLength];
  uint8_t computedCRC = crc8(receivePacketBuffer, 3 + _dataLength);
  if(computedCRC != receivedCRC)
    return PACKET_INVALID;

  receivePayloadLength = _dataLength;
  memset(receivePayloadBuffer, 0, sizeof(receivePayloadBuffer));
  for(uint8_t i = 0; i < receivePayloadLength; i++)
  {
    receivePayloadBuffer[i] = receivePacketBuffer[3 + i];
  }

  return _dataIdentifier;
}
