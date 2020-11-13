#include "tramp.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "targets.h"
#include <Arduino.h>

void switchToSmartAudio(void)
{
    if (!vtxModeLocked)
    {
        Serial_begin(SMARTAUDIO_BAUD);
        myEEPROM.vtxMode = SMARTAUDIO;
        updateEEPROM = true;
    }
}

uint8_t trampCalcCrc(uint8_t *packet)
{
    uint8_t crc = 0;

    for (int i = 1; i < 14; i++)
    {
        crc += packet[i];
    }

    return crc;
}

void trampSendPacket(void)
{
    for (int i = 0; i < 16; i++)
    {
        Serial_write(txPacket[i]);
    }
    Serial_flush();
}

void trampBuildrPacket(void)
{
    zeroTxPacket();
    txPacket[0] = 15;
    txPacket[1] = 'r';
    txPacket[2] = MIN_FREQ & 0xff;
    txPacket[3] = (MIN_FREQ >> 8) & 0xff;
    txPacket[4] = MAX_FREQ & 0xff;
    txPacket[5] = (MAX_FREQ >> 8) & 0xff;
    txPacket[6] = MAX_POWER & 0xff;
    txPacket[7] = (MAX_POWER >> 8) & 0xff;
    txPacket[14] = trampCalcCrc(txPacket);
}

void trampBuildvPacket(void)
{
    zeroTxPacket();
    txPacket[0] = 15;
    txPacket[1] = 'v';
    txPacket[2] = myEEPROM.currFreq & 0xff;
    txPacket[3] = (myEEPROM.currFreq >> 8) & 0xff;
    txPacket[4] = myEEPROM.currPowermW & 0xff;          // Configured transmitting power
    txPacket[5] = (myEEPROM.currPowermW >> 8) & 0xff;   // Configured transmitting power
    txPacket[6] = 0;                                    // trampControlMode
    txPacket[7] = pitMode;                              // trampPitMode
    txPacket[8] = myEEPROM.currPowermW & 0xff;          // Actual transmitting power
    txPacket[9] = (myEEPROM.currPowermW >> 8) & 0xff;   // Actual transmitting power
    txPacket[14] = trampCalcCrc(txPacket);
}

void trampBuildsPacket(void)
{
    zeroTxPacket();
    txPacket[0] = 15;
    txPacket[1] = 's';
    txPacket[6] = temperature & 0xff;
    txPacket[7] = (temperature >> 8) & 0xff;
    txPacket[14] = trampCalcCrc(txPacket);
}

void trampProcessFPacket(void)
{
    myEEPROM.currFreq = rxPacket[2] | (rxPacket[3] << 8);
    rtc6705WriteFrequency(myEEPROM.currFreq);

    updateEEPROM = true;
}

void trampProcessPPacket(void)
{
    myEEPROM.currPowermW = rxPacket[2] | (rxPacket[3] << 8);
    setPowermW(myEEPROM.currPowermW);

    updateEEPROM = true;
}

void trampProcessIPacket(void)
{
    pitMode = !rxPacket[2];
    setPowermW(myEEPROM.currPowermW);   // Regardless of input mW, pitmode will force output to 0mW.

    myEEPROM.pitmodeInRange = pitMode;  // Pitmode set via CMS is not remembered with Tramp, but I have forced it here to be useful like SA pitmode.
    myEEPROM.pitmodeOutRange = 0;       // Set to 0 so only one of PIR or POR is set for smartaudio

    updateEEPROM = true;
}

void trampProcessSerial(void)
{
    // wait all bytes to be received
    if (Serial_available() == 15)
    {
        rxPacket[0] = Serial_read();

        if (rxPacket[0] == 15) // 0x0F - tramp header
        {
            // read in buffer
            for (int i = 1; i < 15; i++)
            {
                rxPacket[i] = Serial_read();
            }

            if (rxPacket[14] == trampCalcCrc(rxPacket))
            {
                vtxModeLocked = true; // Successfully got a packet so lock VTx mode.

                switch (rxPacket[1]) // command
                {
                case 'F': // 0x50 - Freq - do not respond to this packet
                    trampProcessFPacket();
                    break;
                case 'P': // 0x50 - Power - do not respond to this packet
                    trampProcessPPacket();
                    break;
                case 'I': // 0x49 - Pitmode - do not respond to this packet. Pitmode is not remember on reboot for Tramp, but I have so that matches SA and is useful.
                    trampProcessIPacket();
                    break;
                case 'r': // 0x72 - Max min freq and power packet
                    trampBuildrPacket();
                    trampSendPacket();
                    break;
                case 'v': // 0x76 - Verify
                    trampBuildvPacket();
                    trampSendPacket();
                    break;
                case 's': // 0x73 - Temperature
                    trampBuildsPacket();
                    trampSendPacket();
                    break;
                }
            }
            else
            {
                switchToSmartAudio();
            }
        }
        else
        {
            switchToSmartAudio();
        }

        clearSerialBuffer();

        // return to make serial monitor readable when debuging
        //     Serial_print_c('\n');
    }
}
