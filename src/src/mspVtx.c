#include "mspVtx.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "targets.h"
#include "serial.h"
#include <string.h>
#include "helpers.h"

#define MSP_HEADER_DOLLAR               0x24
#define MSP_HEADER_X                    0x58
#define MSP_HEADER_REQUEST              0x3C
#define MSP_HEADER_RESPONSE             0x3E
#define MSP_HEADER_ERROR                0x21
#define MSP_HEADER_SIZE                 8

#define MSP_VTX_CONFIG                  88  //out message         Get vtx settings - betaflight
#define MSP_SET_VTX_CONFIG              89  //in message          Set vtx settings - betaflight

#define MSP_VTXTABLE_BAND               137 //out message         vtxTable band/channel data
#define MSP_SET_VTXTABLE_BAND           227 //in message          set vtxTable band/channel data (one band at a time)

#define MSP_VTXTABLE_POWERLEVEL         138 //out message         vtxTable powerLevel data
#define MSP_SET_VTXTABLE_POWERLEVEL     228 //in message          set vtxTable powerLevel data (one powerLevel at a time)

#define MSP_EEPROM_WRITE                250 //in message          no param
#define MSP_REBOOT                      68  //in message reboot settings

#define FC_QUERY_PERIOD_MS              200

typedef enum
{
  GET_VTX_TABLE_SIZE = 0,
  CHECK_POWER_LEVELS,
  CHECK_BANDS,
  SET_DEFAULTS,
  SEND_EEPROM_WRITE,
  MONITORING,
  MSP_STATE_MAX
} mspState_e;

enum {
    MSP_SYNC_DOLLAR = 0,
    MSP_SYNC_X,
    MSP_TYPE,
    MSP_FLAG,
    MSP_FUNCTION,
    MSP_PAYLOAD_SIZE,
    MSP_PAYLOAD,
    MSP_CRC,
};

// https://github.com/betaflight/betaflight/blob/master/src/main/msp/msp.c#L1949
typedef struct
{
    uint8_t vtxType;
    uint8_t band;
    uint8_t channel;
    uint8_t power;
    uint8_t pitmode;
    // uint16_t freq; // This doesnt work and bytes are missing after memcpy.
    uint8_t freqLSB;
    uint8_t freqMSB;
    uint8_t deviceIsReady;
    uint8_t lowPowerDisarm;
    // uint16_t pitModeFreq; // This doesnt work and bytes are missing after memcpy.
    uint8_t pitModeFreqLSB;
    uint8_t pitModeFreqMSB;
    uint8_t vtxTableAvailable;
    uint8_t bands;
    uint8_t channels;
    uint8_t powerLevels;
} mspVtxConfigStruct;

mspVtxConfigStruct in_mspVtxConfigStruct;
uint32_t nextFlightControllerQueryTime = 0;
uint8_t mspState = GET_VTX_TABLE_SIZE;
uint8_t eepromWriteRequired = 0;
uint8_t checkingIndex = 0;

static uint8_t state, in_idx, in_CRC;
static uint16_t in_Function, in_PayloadSize, in_Type;

