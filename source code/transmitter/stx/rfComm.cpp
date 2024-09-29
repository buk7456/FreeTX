
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
uint8_t msgBuff[FIXED_PACKET_SIZE];  //for received messages

enum{
  PAC_BIND,
  PAC_ACK_BIND,
  PAC_READ_OUTPUT_CH_CONFIG,
  PAC_SET_OUTPUT_CH_CONFIG,
  PAC_ACK_OUTPUT_CH_CONFIG,
  PAC_RC_DATA,
  PAC_TELEMETRY,
  PAC_INVALID
};

enum {
  MODE_BIND, 
  MODE_RC_DATA, 
  MODE_GET_TELEM,
  MODE_GET_RECEIVER_CONFIG,
  MODE_SEND_RECEIVER_CONFIG
};

uint8_t  operatingMode = MODE_RC_DATA;
bool     radioInitialised = false;
uint32_t totalPacketsSent = 0;

//function declarations
void setRfPower(uint8_t dBm);
void hop();
void bind();
void transmitRCdata();
void transmitReceiverConfig();
void getReceiverConfig();
void getTelemetry();
void resetTelemetry();
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
  {
    return;
  }
  
  if(!LoRa.isTransmitting())
  {
    //--- Change modes ---
    if(isRequestingBind)
    {
      operatingMode = MODE_BIND;
      isRequestingBind = false;
    }
    else if(isRequestingOutputChConfig && operatingMode != MODE_BIND)
    {
      operatingMode = MODE_GET_RECEIVER_CONFIG;
      isRequestingOutputChConfig = false;
    }
    else if(isSendOutputChConfig && operatingMode != MODE_BIND)
    {
      operatingMode = MODE_SEND_RECEIVER_CONFIG;
      isSendOutputChConfig = false;
    }
    else if(hasPendingRCData && operatingMode == MODE_GET_TELEM)
    {
      hop(); //###
      operatingMode = MODE_RC_DATA;
    }
    
    //--- set power level ---
    if(operatingMode != MODE_BIND)
    {
      static uint8_t power_dBm[3] = {3, 10, 17}; //2mW, 10mW, 50mW
      setRfPower(power_dBm[rfPower]);
    }
  }

  //state machine
  switch(operatingMode)
  {
    case MODE_BIND:
      bind();
      break;
      
    case MODE_GET_RECEIVER_CONFIG:
      getReceiverConfig();
      break;
      
    case MODE_SEND_RECEIVER_CONFIG:
      transmitReceiverConfig();
      break;
      
    case MODE_RC_DATA:
      transmitRCdata();
      break;
      
    case MODE_GET_TELEM:
      getTelemetry();
      break;
  }
  
  //calculate transmitted packets per second
  static uint32_t lastTotalPacketsSent = 0;
  static uint32_t pktsPrevCalcMillis = 0;
  uint32_t ttElapsed = millis() - pktsPrevCalcMillis;
  if(ttElapsed >= 1000)
  {
    pktsPrevCalcMillis = millis();
    uint32_t pps = (totalPacketsSent - lastTotalPacketsSent) * 1000;
    pps /= ttElapsed;
    transmitterPacketRate = pps & 0xFF;
    lastTotalPacketsSent = totalPacketsSent;
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
  
  #define TIMEOUT_MODE_BIND  5000 
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
      Sys.transmitterID = random(0x01, 0xFF); //generate random ID
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
    uint8_t dataToSend[FIXED_PAYLOAD_SIZE];
    memset(dataToSend, 0, sizeof(dataToSend));
    uint8_t i;
    for(i = 0; i < NUM_HOP_CHANNELS; i++)
    {
      if(i >= FIXED_PAYLOAD_SIZE - 2)
        break;
      dataToSend[i] = Sys.fhss_schema[i];
    }
    dataToSend[i++] = isMainReceiver & 0x01;
    dataToSend[i++] = Sys.receiverID;

    buildPacket(Sys.transmitterID, 0x00, PAC_BIND, dataToSend, sizeof(dataToSend));
    if(LoRa.beginPacket())
    {
      LoRa.write(packet, sizeof(packet));
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
      readReceivedMessage();
      if(checkReceivedMessage(0x00, Sys.transmitterID) == PAC_ACK_BIND && msgBuff[3] > 0x00) //also checks receiverID
      {
        bindStatusCode = 1; //bind success
        Sys.receiverID = msgBuff[3];
        //Save to eeprom
        eeSaveSysConfig();
        //clear flags
        bindInitialised = false;
        isListeningForAck = false;
        transmitInitiated = false;
        //change mode and exit
        operatingMode = MODE_RC_DATA;
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
      //change mode and exit
      operatingMode = MODE_RC_DATA;
      return;
    }
  }
}

//--------------------------------------------------------------------------------------------------

