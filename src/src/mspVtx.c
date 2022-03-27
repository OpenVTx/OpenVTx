#include "mspVtx.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "targets.h"
#include "serial.h"
#include <string.h>
#include "helpers.h"

#define MSP_HEADER_DOLLAR       0x24
#define MSP_HEADER_X            0x58
#define MSP_HEADER_REQUEST      0x3C
#define MSP_HEADER_RESPONSE     0x3E
#define MSP_HEADER_ERROR        0x21

#define MSP_VTX_CONFIG                  88 //out message         Get vtx settings - betaflight
#define MSP_SET_VTX_CONFIG              89 //in message          Set vtx settings - betaflight

#define MSP_VTXTABLE_BAND               137    //out message         vtxTable band/channel data
#define MSP_SET_VTXTABLE_BAND           227    //in message          set vtxTable band/channel data (one band at a time)

#define MSP_VTXTABLE_POWERLEVEL         138    //out message         vtxTable powerLevel data
#define MSP_SET_VTXTABLE_POWERLEVEL     228 //in message          set vtxTable powerLevel data (one powerLevel at a time)

#define FC_QUERY_PERIOD_MS      1000

uint32_t lastFlightControllerQueryTime = 0;

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

static uint8_t state, in_idx, in_CRC;
static uint16_t in_Function, in_PayloadSize;

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

    delay(50); // wait for any response
}

void setVtxTableBand(uint8_t band, uint16_t bandNameLength, uint8_t labelChar1, uint8_t labelChar2, uint8_t labelChar3, uint8_t labelChar4, uint8_t bandLetter)
{
    uint16_t payloadSize = 25;

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
    txPacket[9] = bandNameLength;
    txPacket[10] = labelChar1; // nameChar
    txPacket[11] = labelChar2;
    txPacket[12] = labelChar3;
    txPacket[13] = labelChar4;
    txPacket[14] = bandLetter;
    txPacket[15] = 0; // isFactoryBand
    txPacket[16] = 8; // channelCount

    int i;
    for(i = 0; i < 8; i++)
    {
        txPacket[17 + (i * 2)] =  getFreqByIdx(((band-1)*8) + i) & 0xFF;
        txPacket[18 + (i * 2)] = (getFreqByIdx(((band-1)*8) + i) >> 8) & 0xFF;
    }
    
    uint8_t crc = 0;
    for(i = 3; i < payloadSize+8; i++)
    {
        crc = mspCalcCrc(crc, txPacket[i]);
    }

    txPacket[payloadSize+8] = crc;

    mspSendPacket(payloadSize+9);
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

    
    txPacket[8] = 1; // idx LSB
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
}

void mspProcessPacket(void)
{
    switch (in_Function)
    {
    case MSP_VTX_CONFIG:
        memcpy(&in_mspVtxConfigStruct, rxPacket + 8, in_PayloadSize);

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

        // serial_write(in_mspVtxConfigStruct.vtxType);
        // serial_write(in_mspVtxConfigStruct.band);
        // serial_write(in_mspVtxConfigStruct.channel);
        // serial_write(in_mspVtxConfigStruct.power);
        // serial_write(in_mspVtxConfigStruct.pitmode); // Currently not working due to vtxstatus https://github.com/betaflight/betaflight/blob/5b5df65934ad05c0cc95f856ffcdb1f4e7c170f5/src/main/msp/msp.c#L1964
        // serial_write(in_mspVtxConfigStruct.freqLSB);
        // serial_write(in_mspVtxConfigStruct.freqMSB);
        // serial_write(in_mspVtxConfigStruct.deviceIsReady);
        // serial_write(in_mspVtxConfigStruct.lowPowerDisarm);
        // serial_write(in_mspVtxConfigStruct.pitModeFreqLSB);
        // serial_write(in_mspVtxConfigStruct.pitModeFreqMSB);
        // serial_write(in_mspVtxConfigStruct.vtxTableAvailable);
        // serial_write(in_mspVtxConfigStruct.bands);
        // serial_write(in_mspVtxConfigStruct.channels);
        // serial_write(in_mspVtxConfigStruct.powerLevels);
        break;
    // case 'R':// Reboot to bootloader
    //     if (rxPacket[2] == 'S' && rxPacket[3] == 'T')
    //         reboot_into_bootloader(TRAMP_BAUD);
    //     break;
    }
}

void mspQueryFlightController(uint32_t now)
{
    if (now > lastFlightControllerQueryTime)
    {
        // clearVtxTable();

        // queryVtxTablePowerLevel(2);
        // setVtxTablePowerLevel(1, 1,   '0', ' ', ' ');
        // setVtxTablePowerLevel(2, 2,   'R', 'C', 'E');
        // setVtxTablePowerLevel(3, 25,  '2', '5', ' ');
        // setVtxTablePowerLevel(4, 100, '1', '0', '0');
        // setVtxTablePowerLevel(5, 400, '4', '0', '0');

        // queryVtxTableBand(1);
        // setVtxTableBand(1, 4, 'B', 'O', 'S', 'A', 'A');
        // setVtxTableBand(2, 4, 'B', 'O', 'S', 'B', 'B');
        // setVtxTableBand(3, 4, 'B', 'O', 'S', 'E', 'E');
        // setVtxTableBand(4, 4, 'F', 'A', 'T', ' ', 'F');
        // setVtxTableBand(5, 4, 'R', 'A', 'C', 'E', 'R');
        // setVtxTableBand(6, 4, 'L', 'O', 'W', 'R', 'L');

        // queryVtxConfig();
        lastFlightControllerQueryTime = now + FC_QUERY_PERIOD_MS;
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
                    mspProcessPacket();
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