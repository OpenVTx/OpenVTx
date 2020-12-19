#include "smartAudio.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "targets.h"
#include "helpers.h"
#include "serial.h"


const uint16_t channelFreqTable[48] = {
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // F
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // R
    5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621  // L
};


// https://github.com/betaflight/betaflight/blob/287741b816fb5bdac1f72a825846303454765fac/src/main/io/vtx_smartaudio.c#L152
uint8_t smartadioCalcCrc(const uint8_t *data, uint8_t len)
{
#define POLYGEN 0xd5
    uint8_t crc = 0;
    uint8_t currByte;

    for (int i = 0; i < len; i++)
    {
        currByte = data[i];
        crc ^= currByte;
        for (int i = 0; i < 8; i++)
        {
            if ((crc & 0x80) != 0)
            {
                crc = (uint8_t)((crc << 1) ^ POLYGEN);
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void smartaudioSendPacket(void)
{
    uint8_t len = txPacket[3] + 4;
    txPacket[(len-1)] = smartadioCalcCrc(&txPacket[2], (len - 3)); // Fill CRC
    Serial_write_len(txPacket, len);
    serial_flush();
}

void smartaudioBuildSettingsPacket(void)
{
    uint8_t operationMode = 0;
    bitWrite(operationMode, 0, myEEPROM.freqMode);
    bitWrite(operationMode, 1, pitMode);
    bitWrite(operationMode, 2, myEEPROM.pitmodeInRange);
    bitWrite(operationMode, 3, myEEPROM.pitmodeOutRange);
    bitWrite(operationMode, 4, myEEPROM.unlocked);

    txPacket[0] = SMARTAUDIO_SYNC;
    txPacket[1] = SMARTAUDIO_HEADER;
    txPacket[2] = VERSION_2_1_COMMAND;
    txPacket[3] = 12;                                               // Payload length
    txPacket[4] = myEEPROM.channel;                                 // Channel
    txPacket[5] = myEEPROM.currPowerIndex;                          // Power​​Level
    txPacket[6] = operationMode;                                    // OperationMode
    txPacket[7] = (myEEPROM.currFreq >> 8) & 0xff;                  // Current​​Frequency​​
    txPacket[8] = myEEPROM.currFreq & 0xff;                         // Current​​Frequency​​
    txPacket[9] = myEEPROM.currPowerdB;                             // current power in dBm
    txPacket[10] = 0x03;                                            // Number of power levels.  If changes remember to alter payload length
    txPacket[11] = 0;                                               // 1mW
    txPacket[12] = 14;                                              // 25mW
    txPacket[13] = 20;                                              // 100mW
    txPacket[14] = 23;                                              // 200mW
    //txPacket[15] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC
    smartaudioSendPacket();
}

void smartaudioProcessFrequencyPacket(void)
{
    int returnFreq;
    int newFreq = rxPacket[5] | (rxPacket[4] << 8);

    if ((newFreq >> 8) & PIT_MODE_FREQ_REQUEST)
    {
        returnFreq = myEEPROM.currFreq; // POR is not supported in SA2.1 so return currFreq
    }
    else if ((newFreq >> 8) & PIT_MODE_FREQ_SET)
    {
        returnFreq = myEEPROM.currFreq; // POR is not supported in SA2.1 so do not set a POR freq
    }
    else
    {
        myEEPROM.currFreq = newFreq;
        returnFreq = myEEPROM.currFreq;
        rtc6705WriteFrequency(myEEPROM.currFreq);
    }

    txPacket[0] = SMARTAUDIO_SYNC;
    txPacket[1] = SMARTAUDIO_HEADER;
    txPacket[2] = SET_FREQUENCY;
    txPacket[3] = 0x04; // Length
    txPacket[4] = (returnFreq >> 8) & 0xFF;
    txPacket[5] = returnFreq & 0xFF;
    txPacket[6] = RESERVE_BYTE;
    //txPacket[7] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC

    myEEPROM.freqMode = 1;
    updateEEPROM = 1;

    smartaudioSendPacket();
}

void smartaudioProcessChannelPacket(void)
{
    const uint8_t channel = rxPacket[4];
    myEEPROM.channel = channel;
    myEEPROM.currFreq = channelFreqTable[channel];

    rtc6705WriteFrequency(myEEPROM.currFreq);

    txPacket[0] = SMARTAUDIO_SYNC;
    txPacket[1] = SMARTAUDIO_HEADER;
    txPacket[2] = SET_CHANNEL;
    txPacket[3] = 0x03; // Length
    txPacket[4] = channel;
    txPacket[5] = RESERVE_BYTE;
    //txPacket[6] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC

    myEEPROM.freqMode = 0;
    updateEEPROM = 1;

    smartaudioSendPacket();
}

void smartaudioProcessPowerPacket(void)
{
    bitWrite(rxPacket[4], 7, 0); // SA2.1 sets the MSB to indicate power is in dB. Set MSB to zero and currPower will now be in dB.
    setPowerdB(rxPacket[4]);
    myEEPROM.currPowerdB = rxPacket[4];
    updateEEPROM = 1;

    txPacket[0] = SMARTAUDIO_SYNC;
    txPacket[1] = SMARTAUDIO_HEADER;
    txPacket[2] = SET_POWER;
    txPacket[3] = 0x03; // Length
    txPacket[4] = myEEPROM.currPowerdB;
    txPacket[5] = RESERVE_BYTE;
    //txPacket[6] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC
    smartaudioSendPacket();
}

void smartaudioProcessModePacket(void)
{
    // Set PIR and POR. POR is no longer used in SA2.1 and is treated like PIR
    myEEPROM.pitmodeInRange = bitRead(rxPacket[4], 0);
    myEEPROM.pitmodeOutRange = bitRead(rxPacket[4], 1);

    // This bit is only for CLEARING pitmode.  It does not turn pitMode on and off!!!
    if (bitRead(rxPacket[4], 2))
    {
        pitMode = 0;
        setPowerdB(myEEPROM.currPowerdB);
    }

    // Unlocked bit
    myEEPROM.unlocked = bitRead(rxPacket[4], 3);

    updateEEPROM = 1;

    uint8_t operationMode = 0;
    bitWrite(operationMode, 0, myEEPROM.pitmodeInRange);
    bitWrite(operationMode, 1, myEEPROM.pitmodeOutRange);
    bitWrite(operationMode, 2, pitMode);
    bitWrite(operationMode, 3, myEEPROM.unlocked);

    txPacket[0] = SMARTAUDIO_SYNC;
    txPacket[1] = SMARTAUDIO_HEADER;
    txPacket[2] = SET_OPERATION_MODE;
    txPacket[3] = 0x03; // Length
    txPacket[4] = operationMode;
    txPacket[5] = RESERVE_BYTE;
    //txPacket[6] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC
    smartaudioSendPacket();
}

enum {
    SA_SYNC = 0,
    SA_HEADER,
    SA_COMMAND,
    SA_LENGTH,
    SA_DATA,
    SA_CRC,
};

static uint8_t state, in_idx, in_len;

void smartaudioProcessSerial(void)
{
    uint8_t data, state_next = SA_SYNC;
    if (serial_available()) {
        data = serial_read();

        rxPacket[in_idx++] = data;

        switch (state) {
            case SA_SYNC:
                if (data == SMARTAUDIO_SYNC) {
                    state_next = SA_HEADER;
                }
                break;
            case SA_HEADER:
                if (data == SMARTAUDIO_HEADER)
                    state_next = SA_COMMAND;
                break;
            case SA_COMMAND:
                state_next = SA_LENGTH;
                break;
            case SA_LENGTH:
                state_next = data ? SA_DATA : SA_CRC;
                in_len = in_idx + data;
                break;
            case SA_DATA:
                if (in_len <= in_idx)
                    state_next = SA_CRC;
                break;
            case SA_CRC:
                // CRC check and packet processing
                if (smartadioCalcCrc(rxPacket, in_len) == data) {
                    status_led3(1);
                    vtxModeLocked = 1; // Successfully got a packet so lock VTx mode.

                    switch (rxPacket[2] >> 1) // Commands
                    {
                    case GET_SETTINGS:
                        smartaudioBuildSettingsPacket();
                        break;
                    case SET_POWER:
                        smartaudioProcessPowerPacket();
                        break;
                    case SET_CHANNEL:
                        smartaudioProcessChannelPacket();
                        break;
                    case SET_FREQUENCY:
                        smartaudioProcessFrequencyPacket();
                        break;
                    case SET_OPERATION_MODE:
                        smartaudioProcessModePacket();
                        break;
                    }
                }
                break;
            default:
                break;
        }

        if (state_next == SA_SYNC) {
            // Restart
            in_idx = 0;
        }

        state = state_next;
    }
    
    status_led3(0);
}
