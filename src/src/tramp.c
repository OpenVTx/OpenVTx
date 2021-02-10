#include "tramp.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "targets.h"
#include "serial.h"

uint16_t temperature = 0; // Dummy value.

#define TRAMP_HEADER    0x0F // 15
#define TRAMP_MSG_SIZE  15


uint8_t trampCalcCrc(uint8_t *packet)
{
    uint8_t crc = 0;

    for (int i = 1; i < (TRAMP_MSG_SIZE - 1); i++)
    {
        crc += packet[i];
    }

    return crc;
}

void trampSendPacket(void)
{
    txPacket[TRAMP_MSG_SIZE-1] = trampCalcCrc(txPacket);
    txPacket[TRAMP_MSG_SIZE] = 0;
    Serial_write_len(txPacket, (TRAMP_MSG_SIZE+1));
    serial_flush();
}

void trampBuildrPacket(void)
{
    zeroTxPacket();
    txPacket[0] = TRAMP_HEADER;
    txPacket[1] = 'r';
    txPacket[2] = MIN_FREQ & 0xff;
    txPacket[3] = (MIN_FREQ >> 8) & 0xff;
    txPacket[4] = MAX_FREQ & 0xff;
    txPacket[5] = (MAX_FREQ >> 8) & 0xff;
    txPacket[6] = MAX_POWER & 0xff;
    txPacket[7] = (MAX_POWER >> 8) & 0xff;
    trampSendPacket();
}

void trampBuildvPacket(void)
{
    uint16_t mW = myEEPROM.currPowermW;

    zeroTxPacket();
    txPacket[0] = TRAMP_HEADER;
    txPacket[1] = 'v';
    txPacket[2] = myEEPROM.currFreq & 0xff;
    txPacket[3] = (myEEPROM.currFreq >> 8) & 0xff;
    txPacket[4] = mW & 0xff;          // Configured transmitting power
    txPacket[5] = (mW >> 8) & 0xff;   // Configured transmitting power
    txPacket[6] = 0;                                    // trampControlMode
    txPacket[7] = pitMode;                              // trampPitMode
    txPacket[8] = mW & 0xff;          // Actual transmitting power
    txPacket[9] = (mW >> 8) & 0xff;   // Actual transmitting power
    trampSendPacket();
}

void trampBuildsPacket(void)
{
    zeroTxPacket();
    txPacket[0] = TRAMP_HEADER;
    txPacket[1] = 's';
    txPacket[6] = temperature & 0xff;
    txPacket[7] = (temperature >> 8) & 0xff;
    trampSendPacket();
}

void trampProcessFPacket(void)
{
    uint32_t freq = rxPacket[3];
    freq <<= 8;
    freq |= rxPacket[2];
    rtc6705WriteFrequency(freq);
}

void trampProcessPPacket(void)
{
    uint16_t mW = rxPacket[3];
    mW <<= 8;
    mW += rxPacket[2];
    setPowermW(mW);

    myEEPROM.currPowermW = mW;
    updateEEPROM = 1;
}

void trampProcessIPacket(void)
{
    pitMode = !rxPacket[2];

     // When in TRAMP mode power must be set by mW to stop rounding errors due to saved dBm being an int.
     // Regardless of input mW, pitmode will force output to 0mW.
    setPowermW(myEEPROM.currPowermW);

    myEEPROM.pitmodeInRange = pitMode;  // Pitmode set via CMS is not remembered with Tramp, but I have forced it here to be useful like SA pitmode.
    myEEPROM.pitmodeOutRange = 0;       // Set to 0 so only one of PIR or POR is set for smartaudio

    updateEEPROM = 1;
}

void trampProcessSerial(void)
{
    // wait all bytes to be received
    if (TRAMP_MSG_SIZE <= serial_available())
    {
        rxPacket[0] = serial_read();

        if (rxPacket[0] == TRAMP_HEADER)
        {
            // read in buffer
            for (int i = 1; i < TRAMP_MSG_SIZE; i++)
            {
                rxPacket[i] = serial_read();
            }

            if (rxPacket[(TRAMP_MSG_SIZE - 1)] == trampCalcCrc(rxPacket))
            {
                status_led3(1);
                vtxModeLocked = 1; // Successfully got a packet so lock VTx mode.

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
                    break;
                case 'v': // 0x76 - Verify
                    trampBuildvPacket();
                    break;
                case 's': // 0x73 - Temperature
                    trampBuildsPacket();
                    break;
                }

                status_led3(0);
            }
        }

        clearSerialBuffer();
    }
}
