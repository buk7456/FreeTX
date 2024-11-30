
#include "Arduino.h"
#include <SPI.h>

#include "LoRa.h"
#include "config.h"
#include "common.h"
#include "crc.h"
#include "eestore.h"
#include "rfComm.h"

#define MAX_PAYLOAD_SIZE 26
#define MAX_PACKET_SIZE  (4 + MAX_PAYLOAD_SIZE)

uint8_t transmitPayloadBuffer[MAX_PAYLOAD_SIZE];
uint8_t receivePayloadBuffer[MAX_PAYLOAD_SIZE];
uint8_t transmitPayloadLength;
uint8_t receivePayloadLength;

uint8_t transmitPacketBuffer[MAX_PACKET_SIZE];
uint8_t receivePacketBuffer[MAX_PACKET_SIZE];
uint8_t transmitPacketLength;
uint8_t receivePacketLength;

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

bool radioInitialised = false;
uint32_t rcPacketCount = 0;

bool isSendingTelemetry = false;

int16_t telem_rssi;

#define MAX_LISTEN_TIME_ON_HOP_CHANNEL 100 //in ms

//function declarations
void setRfPower(uint8_t dBm);
void bind();
void hop();
void sendTelemetry();
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
  
  //--- BIND
  if(isRequestingBind)
  {
    bind(); //blocking
    isRequestingBind = false;
    return;
  }
  
  //--- NON BLOCKING TELEMETRY TRANSMISSION
  if(isSendingTelemetry)
  {
    //continue transmitting the telemetry
    sendTelemetry();
    return;
  }
    
  //--- READ INCOMING PACKET (NONBIND PACKETS)
 
  uint8_t packetType = PACKET_INVALID;
  static uint32_t timeOfLastPacket = millis();
  
  if(millis() - timeOfLastPacket > MAX_LISTEN_TIME_ON_HOP_CHANNEL)
  {
    timeOfLastPacket = millis();
    hop();
  }

  if(LoRa.parsePacket()) //received a packet
  {
    timeOfLastPacket = millis();
    telem_rssi = LoRa.packetRssi();
    readReceivedPacket();
    hop();
    packetType = checkReceivedPacket(Sys.transmitterID, Sys.receiverID);
  }

  switch(packetType)
  {
    case PACKET_RC_DATA:
      {
        uint8_t numReceivedChannels = ((receivePayloadLength - 1) * 8) / 10; // subtract 1 for flags byte

        if(!Sys.isMainReceiver && numReceivedChannels <= MAX_CHANNELS_PER_RECEIVER)
        {
          //no data for secondary receiver has been received, quit
          break;
        }

        rcPacketCount++;
        lastRCPacketMillis = millis();
        digitalWrite(PIN_LED, HIGH);

        //Decode the rc data
        uint16_t chTemp[NUM_RC_CHANNELS];
        memset(chTemp, 0, sizeof(chTemp));
        for(uint8_t chIdx = 0; chIdx < NUM_RC_CHANNELS && chIdx < numReceivedChannels; chIdx++)
        {
          uint8_t aIdx = chIdx + (chIdx / 4);
          uint8_t bIdx = aIdx + 1;
          uint8_t aShift = ((chIdx % 4) + 1) * 2;
          uint8_t bShift = 8 - aShift;
          uint16_t aMask = ((uint16_t)0x0400) - ((uint16_t)1 << aShift);
          uint8_t  bMask = ((uint16_t)1 << (8 - bShift)) - 1;
          chTemp[chIdx] = (((uint16_t)receivePayloadBuffer[aIdx] << aShift) & aMask) | (((uint16_t)receivePayloadBuffer[bIdx] >> bShift) & bMask);
        }

        uint8_t flag = receivePayloadBuffer[receivePayloadLength - 1];
        bool isFailsafeData = (flag >> 4) & 0x01;

        uint8_t startIdx = 0;
        uint8_t endIdx = MAX_CHANNELS_PER_RECEIVER - 1;
        if(!Sys.isMainReceiver)
        {
          startIdx = MAX_CHANNELS_PER_RECEIVER;
          endIdx = numReceivedChannels - 1;
          if((endIdx - startIdx) > (MAX_CHANNELS_PER_RECEIVER - 1))
            endIdx = MAX_CHANNELS_PER_RECEIVER * 2 - 1;
        }

        for(uint8_t i = startIdx; i <= endIdx; i++)
        {
          if(isFailsafeData)
          {
            failsafeEverBeenReceived[i - startIdx] = true;
            channelFailsafe[i - startIdx] = chTemp[i]; 
            channelFailsafe[i - startIdx] -= 500;
          }
          else
          {
            channelOut[i - startIdx] = chTemp[i]; 
            channelOut[i - startIdx] -= 500;
          }
        }
        
        //set rf power level
        static uint8_t power_dBm[3] = {3, 10, 17}; //2mW, 10mW, 50mW
        setRfPower(power_dBm[flag & 0x07]);
        
        //telemetry request
        bool isRequestingTelemetry = (flag >> 3) & 0x01;
        if(isRequestingTelemetry)
          sendTelemetry();
      }
      break;
      
    case PACKET_READ_OUTPUT_CH_CONFIG:
      {
        if((uint8_t)Sys.isMainReceiver != (receivePayloadBuffer[0] & 0x01))
        {
          //This isn't the receiver we intended, bail out.
          hop();
          break;
        }
        
        //Reply with the configuration
        memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));
        uint8_t idx;
        for(idx = 0; idx < MAX_CHANNELS_PER_RECEIVER; idx++)
        {
          transmitPayloadBuffer[idx] = Sys.outputChConfig[idx];
        }
        transmitPayloadLength = idx;
        buildPacket(Sys.receiverID, Sys.transmitterID, PACKET_READ_OUTPUT_CH_CONFIG, transmitPayloadBuffer, transmitPayloadLength);
        delayMicroseconds(500);
        if(LoRa.beginPacket())
        {
          LoRa.write(transmitPacketBuffer, transmitPacketLength);
          LoRa.endPacket(); //block until done transmitting
          hop();
        }
      }
      break;
    
    case PACKET_SET_OUTPUT_CH_CONFIG:
      {
        if((uint8_t)Sys.isMainReceiver != (receivePayloadBuffer[receivePayloadLength - 1] & 0x01))
        {
          //This isn't the receiver we intended.
          hop();
          break;
        }
        
        //read and save config to eeprom
        for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
        {
          uint8_t val = receivePayloadBuffer[i];
          //write the servoPWMRangeIdx and signalType, leaving the maxSignalType unchanged
          Sys.outputChConfig[i] &= 0x0C;
          Sys.outputChConfig[i] |= val & 0xF3;
        }
        eeSaveSysConfig();
       
        //reply with acknowledgement
        memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));
        buildPacket(Sys.receiverID, Sys.transmitterID, PACKET_ACK_OUTPUT_CH_CONFIG, transmitPayloadBuffer, 0);
        delayMicroseconds(500);
        if(LoRa.beginPacket())
        {
          LoRa.write(transmitPacketBuffer, transmitPacketLength);
          LoRa.endPacket(); //block until done transmitting
          hop();
        }
      }
      break;
  }

  //--- TURN OFF LED TO INDICATE NO INCOMING RC DATA
  if(millis() - lastRCPacketMillis > 100)
    digitalWrite(PIN_LED, LOW);
}

