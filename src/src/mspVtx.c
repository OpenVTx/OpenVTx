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

#define MSP_VTX_CONFIG                  88 //out message         Get vtx settings - betaflight
#define MSP_SET_VTX_CONFIG              89 //in message          Set vtx settings - betaflight

#define MSP_VTXTABLE_BAND               137    //out message         vtxTable band/channel data
#define MSP_SET_VTXTABLE_BAND           227    //in message          set vtxTable band/channel data (one band at a time)

#define MSP_VTXTABLE_POWERLEVEL         138    //out message         vtxTable powerLevel data
#define MSP_SET_VTXTABLE_POWERLEVEL     228 //in message          set vtxTable powerLevel data (one powerLevel at a time)

#define MSP_EEPROM_WRITE                250    //in message          no param

#define FC_QUERY_PERIOD_MS              1000

typedef enum
{
  GET_VTX_TABLE_SIZE = 0,
  GET_POWER_LEVEL_1,
  GET_POWER_LEVEL_2,
  GET_POWER_LEVEL_3,
  GET_POWER_LEVEL_4,
  GET_POWER_LEVEL_5,
  GET_BAND_1,
  GET_BAND_2,
  GET_BAND_3,
  GET_BAND_4,
  GET_BAND_5,
  GET_BAND_6,
  SET_DEFAULTS,
  SEND_EEPROM_WRITE,
  NORMAL,
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

void mspSendPacket(uint8_t len)
{
    // Flight Controller needs a bit time to swap TX to RX state
    delay(10);

    Serial_write_len(txPacket, len);
}

void sendEepromWrite()
{
    if (!eepromWriteRequired)
    {
        mspState = NORMAL;
        return;
    }

    eepromWriteRequired = 0;

    uint16_t payloadSize = 0;

    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;

    txPacket[4] = MSP_EEPROM_WRITE & 0xFF;
    txPacket[5] = (MSP_EEPROM_WRITE >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    uint8_t crc = 0;
    for(int i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);
}

void setVtxTableBand(uint8_t band, uint8_t labelChar1, uint8_t labelChar2, uint8_t labelChar3, uint8_t labelChar4,
                                   uint8_t labelChar5, uint8_t labelChar6, uint8_t labelChar7, uint8_t labelChar8, uint8_t bandLetter)
{
    uint16_t payloadSize = 29;

    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;

    txPacket[4] = MSP_SET_VTXTABLE_BAND & 0xFF;
    txPacket[5] = (MSP_SET_VTXTABLE_BAND >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    txPacket[8] = band;
    txPacket[9] = 8; //bandNameLength;
    txPacket[10] = labelChar1; // nameChar
    txPacket[11] = labelChar2;
    txPacket[12] = labelChar3;
    txPacket[13] = labelChar4;
    txPacket[14] = labelChar5;
    txPacket[15] = labelChar6;
    txPacket[16] = labelChar7;
    txPacket[17] = labelChar8;
    txPacket[18] = bandLetter;
    txPacket[19] = 0; // isFactoryBand
    txPacket[20] = 8; // channelCount

    int i;
    for(i = 0; i < 8; i++)
    {
        txPacket[21 + (i * 2)] =  getFreqByIdx(((band-1)*8) + i) & 0xFF;
        txPacket[22 + (i * 2)] = (getFreqByIdx(((band-1)*8) + i) >> 8) & 0xFF;
    }
    
    uint8_t crc = 0;
    for(i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);

    eepromWriteRequired = 1;
}

void queryVtxTableBand(uint8_t idx)
{
    uint16_t payloadSize = 1;

    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;

    txPacket[4] = MSP_VTXTABLE_BAND & 0xFF;
    txPacket[5] = (MSP_VTXTABLE_BAND >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    txPacket[8] = idx; // get power array entry
    
    uint8_t crc = 0;
    for(int i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);
}

void setVtxTablePowerLevel(uint8_t idx, uint16_t powerValue, uint8_t labelChar1, uint8_t labelChar2, uint8_t labelChar3)
{
    uint16_t payloadSize = 7;

    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;

    txPacket[4] = MSP_SET_VTXTABLE_POWERLEVEL & 0xFF;
    txPacket[5] = (MSP_SET_VTXTABLE_POWERLEVEL >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    txPacket[8] = idx; // power array entry
    txPacket[9] = powerValue & 0xFF; // powerValue LSB
    txPacket[10] = (powerValue >> 8) & 0xFF; // powerValue MSB
    txPacket[11] = 3; // powerLevelLabelLength 3
    txPacket[12] = labelChar1; // power array entry
    txPacket[13] = labelChar2; // power array entry
    txPacket[14] = labelChar3; // power array entry
    
    uint8_t crc = 0;
    for(int i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);

    eepromWriteRequired = 1;
}

void queryVtxTablePowerLevel(uint8_t idx)
{
    uint16_t payloadSize = 1;

    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;

    txPacket[4] = MSP_VTXTABLE_POWERLEVEL & 0xFF;
    txPacket[5] = (MSP_VTXTABLE_POWERLEVEL >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    txPacket[8] = idx; // get power array entry
    
    uint8_t crc = 0;
    for(int i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);
}

void setDefaultBandChannelPower()
{
    uint16_t payloadSize = 4;

    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;

    txPacket[4] = MSP_SET_VTX_CONFIG & 0xFF;
    txPacket[5] = (MSP_SET_VTX_CONFIG >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;

    
    txPacket[8] = 27; // idx LSB - Set default to F4 5800MHz
    txPacket[9] = 0; // idx MSB
    txPacket[10] = 3; // idx Power idx
    txPacket[11] = 0; // pitmode
    
    uint8_t crc = 0;
    for(int i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);

    eepromWriteRequired = 1;
}

void clearVtxTable(void)
{
    uint16_t payloadSize = 15;

    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;

    txPacket[4] = MSP_SET_VTX_CONFIG & 0xFF;
    txPacket[5] = (MSP_SET_VTX_CONFIG >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;

    
    txPacket[8] = 28; // idx LSB - Set default to F4 5800MHz
    txPacket[9] = 0; // idx MSB
    txPacket[10] = 0; // idx Power idx
    txPacket[11] = 0; // pitmode
    txPacket[12] = 0; // lowPowerDisarm 
    txPacket[13] = 0; // pitModeFreq LSB
    txPacket[14] = 0; // pitModeFreq MSB
    txPacket[15] = 1; // newBand 
    txPacket[16] = 1; // newChannel 
    txPacket[17] = 0; // newFreq  LSB
    txPacket[18] = 0; // newFreq  MSB
    txPacket[19] = 6; // newBandCount  
    txPacket[20] = 8; // newChannelCount 
    txPacket[21] = 5; // newPowerCount 
    txPacket[22] = 1; // vtxtable should be cleared  
    
    uint8_t crc = 0;
    for(int i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);

    eepromWriteRequired = 1;
}

void queryVtxConfig(void)
{
    uint16_t payloadSize = 0;

    zeroTxPacket();
    txPacket[0] = '$';
    txPacket[1] = 'X';
    txPacket[2] = '<';
    txPacket[3] = 0;

    txPacket[4] = MSP_VTX_CONFIG & 0xFF;
    txPacket[5] = (MSP_VTX_CONFIG >> 8) & 0xFF;
    
    txPacket[6] = payloadSize & 0xFF;
    txPacket[7] = (payloadSize >> 8) & 0xFF;
    
    uint8_t crc = 0;
    for(int i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);
}

void mspProcessPacket(void)
{
    uint16_t powerValue;
    uint8_t bandNameLength, bandLetter, bandFactory, bandLength;
    uint8_t frequenciesCorrect = 1;

    switch (in_Function)
    {
    case MSP_VTX_CONFIG:
        memcpy(&in_mspVtxConfigStruct, rxPacket + 8, in_PayloadSize);

        switch (mspState)
        {
        case GET_VTX_TABLE_SIZE:
            if (in_mspVtxConfigStruct.bands == getFreqTableBands() && in_mspVtxConfigStruct.channels == getFreqTableChannels() && in_mspVtxConfigStruct.powerLevels == SA_NUM_POWER_LEVELS)
            {
                mspState = GET_POWER_LEVEL_1;
                nextFlightControllerQueryTime = millis();
                break;
            }
            clearVtxTable();
            break;
        case NORMAL:
            initFreqPacketRecived = 1;

            pitMode = in_mspVtxConfigStruct.pitmode;

            // Set power before freq changes to prevent PLL settling issues and spamming other frequencies.
            in_mspVtxConfigStruct.power -= 1; // Correct for BF starting at 1.
            setPowerdB(saPowerLevelsLut[in_mspVtxConfigStruct.power]);

            uint8_t channel = ((in_mspVtxConfigStruct.band - 1) * 8) + (in_mspVtxConfigStruct.channel - 1);
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
        powerValue = ((uint16_t)rxPacket[10] << 8) + rxPacket[9];
        switch (mspState)
        {
        case GET_POWER_LEVEL_1:
            if (powerValue ==  saPowerLevelsLut[0] &&  rxPacket[12] == '0' &&  rxPacket[13] == ' ' &&  rxPacket[14] == ' ')
            {
                mspState = GET_POWER_LEVEL_2;
                nextFlightControllerQueryTime = millis();
                break;
            }
            setVtxTablePowerLevel(1, 1,   '0', ' ', ' ');
            break;
        case GET_POWER_LEVEL_2:
            if (powerValue ==  saPowerLevelsLut[1] &&  rxPacket[12] == 'R' &&  rxPacket[13] == 'C' &&  rxPacket[14] == 'E')
            {
                mspState = GET_POWER_LEVEL_3;
                nextFlightControllerQueryTime = millis();
                break;
            }
            setVtxTablePowerLevel(2, 2,   'R', 'C', 'E');
            break;
        case GET_POWER_LEVEL_3:
            if (powerValue ==  saPowerLevelsLut[2] &&  rxPacket[12] == '2' &&  rxPacket[13] == '5' &&  rxPacket[14] == ' ')
            {
                mspState = GET_POWER_LEVEL_4;
                nextFlightControllerQueryTime = millis();
                break;
            }
            setVtxTablePowerLevel(3, 14,  '2', '5', ' ');
            break;
        case GET_POWER_LEVEL_4:
            if (powerValue ==  saPowerLevelsLut[3] &&  rxPacket[12] == '1' &&  rxPacket[13] == '0' &&  rxPacket[14] == '0')
            {
                mspState = GET_POWER_LEVEL_5;
                nextFlightControllerQueryTime = millis();
                break;
            }
            setVtxTablePowerLevel(4, 20, '1', '0', '0');
            break;
        case GET_POWER_LEVEL_5:
            if (powerValue ==  saPowerLevelsLut[4] &&  rxPacket[12] == '4' &&  rxPacket[13] == '0' &&  rxPacket[14] == '0')
            {
                mspState = GET_BAND_1;
                nextFlightControllerQueryTime = millis();
                break;
            }
            setVtxTablePowerLevel(5, 26, '4', '0', '0');
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
        case GET_BAND_1:
            if (bandNameLength ==  8 && bandLetter == 'A' && !bandFactory && bandLength == 8)
            {
                if (rxPacket[10] == 'B' && rxPacket[11] == 'A' && rxPacket[12] == 'N' && rxPacket[13] == 'D' && 
                    rxPacket[14] == ' ' && rxPacket[15] == 'A' && rxPacket[16] == ' ' && rxPacket[17] == ' ')
                {
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        powerValue = ((uint16_t)rxPacket[22 + (2*i)] << 8) + rxPacket[21 + (2*i)];
                        if (powerValue != getFreqByIdx(i))
                            frequenciesCorrect = 0;
                    }

                    if (frequenciesCorrect)
                    {
                        mspState = GET_BAND_2;
                        nextFlightControllerQueryTime = millis();
                        break;
                    }
                }
            }
            setVtxTableBand(1, 'B', 'A', 'N', 'D', ' ', 'A', ' ', ' ', 'A');
            break;
        case GET_BAND_2:
            if (bandNameLength ==  8 && bandLetter == 'B' && !bandFactory && bandLength == 8)
            {
                if (rxPacket[10] == 'B' && rxPacket[11] == 'A' && rxPacket[12] == 'N' && rxPacket[13] == 'D' && 
                    rxPacket[14] == ' ' && rxPacket[15] == 'B' && rxPacket[16] == ' ' && rxPacket[17] == ' ')
                {
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        powerValue = ((uint16_t)rxPacket[22 + (2*i)] << 8) + rxPacket[21 + (2*i)];
                        if (powerValue != getFreqByIdx(i + 8))
                            frequenciesCorrect = 0;
                    }

                    if (frequenciesCorrect)
                    {
                        mspState = GET_BAND_3;
                        nextFlightControllerQueryTime = millis();
                        break;
                    }
                }
            }
            setVtxTableBand(2, 'B', 'A', 'N', 'D', ' ', 'B', ' ', ' ', 'B');
            break;
        case GET_BAND_3:
            if (bandNameLength ==  8 && bandLetter == 'E' && !bandFactory && bandLength == 8)
            {
                if (rxPacket[10] == 'B' && rxPacket[11] == 'A' && rxPacket[12] == 'N' && rxPacket[13] == 'D' && 
                    rxPacket[14] == ' ' && rxPacket[15] == 'E' && rxPacket[16] == ' ' && rxPacket[17] == ' ')
                {
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        powerValue = ((uint16_t)rxPacket[22 + (2*i)] << 8) + rxPacket[21 + (2*i)];
                        if (powerValue != getFreqByIdx(i + 16))
                            frequenciesCorrect = 0;
                    }

                    if (frequenciesCorrect)
                    {
                        mspState = GET_BAND_4;
                        nextFlightControllerQueryTime = millis();
                        break;
                    }
                }
            }
            setVtxTableBand(3, 'B', 'A', 'N', 'D', ' ', 'E', ' ', ' ', 'E');
            break;
        case GET_BAND_4:
            if (bandNameLength ==  8 && bandLetter == 'F' && !bandFactory && bandLength == 8)
            {
                if (rxPacket[10] == 'F' && rxPacket[11] == 'A' && rxPacket[12] == 'T' && rxPacket[13] == 'S' && 
                    rxPacket[14] == 'H' && rxPacket[15] == 'A' && rxPacket[16] == 'R' && rxPacket[17] == 'K')
                {
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        powerValue = ((uint16_t)rxPacket[22 + (2*i)] << 8) + rxPacket[21 + (2*i)];
                        if (powerValue != getFreqByIdx(i + 24))
                            frequenciesCorrect = 0;
                    }

                    if (frequenciesCorrect)
                    {
                        mspState = GET_BAND_5;
                        nextFlightControllerQueryTime = millis();
                        break;
                    }
                }
            }
            setVtxTableBand(4, 'F', 'A', 'T', 'S', 'H', 'A', 'R', 'K', 'F');
            break;
        case GET_BAND_5:
            if (bandNameLength ==  8 && bandLetter == 'R' && !bandFactory && bandLength == 8)
            {
                if (rxPacket[10] == 'R' && rxPacket[11] == 'A' && rxPacket[12] == 'C' && rxPacket[13] == 'E' && 
                    rxPacket[14] == ' ' && rxPacket[15] == ' ' && rxPacket[16] == ' ' && rxPacket[17] == ' ')
                {
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        powerValue = ((uint16_t)rxPacket[22 + (2*i)] << 8) + rxPacket[21 + (2*i)];
                        if (powerValue != getFreqByIdx(i + 32))
                            frequenciesCorrect = 0;
                    }

                    if (frequenciesCorrect)
                    {
                        mspState = GET_BAND_6;
                        nextFlightControllerQueryTime = millis();
                        break;
                    }
                }
            }
            setVtxTableBand(5, 'R', 'A', 'C', 'E', ' ', ' ', ' ', ' ', 'R');
            break;
        case GET_BAND_6:
            if (bandNameLength ==  8 && bandLetter == 'L' && !bandFactory && bandLength == 8)
            {
                if (rxPacket[10] == 'R' && rxPacket[11] == 'A' && rxPacket[12] == 'C' && rxPacket[13] == 'E' && 
                    rxPacket[14] == ' ' && rxPacket[15] == 'L' && rxPacket[16] == 'O' && rxPacket[17] == 'W')
                {
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        powerValue = ((uint16_t)rxPacket[22 + (2*i)] << 8) + rxPacket[21 + (2*i)];
                        if (powerValue != getFreqByIdx(i + 40))
                            frequenciesCorrect = 0;
                    }

                    if (frequenciesCorrect)
                    {
                        mspState = NORMAL;
                        if (eepromWriteRequired) mspState = SET_DEFAULTS;
                        nextFlightControllerQueryTime = millis();
                        break;
                    }
                }
            }
            setVtxTableBand(6, 'R', 'A', 'C', 'E', ' ', 'L', 'O', 'W', 'L');
            break;
        }
        break;

    // Reply received and the FC is ready to be queried again.
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
        mspState = NORMAL;
        nextFlightControllerQueryTime = millis();
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
                    status_led3(1);
                    vtxModeLocked = 1; // Successfully got a packet so lock VTx mode.
                    if (in_Type != MSP_HEADER_ERROR) mspProcessPacket();
                    status_led3(0);
                }
                state = MSP_SYNC_DOLLAR;
                in_idx = 0;
                break;
            default:
                state = MSP_SYNC_DOLLAR;
                in_idx = 0;
            break;
        }
    }
}

void mspUpdate(uint32_t now)
{
    mspProcessSerial();

    if (now < nextFlightControllerQueryTime)
    {
        return;
    }

    nextFlightControllerQueryTime = now + FC_QUERY_PERIOD_MS;

    switch (mspState)
    {
        case GET_VTX_TABLE_SIZE:
            queryVtxConfig();
            break;
        case GET_POWER_LEVEL_1:
            queryVtxTablePowerLevel(1);
            break;
        case GET_POWER_LEVEL_2:
            queryVtxTablePowerLevel(2);
            break;
        case GET_POWER_LEVEL_3:
            queryVtxTablePowerLevel(3);
            break;
        case GET_POWER_LEVEL_4:
            queryVtxTablePowerLevel(4);
            break;
        case GET_POWER_LEVEL_5:
            queryVtxTablePowerLevel(5);
            break;
        case GET_BAND_1:
            queryVtxTableBand(1);
            break;
        case GET_BAND_2:
            queryVtxTableBand(2);
            break;
        case GET_BAND_3:
            queryVtxTableBand(3);
            break;
        case GET_BAND_4:
            queryVtxTableBand(4);
            break;
        case GET_BAND_5:
            queryVtxTableBand(5);
            break;
        case GET_BAND_6:
            queryVtxTableBand(6);
            break;
        case SET_DEFAULTS:
            setDefaultBandChannelPower();
            break;
        case SEND_EEPROM_WRITE:
            sendEepromWrite();
            break;
        case NORMAL:
            queryVtxConfig();
            break;
        default:
            return;
        break;
    }
    
    return;
}