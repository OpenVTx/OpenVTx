// https://www.team-blacksheep.com/tbs_smartaudio_rev08.pdf

#define SMARTAUDIO_SYNC 0xAA
#define SMARTAUDIO_HEADER 0x55

#define GET_SETTINGS 0x01
#define SET_POWER 0x02
#define SET_CHANNEL 0x03
#define SET_FREQUENCY 0x04
#define SET_OPERATION_MODE 0x05

#define PIT_MODE_FREQ_REQUEST 0x40
#define PIT_MODE_FREQ_SET 0x80

#define VERSION_1_COMMAND 0x01
#define VERSION_2_COMMAND 0x09
#define VERSION_2_1_COMMAND 0x11
#define RESERVE_BYTE 0x01

uint16_t channelFreqTable[48] = {
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // F
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // R
    5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621  // L
};

void switchToTramp()
{
    if (!vtxModeLocked)
    {
        Serial_begin(TRAMP_BAUD);
        myEEPROM.vtxMode = TRAMP;
        updateEEPROM = true;
    }
}

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
                crc = (byte)((crc << 1) ^ POLYGEN);
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void smartaudioSendPacket()
{
    for (int i = 0; i < (5 + txPacket[3]); i++)
    {
        Serial_write(txPacket[i]);
    }
    Serial_flush();
}

void smartaudioBuildSettingsPacket()
{
    byte operationMode = 0;
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
    txPacket[15] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC
}

void smartaudioProcessFrequencyPacket()
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
    txPacket[3] = 0x04​​; // Length
    txPacket[4] = (returnFreq >> 8) & 0xFF;
    txPacket[5] = returnFreq & 0xFF;
    txPacket[6] = RESERVE_BYTE;
    txPacket[7] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC

    myEEPROM.freqMode = 1;
    updateEEPROM = true;
}

void smartaudioProcessChannelPacket()
{
    myEEPROM.channel = rxPacket[4];
    myEEPROM.currFreq = channelFreqTable[myEEPROM.channel];

    rtc6705WriteFrequency(myEEPROM.currFreq);

    txPacket[0] = SMARTAUDIO_SYNC;
    txPacket[1] = SMARTAUDIO_HEADER;
    txPacket[2] = SET_CHANNEL;
    txPacket[3] = 0x03​​; // Length
    txPacket[4] = myEEPROM.channel;
    txPacket[5] = RESERVE_BYTE;
    txPacket[6] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC

    myEEPROM.freqMode = 0;
    updateEEPROM = true;
}

void smartaudioProcessPowerPacket()
{
    myEEPROM.currPowerdB = rxPacket[4];
    bitWrite(myEEPROM.currPowerdB, 7, 0); // SA2.1 sets the MSB to indicate power is in dB. Set MSB to zero and currPower will now be in dB.
    setPowerdB(myEEPROM.currPowerdB);
    updateEEPROM = true;

    txPacket[0] = SMARTAUDIO_SYNC;
    txPacket[1] = SMARTAUDIO_HEADER;
    txPacket[2] = SET_POWER;
    txPacket[3] = 0x03​​; // Length
    txPacket[4] = myEEPROM.currPowerdB;
    txPacket[5] = RESERVE_BYTE;
    txPacket[6] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC
}

void smartaudioProcessModePacket()
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

    updateEEPROM = true;

    byte operationMode = 0;
    bitWrite(operationMode, 0, myEEPROM.pitmodeInRange);
    bitWrite(operationMode, 1, myEEPROM.pitmodeOutRange);
    bitWrite(operationMode, 2, pitMode);
    bitWrite(operationMode, 3, myEEPROM.unlocked);

    txPacket[0] = SMARTAUDIO_SYNC;
    txPacket[1] = SMARTAUDIO_HEADER;
    txPacket[2] = SET_OPERATION_MODE;
    txPacket[3] = 0x03​​; // Length
    txPacket[4] = operationMode;
    txPacket[5] = RESERVE_BYTE;
    txPacket[6] = smartadioCalcCrc(&txPacket[2], txPacket[3] + 1); // CRC
}

void smartaudioProcessSerial()
{
    if (Serial_available())
    {
        // delay to allow all bytes to be received
        delay(30);

        Serial_read();               // blanking frame
        rxPacket[0] = Serial_read(); // SMARTAUDIO_SYNC
        rxPacket[1] = Serial_read(); // SMARTAUDIO_HEADER

        if (rxPacket[0] == SMARTAUDIO_SYNC && rxPacket[1] == SMARTAUDIO_HEADER)
        {
            rxPacket[2] = Serial_read(); // Commands
            rxPacket[3] = Serial_read(); // Data​​Length

            for (uint8_t i = 0; i < rxPacket[3]; i++)
            {
                rxPacket[4 + i] = Serial_read(); // Payload
            }

            uint8_t CRC = Serial_read();

            if (smartadioCalcCrc(rxPacket, 4 + rxPacket[3]) == CRC) // CRC check
            {
                vtxModeLocked = true; // Successfully got a packet so lock VTx mode.

                zeroTxPacket();

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
                smartaudioSendPacket();
            }
            else
            {
                switchToTramp();
            }
        }
        else
        {
            switchToTramp();
        }

        clearSerialBuffer();

        // return to make serial monitor readable when debuging
        //     Serial_print_c('\n');
    }
}