//================================= HELPERS ========================================================

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
  //--- Set to lowest power level
  setRfPower(2); // 2dBm
  
  //--- Set to bind frequency
  LoRa.sleep();
  LoRa.setFrequency(freqList[0]);
  LoRa.idle();
  
  //--- Listen for bind
  
  bool receivedBind = false;
  const uint16_t BIND_LISTEN_TIMEOUT = 500;
  uint32_t endTime = millis() + BIND_LISTEN_TIMEOUT;
  while(millis() < endTime)
  {
    if(LoRa.parsePacket()) //received a packet
    {
      readReceivedPacket();
      uint8_t txId = (receivePacketBuffer[0] >> 1) & 0x7F;;
      if(txId != 0x00 && checkReceivedPacket(txId, 0x00) == PACKET_BIND)
      {
        if(receivePayloadLength == (NUM_HOP_CHANNELS + 2)) // +2 bytes for flag and receiverID
        {
          receivedBind = true;
          break;
        }
      }
    }
    delay(10);
  }
  
  if(!receivedBind) //hop and exit
  {
    hop();  
    return;
  }
  
  //--- Extract data

  Sys.transmitterID = (receivePacketBuffer[0] >> 1) & 0x7F;
  
  //get hop channels
  uint8_t idx = 0;
  for(uint8_t i = 0; i < NUM_HOP_CHANNELS; i++)
  {
    if(receivePayloadBuffer[idx] < NUM_FREQ)
      Sys.fhss_schema[i] = receivePayloadBuffer[idx];
    idx++;
  }
  
  //check if we are binding as main or secondary
  Sys.isMainReceiver = receivePayloadBuffer[idx] & 0x01;
  idx++;
  if(!Sys.isMainReceiver)
    Sys.receiverID = receivePayloadBuffer[idx];

  //--- Send reply

  memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));
  if(Sys.isMainReceiver) //generate receiverID
  {
    randomSeed(micros());
    Sys.receiverID = random(0x01, 0x7F);
  }
  transmitPayloadBuffer[0] = Sys.receiverID;
  transmitPayloadLength = 1;

  buildPacket(0x00, Sys.transmitterID, PACKET_ACK_BIND, transmitPayloadBuffer, transmitPayloadLength);
  delayMicroseconds(500);
  if(LoRa.beginPacket())
  {
    LoRa.write(transmitPacketBuffer, transmitPacketLength);
    LoRa.endPacket(); //block until done transmitting
    hop();
  }
  
  //--- Save to eeprom
  eeSaveSysConfig();
}