void transmitRCdata()
{
  if(!hasPendingRCData) //nothing to do
    return;
  
  static bool transmitInitiated = false;
  static bool hopPending = false;
  
  if(!rfEnabled) //don't send
  {
    transmitInitiated = false;
    hopPending = false;
    hasPendingRCData = false;
    
    resetTelemetry();
    
    return;
  }
  
  //START TRANSMIT
  if(!transmitInitiated) 
  {
    //--- Encode data into a bit stream ---

    uint8_t dataToSend[FIXED_PAYLOAD_SIZE];
    memset(dataToSend, 0, sizeof(dataToSend));
    
    for(uint8_t chIdx = 0; chIdx < NUM_RC_CHANNELS; chIdx++)
    {
      uint16_t val = chData[chIdx]; 
      //encode into bit stream, with resolution of 10 bits
      uint8_t aIdx = chIdx + (chIdx / 4);
      uint8_t bIdx = aIdx + 1;
      uint8_t aShift = ((chIdx % 4) + 1) * 2;
      uint8_t bShift = 8 - aShift;
      dataToSend[aIdx] |= ((val >> aShift) & 0xFF);
      dataToSend[bIdx] |= ((val << bShift) & 0xFF);
    }
    
    //write flags in final byte
    dataToSend[sizeof(dataToSend) - 1]  = (isFailsafeData & 0x01) << 4;
    dataToSend[sizeof(dataToSend) - 1] |= (isRequestingTelemetry & 0x01) << 3;
    dataToSend[sizeof(dataToSend) - 1] |= rfPower & 0x07;
    
    //--- Build packet and transmit ---
    buildPacket(Sys.transmitterID, Sys.receiverID, PAC_RC_DATA, dataToSend, sizeof(dataToSend));
    if(LoRa.beginPacket())
    {
      LoRa.write(packet, sizeof(packet));
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
    
    if(isRequestingTelemetry)
    {
      isRequestingTelemetry = false;
      operatingMode = MODE_GET_TELEM;
    }
  }
}

//--------------------------------------------------------------------------------------------------

void getTelemetry()
{
  static uint32_t timeOfLastTelemReception = 0;
  
  if(LoRa.parsePacket()) //received a packet
  {
    readReceivedMessage();
    hop(); 
    if(checkReceivedMessage(Sys.receiverID, Sys.transmitterID) == PAC_TELEMETRY)
    {
      timeOfLastTelemReception = millis();
      receiverPacketRate = msgBuff[3];
      for(uint8_t cntr = 0; cntr < MAX_TELEMETRY_COUNT; cntr++)
      {
        uint8_t buffIdx = 4 + (cntr * 3);
        if(buffIdx >= sizeof(msgBuff) - 1) //we've reached the end, no more values to read
          break;
        telemID[cntr]  = msgBuff[buffIdx];
        telemVal[cntr] = joinBytes(msgBuff[buffIdx + 1], msgBuff[buffIdx + 2]);
      }
    }
    //Change mode
    operatingMode = MODE_RC_DATA;
  }
  
  //reset telemetry
  if(millis() - timeOfLastTelemReception > 2500)
    resetTelemetry();
}

//--------------------------------------------------------------------------------------------------

void resetTelemetry()
{
  receiverPacketRate = 0;
  for(uint8_t cntr = 0; cntr < MAX_TELEMETRY_COUNT; cntr++)
    telemVal[cntr] = TELEMETRY_NO_DATA;
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
    uint8_t dataToSend[FIXED_PAYLOAD_SIZE];
    memset(dataToSend, 0, sizeof(dataToSend));
    dataToSend[0] = isMainReceiver & 0x01;

    buildPacket(Sys.transmitterID, Sys.receiverID, PAC_READ_OUTPUT_CH_CONFIG, dataToSend, sizeof(dataToSend));
    if(LoRa.beginPacket())
    {
      LoRa.write(packet, sizeof(packet));
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
      readReceivedMessage();
      hop();
      if(checkReceivedMessage(Sys.receiverID, Sys.transmitterID) == PAC_READ_OUTPUT_CH_CONFIG)
      {
        gotOutputChConfig = true;
        for(uint8_t i = 0; i < NUM_RC_CHANNELS; i++)
          outputChConfig[i] = msgBuff[3 + i];
        //Change mode
        operatingMode = MODE_RC_DATA;
        //reset
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
        hop();
        operatingMode = MODE_RC_DATA;
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
    uint8_t dataToSend[FIXED_PAYLOAD_SIZE];
    memset(dataToSend, 0, sizeof(dataToSend));
    uint8_t i;
    for(i = 0; i < NUM_RC_CHANNELS; i++)
    {
      dataToSend[i] = outputChConfig[i];
    }
    dataToSend[i++] = isMainReceiver & 0x01;
    
    buildPacket(Sys.transmitterID, Sys.receiverID, PAC_SET_OUTPUT_CH_CONFIG, dataToSend, sizeof(dataToSend));
    if(LoRa.beginPacket())
    {
      LoRa.write(packet, sizeof(packet));
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
      readReceivedMessage();
      hop();
      if(checkReceivedMessage(Sys.receiverID, Sys.transmitterID) == PAC_ACK_OUTPUT_CH_CONFIG)
      {
        //indicate success
        receiverConfigStatusCode = 1;
        //Change mode
        operatingMode = MODE_RC_DATA;
        //reset
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
        //indicate failure
        receiverConfigStatusCode = 2;
        //change mode
        operatingMode = MODE_RC_DATA;
        hop();
      }
    }
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
  //write crc byte at the end
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
