
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

#if NUM_HOP_CHANNELS > FIXED_PAYLOAD_SIZE
  #error Number of hop channels cannot exceed size of payload
#endif 

#define FIXED_PACKET_SIZE  (FIXED_PAYLOAD_SIZE + 4)

uint8_t packet[FIXED_PACKET_SIZE];   //for messages to transmit
uint8_t msgBuff[FIXED_PACKET_SIZE]; //for received messages

enum {
  PAC_BIND,
  PAC_ACK_BIND,
  PAC_READ_OUTPUT_CH_CONFIG,
  PAC_SET_OUTPUT_CH_CONFIG,
  PAC_ACK_OUTPUT_CH_CONFIG,
  PAC_RC_DATA,
  PAC_TELEMETRY,
  PAC_INVALID
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
void buildPacket(uint8_t srcID, uint8_t destID, uint8_t dataIdentifier, uint8_t *dataBuff, uint8_t dataLen);
void readReceivedMessage();
uint8_t checkReceivedMessage(uint8_t srcID, uint8_t destID);

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
    //delay a bit (might not be necessary)
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
 
  uint8_t packetType = PAC_INVALID;
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
    readReceivedMessage();
    hop();
    packetType = checkReceivedMessage(Sys.transmitterID, Sys.receiverID);
  }

  switch(packetType)
  {
    case PAC_RC_DATA:
      {
        rcPacketCount++;
        lastRCPacketMillis = millis();

        digitalWrite(PIN_LED, HIGH);
        
        //Decode the rc data
        uint16_t chTemp[NUM_RC_CHANNELS];
        for(uint8_t chIdx = 0; chIdx < NUM_RC_CHANNELS; chIdx++)
        {
          uint8_t aIdx = 3 + chIdx + (chIdx / 4); //3 is the index in msgBuff where the data begins
          uint8_t bIdx = aIdx + 1;
          uint8_t aShift = ((chIdx % 4) + 1) * 2;
          uint8_t bShift = 8 - aShift;
          uint16_t aMask = ((uint16_t)0x0400) - ((uint16_t)1 << aShift);
          uint8_t  bMask = ((uint16_t)1 << (8-bShift)) - 1;
          chTemp[chIdx] = (((uint16_t)msgBuff[aIdx] << aShift) & aMask) | (((uint16_t)msgBuff[bIdx] >> bShift) & bMask);
        }
        
        uint16_t chData[MAX_CHANNELS_PER_RECEIVER];
        memset(chData, 0, sizeof(chData));
        for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
        {
          if(Sys.isMainReceiver) //copy the lower channels
            chData[i] = chTemp[i];
          else //Secondary receiver, copy the upper channels
            chData[i] = chTemp[MAX_CHANNELS_PER_RECEIVER + i];
        }      
        
        uint8_t flag = msgBuff[3 + FIXED_PAYLOAD_SIZE - 1];
        
        //Check if failsafe data. If so, don't modify outputs
        if((flag >> 4) & 0x01)
        {
          failsafeEverBeenReceived = true;
          for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
          {
            channelFailsafe[i] = chData[i]; 
            channelFailsafe[i] -= 500;
          }
        }
        else //normal channel values
        {
          for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
          {
            channelOut[i] = chData[i]; 
            channelOut[i] -= 500;
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
      
    case PAC_READ_OUTPUT_CH_CONFIG:
      {
        if((uint8_t)Sys.isMainReceiver != (msgBuff[3] & 0x01))
        {
          //This isn't the receiver we intended, bail out.
          hop();
          break;
        }
        
        //Reply with the configuration

        uint8_t dataToSend[FIXED_PAYLOAD_SIZE]; 
        memset(dataToSend, 0, sizeof(dataToSend));
        for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
        {
          if(Sys.isMainReceiver) 
            dataToSend[i] = Sys.outputChConfig[i];
          else 
            dataToSend[MAX_CHANNELS_PER_RECEIVER + i] = Sys.outputChConfig[i];
        }
        
        buildPacket(Sys.receiverID, Sys.transmitterID, PAC_READ_OUTPUT_CH_CONFIG, dataToSend, sizeof(dataToSend));
        delayMicroseconds(500);
        if(LoRa.beginPacket())
        {
          LoRa.write(packet, sizeof(packet));
          LoRa.endPacket(); //block until done transmitting
          hop();
        }
      }
      break;
    
    case PAC_SET_OUTPUT_CH_CONFIG:
      {
        if((uint8_t)Sys.isMainReceiver != (msgBuff[3 + NUM_RC_CHANNELS] & 0x01))
        {
          //This isn't the receiver we intended.
          hop();
          break;
        }
        
        //read and save config to eeprom
        for(uint8_t i = 0; i < MAX_CHANNELS_PER_RECEIVER; i++)
        {
          uint8_t val = Sys.isMainReceiver ? msgBuff[3 + i] : msgBuff[3 + i + MAX_CHANNELS_PER_RECEIVER];
          //write the servoPWMRangeIdx and signalType, leaving the maxSignalType unchanged
          Sys.outputChConfig[i] &= 0x0C;
          Sys.outputChConfig[i] |= val & 0xF3;
        }
        eeSaveSysConfig();
       
        //reply 
        buildPacket(Sys.receiverID, Sys.transmitterID, PAC_ACK_OUTPUT_CH_CONFIG, NULL, 0);
        delayMicroseconds(500);
        if(LoRa.beginPacket())
        {
          LoRa.write(packet, sizeof(packet));
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
      readReceivedMessage();
      uint8_t txId = msgBuff[0];
      if(txId != 0x00 && checkReceivedMessage(txId, 0x00) == PAC_BIND)
      {
        receivedBind = true;
        break;
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

  Sys.transmitterID = msgBuff[0];
  
  //get hop channels
  uint8_t idx = 3; //index in msgBuff
  for(uint8_t i = 0; i < NUM_HOP_CHANNELS; i++)
  {
    if(msgBuff[idx] < NUM_FREQ)
      Sys.fhss_schema[i] = msgBuff[idx];
    idx++;
  }
  
  //check if we are binding as main or secondary
  Sys.isMainReceiver = msgBuff[idx] & 0x01;
  idx++;
  if(!Sys.isMainReceiver)
    Sys.receiverID = msgBuff[idx];

  //--- Send reply
  
  uint8_t dataToSend[FIXED_PAYLOAD_SIZE];
  memset(dataToSend, 0, sizeof(dataToSend));
  if(Sys.isMainReceiver) //generate receiverID
  {
    randomSeed(micros());
    Sys.receiverID = random(0x01, 0xFF);
  }
  dataToSend[0] = Sys.receiverID;
  buildPacket(0x00, Sys.transmitterID, PAC_ACK_BIND, dataToSend, sizeof(dataToSend));
  delayMicroseconds(500);
  if(LoRa.beginPacket())
  {
    LoRa.write(packet, sizeof(packet));
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
    
    uint8_t dataToSend[FIXED_PAYLOAD_SIZE]; 
    memset(dataToSend, 0, sizeof(dataToSend));
    
    //packet rate
    dataToSend[0] = rcPacketsPerSecond;
    
    //external voltage
    uint16_t telem_volts = externalVolts / 10; //convert to 10mV scale
    if(externalVolts < 2000 || millis() < 5000UL)
      telem_volts = TELEMETRY_NO_DATA;
    dataToSend[1] = 0x01; //sensor ID
    dataToSend[2] = (telem_volts >> 8) & 0xFF; //high byte
    dataToSend[3] = telem_volts & 0xFF; //low byte
    
    //rssi
    dataToSend[4] = 0x7F; //sensor ID
    dataToSend[5] = (telem_rssi >> 8) & 0xFF; //high byte
    dataToSend[6] = telem_rssi & 0xFF; //low byte
    

    //--- Build packet and start transmit ---
    buildPacket(Sys.receiverID, Sys.transmitterID, PAC_TELEMETRY, dataToSend, sizeof(dataToSend));
    if(LoRa.beginPacket())
    {
      LoRa.write(packet, sizeof(packet));
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

void buildPacket(uint8_t srcID, uint8_t destID, uint8_t dataIdentifier, uint8_t *dataBuff, uint8_t dataLen)
{
  memset(packet, 0, sizeof(packet));
  packet[0] = srcID;
  packet[1] = destID;
  packet[2] = dataIdentifier;
  for(uint8_t i = 0; i < dataLen; i++)
  {
    uint8_t idx = 3 + i;
    if(idx < sizeof(packet) - 1)
    {
      packet[idx] = *dataBuff;
      ++dataBuff;
    }
  }
  packet[sizeof(packet) - 1] = crc8(packet, sizeof(packet) - 1);
}

//--------------------------------------------------------------------------------------------------

void readReceivedMessage()
{
  memset(msgBuff, 0, sizeof(msgBuff));
  uint8_t cntr = 0;
  while (LoRa.available() > 0) 
  {
    if(cntr < sizeof(msgBuff))
    {
      msgBuff[cntr] = LoRa.read();
      cntr++;
    }
    else // discard any extra data
      LoRa.read(); 
  }
}

//--------------------------------------------------------------------------------------------------

uint8_t checkReceivedMessage(uint8_t srcID, uint8_t destID)
{
  if(msgBuff[0] != srcID || msgBuff[1] != destID)
    return PAC_INVALID;
  
  //check crc
  uint8_t crcQQ = msgBuff[sizeof(msgBuff) - 1];
  uint8_t computedCRC = crc8(msgBuff, sizeof(msgBuff) - 1);
  if(crcQQ != computedCRC)
    return PAC_INVALID;
  
  return msgBuff[2];
}