uint8_t mspCalcCrc(uint8_t crc, unsigned char a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

void mspReset(void)
{
    state = MSP_SYNC_DOLLAR;
    in_idx = 0;
}

void mspSendPacket(uint8_t len)
{
    delay(10); // Flight Controller needs a bit time to swap TX to RX state

    Serial_write_len(txPacket, len);
}

void mspCreateHeader(void)
{
    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;
}

void mspSendSimpleRequest(uint16_t opCode)
{
    mspCreateHeader();

    txPacket[4] = opCode & 0xFF;
    txPacket[5] = (opCode >> 8) & 0xFF;
    
    txPacket[6] = 0; // PayloadSize LSB
    txPacket[7] = 0; // PayloadSize MSB
    
    uint8_t crc = 0;
    for(int i = 3; i < 8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[MSP_HEADER_SIZE] = crc;

    mspSendPacket(MSP_HEADER_SIZE+1);
}

void sendEepromWrite()
{
    if (eepromWriteRequired)
        mspSendSimpleRequest(MSP_EEPROM_WRITE);

    eepromWriteRequired = 0;
    mspState = MONITORING;
}

void setVtxTableBand(uint8_t band)
{
    uint16_t payloadSize = 29;

    mspCreateHeader();

    txPacket[4] = MSP_SET_VTXTABLE_BAND & 0xFF;
    txPacket[5] = (MSP_SET_VTXTABLE_BAND >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    txPacket[8] = band;
    txPacket[9] = BAND_NAME_LENGTH;

    for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
    {
        txPacket[10 + i] = channelFreqLabelByIdx((band - 1) * CHANNEL_COUNT + i);
    }
    
    txPacket[10+CHANNEL_COUNT] = getBandLetterByIdx(band - 1);
    txPacket[11+CHANNEL_COUNT] = IS_FACTORY_BAND;
    txPacket[12+CHANNEL_COUNT] = CHANNEL_COUNT;

    int i;
    for(i = 0; i < CHANNEL_COUNT; i++)
    {
        txPacket[(13+CHANNEL_COUNT) + (i * 2)] =  getFreqByIdx(((band-1) * CHANNEL_COUNT) + i) & 0xFF;
        txPacket[(14+CHANNEL_COUNT) + (i * 2)] = (getFreqByIdx(((band-1) * CHANNEL_COUNT) + i) >> 8) & 0xFF;
    }
    
    uint8_t crc = 0;
    for(i = 3; i < MSP_HEADER_SIZE+payloadSize; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[MSP_HEADER_SIZE+payloadSize] = crc;

    mspSendPacket(MSP_HEADER_SIZE+payloadSize+1);

    eepromWriteRequired = 1;
}

void queryVtxTableBand(uint8_t idx)
{
    uint16_t payloadSize = 1;

    mspCreateHeader();

    txPacket[4] = MSP_VTXTABLE_BAND & 0xFF;
    txPacket[5] = (MSP_VTXTABLE_BAND >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    txPacket[8] = idx; // get power array entry
    
    uint8_t crc = 0;
    for(int i = 3; i < MSP_HEADER_SIZE+payloadSize; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[MSP_HEADER_SIZE+payloadSize] = crc;

    mspSendPacket(MSP_HEADER_SIZE+payloadSize+1);
}

void setVtxTablePowerLevel(uint8_t idx)
{
    uint16_t payloadSize = 7;

    mspCreateHeader();

    txPacket[4] = MSP_SET_VTXTABLE_POWERLEVEL & 0xFF;
    txPacket[5] = (MSP_SET_VTXTABLE_POWERLEVEL >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    txPacket[8] = idx;
    txPacket[9] = saPowerLevelsLut[idx - 1] & 0xFF;         // powerValue LSB
    txPacket[10] = (saPowerLevelsLut[idx - 1] >> 8) & 0xFF; // powerValue MSB
    txPacket[11] = POWER_LEVEL_LABEL_LENGTH; 
    txPacket[12] = saPowerLevelsLabel[((idx - 1) * POWER_LEVEL_LABEL_LENGTH) + 0];
    txPacket[13] = saPowerLevelsLabel[((idx - 1) * POWER_LEVEL_LABEL_LENGTH) + 1];
    txPacket[14] = saPowerLevelsLabel[((idx - 1) * POWER_LEVEL_LABEL_LENGTH) + 2];
    
    uint8_t crc = 0;
    for(int i = 3; i < MSP_HEADER_SIZE+payloadSize; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[MSP_HEADER_SIZE+payloadSize] = crc;

    mspSendPacket(MSP_HEADER_SIZE+payloadSize+1);

    eepromWriteRequired = 1;
}

void queryVtxTablePowerLevel(uint8_t idx)
{
    uint16_t payloadSize = 1;

    mspCreateHeader();

    txPacket[4] = MSP_VTXTABLE_POWERLEVEL & 0xFF;
    txPacket[5] = (MSP_VTXTABLE_POWERLEVEL >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    txPacket[8] = idx + 1;
    
    uint8_t crc = 0;
    for(int i = 3; i < MSP_HEADER_SIZE+payloadSize; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[MSP_HEADER_SIZE+payloadSize] = crc;

    mspSendPacket(MSP_HEADER_SIZE+payloadSize+1);
}

void clearVtxTable(void)
{
    uint16_t payloadSize = 15;

    mspCreateHeader();

    txPacket[4] = MSP_SET_VTX_CONFIG & 0xFF;
    txPacket[5] = (MSP_SET_VTX_CONFIG >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;

    
    txPacket[8] = 0; // idx LSB
    txPacket[9] = 0;  // idx MSB
    txPacket[10] = 3; // 25mW Power idx
    txPacket[11] = 0; // pitmode
    txPacket[12] = 0; // lowPowerDisarm 
    txPacket[13] = 0; // pitModeFreq LSB
    txPacket[14] = 0; // pitModeFreq MSB
    txPacket[15] = 4; // newBand - Band Fatshark
    txPacket[16] = 4; // newChannel - Channel 4
    txPacket[17] = 0; // newFreq  LSB
    txPacket[18] = 0; // newFreq  MSB
    txPacket[19] = 6; // newBandCount  
    txPacket[20] = 8; // newChannelCount 
    txPacket[21] = 5; // newPowerCount 
    txPacket[22] = 1; // vtxtable should be cleared  
    
    uint8_t crc = 0;
    for(int i = 3; i < MSP_HEADER_SIZE+payloadSize; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[MSP_HEADER_SIZE+payloadSize] = crc;

    mspSendPacket(MSP_HEADER_SIZE+payloadSize+1);

    eepromWriteRequired = 1;
}

void mspProcessPacket(void)
{
    uint16_t value;
    uint8_t bandNameLength, bandLetter, bandFactory, bandLength;
    uint8_t frequenciesCorrect = 1;

    switch (in_Function)
    {
    case MSP_VTX_CONFIG:
        memcpy(&in_mspVtxConfigStruct, rxPacket + 8, in_PayloadSize);
        uint8_t channel;

        switch (mspState)
        {
        case GET_VTX_TABLE_SIZE:

            //  Store initially received values.  If the VTx Table is correct, only then set these values.  //
            pitMode = in_mspVtxConfigStruct.pitmode;

            in_mspVtxConfigStruct.power -= 1; // Correct for BF starting at 1.
            if (in_mspVtxConfigStruct.lowPowerDisarm) // Force on boot because BF doesnt send a low power index.
            {
                in_mspVtxConfigStruct.power = 0; 
            }

            if (in_mspVtxConfigStruct.power < SA_NUM_POWER_LEVELS)
            {
                myEEPROM.currPowerdB = saPowerLevelsLut[in_mspVtxConfigStruct.power];
            }
            else
            {
                myEEPROM.currPowerdB = 14; // 25 mW
            }
            
            myEEPROM.channel = ((in_mspVtxConfigStruct.band - 1) * 8) + (in_mspVtxConfigStruct.channel - 1);   
            if (myEEPROM.channel >= getFreqTableSize())
            {
                myEEPROM.channel = 27; // F4 5800MHz
            } 
            myEEPROM.freqMode = 0;
            //////////////////////////////////////////////////////////////////////////////////////////////////

            if (in_mspVtxConfigStruct.bands == getFreqTableBands() && in_mspVtxConfigStruct.channels == getFreqTableChannels() && in_mspVtxConfigStruct.powerLevels == SA_NUM_POWER_LEVELS)
            {
                mspState = CHECK_POWER_LEVELS;
                nextFlightControllerQueryTime = millis();
                break;
            }
            clearVtxTable();
            break;
        case MONITORING:
            pitMode = in_mspVtxConfigStruct.pitmode;

            // Set power before freq changes to prevent PLL settling issues and spamming other frequencies.
            in_mspVtxConfigStruct.power -= 1; // Correct for BF starting at 1.
            setPowerdB(saPowerLevelsLut[in_mspVtxConfigStruct.power]);

            channel = ((in_mspVtxConfigStruct.band - 1) * 8) + (in_mspVtxConfigStruct.channel - 1);
            if (channel < getFreqTableSize())
            {            
                myEEPROM.channel = channel;
                rtc6705WriteFrequency(getFreqByIdx(channel));
                myEEPROM.freqMode = 0;
            }
            break;
        }
        break;

    case MSP_VTXTABLE_POWERLEVEL:
        value = ((uint16_t)rxPacket[10] << 8) + rxPacket[9];
        switch (mspState)
        {        
        case CHECK_POWER_LEVELS:
            if (value ==  saPowerLevelsLut[checkingIndex] && rxPacket[11] == POWER_LEVEL_LABEL_LENGTH) // Check lengths before trying to check content
            {
                if (rxPacket[12] == saPowerLevelsLabel[checkingIndex * POWER_LEVEL_LABEL_LENGTH + 0] &&
                    rxPacket[13] == saPowerLevelsLabel[checkingIndex * POWER_LEVEL_LABEL_LENGTH + 1] &&
                    rxPacket[14] == saPowerLevelsLabel[checkingIndex * POWER_LEVEL_LABEL_LENGTH + 2])
                {
                    checkingIndex++;
                    if (checkingIndex > SA_NUM_POWER_LEVELS - 1)
                    {
                        checkingIndex = 0;
                        mspState = CHECK_BANDS;
                    }
                    nextFlightControllerQueryTime = millis();
                    break;
                }
            }
            setVtxTablePowerLevel(checkingIndex + 1);
            break;
        }
        break;

    case MSP_VTXTABLE_BAND:
        bandNameLength = rxPacket[9];
        bandLetter = rxPacket[10 + bandNameLength];
        bandFactory = rxPacket[11 + bandNameLength];
        bandLength = rxPacket[12 + bandNameLength];
        switch (mspState)
        {
        case CHECK_BANDS:
            if (bandNameLength ==  BAND_NAME_LENGTH && bandLetter == getBandLetterByIdx(checkingIndex) && bandFactory == IS_FACTORY_BAND && bandLength == CHANNEL_COUNT) // Check lengths before trying to check content
            {
                if (rxPacket[10] == channelFreqLabelByIdx(checkingIndex * CHANNEL_COUNT + 0) &&
                    rxPacket[11] == channelFreqLabelByIdx(checkingIndex * CHANNEL_COUNT + 1) &&
                    rxPacket[12] == channelFreqLabelByIdx(checkingIndex * CHANNEL_COUNT + 2) &&
                    rxPacket[13] == channelFreqLabelByIdx(checkingIndex * CHANNEL_COUNT + 3) &&
                    rxPacket[14] == channelFreqLabelByIdx(checkingIndex * CHANNEL_COUNT + 4) &&
                    rxPacket[15] == channelFreqLabelByIdx(checkingIndex * CHANNEL_COUNT + 5) &&
                    rxPacket[16] == channelFreqLabelByIdx(checkingIndex * CHANNEL_COUNT + 6) &&
                    rxPacket[17] == channelFreqLabelByIdx(checkingIndex * CHANNEL_COUNT + 7))
                {
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        value = ((uint16_t)rxPacket[22 + (2*i)] << 8) + rxPacket[21 + (2*i)];
                        if (value != getFreqByIdx(checkingIndex * CHANNEL_COUNT + i))
                            frequenciesCorrect = 0;
                    }

                    if (frequenciesCorrect)
                    {
                        checkingIndex++;
                        if (checkingIndex > getFreqTableBands() - 1)
                        {
                            mspState = MONITORING;
                            if (eepromWriteRequired) mspState = SET_DEFAULTS;
                        }
                        nextFlightControllerQueryTime = millis();
                        break;
                    }
                }
            }
            setVtxTableBand(checkingIndex + 1);
            break;
        }
        break;

    case MSP_SET_VTX_CONFIG:
        switch (mspState)
        {
        case SET_DEFAULTS:
            mspState = SEND_EEPROM_WRITE;
            nextFlightControllerQueryTime = millis();
            break;
        }

    case MSP_SET_VTXTABLE_BAND:
    case MSP_SET_VTXTABLE_POWERLEVEL:
        nextFlightControllerQueryTime = millis();
        break;

    case MSP_EEPROM_WRITE:
        mspState = MONITORING;
        nextFlightControllerQueryTime = millis();
        break;
    case MSP_REBOOT:
        reboot_into_bootloader(9600);
        break;
    }
}

void mspProcessSerial(void)
{
    if (serial_available())
    {
        uint8_t data = serial_read();

        rxPacket[in_idx++] = data;

        switch (state)
        {
            case MSP_SYNC_DOLLAR:
                if (data == MSP_HEADER_DOLLAR) {
                    state = MSP_SYNC_X;
                } else {
                    state = MSP_SYNC_DOLLAR;
                    in_idx = 0;
                }
                break;
            case MSP_SYNC_X:
                if (data == MSP_HEADER_X) {
                    state = MSP_TYPE;
                    status_led3(1); // Got header so turn on incoming packet LED.
                } else {
                    state = MSP_SYNC_DOLLAR;
                    in_idx = 0;
                }
                break;
            case MSP_TYPE:
                if (data == MSP_HEADER_REQUEST || data == MSP_HEADER_RESPONSE || data == MSP_HEADER_ERROR) {
                    in_Type = data;
                    state = MSP_FLAG;
                    in_CRC = 0;
                } else {
                    state = MSP_SYNC_DOLLAR;
                    in_idx = 0;
                }
                break;
            case MSP_FLAG:
                in_CRC = mspCalcCrc(in_CRC, data);
                state = MSP_FUNCTION;
                break;
            case MSP_FUNCTION:
                in_CRC = mspCalcCrc(in_CRC, data);
                if (in_idx == 5)
                {
                    in_Function = ((uint16_t)rxPacket[5] << 8) | rxPacket[4];
                    state = MSP_PAYLOAD_SIZE;
                }
                break;
            case MSP_PAYLOAD_SIZE:
                in_CRC = mspCalcCrc(in_CRC, data);
                if (in_idx == 7)
                {
                    in_PayloadSize = ((uint16_t)rxPacket[7] << 8) | rxPacket[6];
                    state = MSP_PAYLOAD;
                }
                break;
            case MSP_PAYLOAD:
                in_CRC = mspCalcCrc(in_CRC, data);
                if (in_idx == (8 + in_PayloadSize))
                {
                    state = MSP_CRC;
                }
                break;
            case MSP_CRC:
                if (in_CRC == data)
                {
                    vtxModeLocked = 1; // Successfully got a packet so lock VTx mode.
                    if (in_Type != MSP_HEADER_ERROR)
                        mspProcessPacket();
                }
                state = MSP_SYNC_DOLLAR;
                in_idx = 0;
                break;
            default:
                state = MSP_SYNC_DOLLAR;
                in_idx = 0;
            break;
        }

        if (state == MSP_SYNC_DOLLAR)
            status_led3(0);
    }
}

void mspUpdate(uint32_t now)
{
    mspProcessSerial();

    if (now < nextFlightControllerQueryTime)
    {
        return;
    }

    nextFlightControllerQueryTime = now + FC_QUERY_PERIOD_MS; // Wait for any reply.

    switch (mspState)
    {
        case GET_VTX_TABLE_SIZE:
            mspSendSimpleRequest(MSP_VTX_CONFIG);
            break;
        case CHECK_POWER_LEVELS:
            queryVtxTablePowerLevel(checkingIndex);
            break;
        case CHECK_BANDS:
            queryVtxTableBand(checkingIndex + 1);
            break;
        case SET_DEFAULTS:
            initFreqPacketRecived = 1;
            setPowerdB(myEEPROM.currPowerdB);
            rtc6705WriteFrequency(getFreqByIdx(myEEPROM.channel));
            break;
        case SEND_EEPROM_WRITE:
            sendEepromWrite();
            break;
        case MONITORING:
            if (!initFreqPacketRecived)
            {
                initFreqPacketRecived = 1;
                setPowerdB(myEEPROM.currPowerdB);
                rtc6705WriteFrequency(getFreqByIdx(myEEPROM.channel));
            }
            break;
        default:
            return;
        break;
    }
    
    return;
}