//--------------------------------------------------------------------------------------------------

void sendTelemetry() //### TODO implement custom telemetry, establish standard IDs
{
  if(!Sys.isMainReceiver)
  {
    hop();
    isSendingTelemetry = false;
    return;
  }
  
  static bool transmitInitiated = false;
  
  if(!transmitInitiated) 
  {
    //--- Calculate packets per second
    static uint32_t prevRCPacketCount = 0; 
    static uint32_t ttPrevMillis = 0;
    static uint8_t rcPacketsPerSecond = 0; 
    uint32_t ttElapsed = millis() - ttPrevMillis;
    if (ttElapsed >= 1000)
    {
      ttPrevMillis = millis();
      rcPacketsPerSecond = ((rcPacketCount - prevRCPacketCount) * 1000) / ttElapsed;
      prevRCPacketCount = rcPacketCount;
    }
    if(millis() - lastRCPacketMillis >= 1000)
      rcPacketsPerSecond = 0;
    
    //--- Prepare telemetry data and send it

    memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));

    //alternate between telemetry types
    static uint8_t counter = 0;
    counter++;
    if(counter % 2 == 0 && hasGNSSReceiver && sizeof(GNSSTelemetryData) <= sizeof(transmitPayloadBuffer))
      telemetryType = TELEMETRY_TYPE_GNSS;
    else
      telemetryType = TELEMETRY_TYPE_GENERAL;

    switch(telemetryType)
    {
      case TELEMETRY_TYPE_GENERAL: //todo improve this section to avoid possible buffer overflows
        {
          uint8_t idx = 0;

          //packet rate
          transmitPayloadBuffer[idx++] = rcPacketsPerSecond;
          
          //external voltage
          uint16_t telem_volts = externalVolts / 10; //convert to 10mV scale
          if(externalVolts < 2000 || millis() < 5000UL)
            telem_volts = TELEMETRY_NO_DATA;
          transmitPayloadBuffer[idx++] = 0x01; //sensor ID
          transmitPayloadBuffer[idx++] = (telem_volts >> 8) & 0xFF; //high byte
          transmitPayloadBuffer[idx++] = telem_volts & 0xFF; //low byte
          
          //rssi
          transmitPayloadBuffer[idx++] = 0x7F; //sensor ID
          transmitPayloadBuffer[idx++] = (telem_rssi >> 8) & 0xFF; //high byte
          transmitPayloadBuffer[idx++] = telem_rssi & 0xFF; //low byte

          transmitPayloadLength = idx;

          buildPacket(Sys.receiverID, Sys.transmitterID, PACKET_TELEMETRY_GENERAL, transmitPayloadBuffer, transmitPayloadLength);
        }
        break;
      
      case TELEMETRY_TYPE_GNSS: //todo
        {
          memset(transmitPayloadBuffer, 0, sizeof(transmitPayloadBuffer));
          memcpy(transmitPayloadBuffer, &GNSSTelemetryData, sizeof(GNSSTelemetryData));
          transmitPayloadLength = sizeof(GNSSTelemetryData);
          buildPacket(Sys.receiverID, Sys.transmitterID, PACKET_TELEMETRY_GNSS, transmitPayloadBuffer, transmitPayloadLength);
        }
        break;
    }

    //start transmit
    if(LoRa.beginPacket())
    {
      LoRa.write(transmitPacketBuffer, transmitPacketLength);
      LoRa.endPacket(true); //async
      delayMicroseconds(500);
      transmitInitiated = true;
      isSendingTelemetry = true;
    }
  }
  
  // ON TRANSMIT COMPLETE
  if(!LoRa.isTransmitting())
  {
    hop();
    transmitInitiated = false;
    isSendingTelemetry = false;
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
