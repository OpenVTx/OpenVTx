#include "tramp.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "targets.h"
#include "serial.h"


#define TRAMP_HEADER    0x0F // 15
#define TRAMP_MSG_SIZE  15

struct tramp_msg {
    uint8_t header;
    uint8_t cmd;
    uint8_t payload[12];
    uint8_t crc;
};

enum {
    TRAMP_SYNC = 0,
    TRAMP_DATA,
    TRAMP_CRC,
};

static uint8_t state, in_idx;

uint8_t trampCalcCrc(uint8_t *packet)
{
    uint8_t crc = 0;

    for (int i = 1; i < (TRAMP_MSG_SIZE - 1); i++)
    {
        crc += packet[i];
    }

    return crc;
}

void trampReset(void)
{
    state = TRAMP_SYNC;
    in_idx = 0;
}

void trampSendPacket(void)
{
    // Flight Controller needs a bit time to swap TX to RX state
    delay(10);

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
    uint16_t temperature = 0; // Dummy value.

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

    if (mW == RACE_MODE)
        pitMode = 1;

    setPowermW(mW);
}

void trampProcessIPacket(void)
{
    pitMode = !rxPacket[2];

    if (myEEPROM.currPowermW == RACE_MODE)
    {
        setPowerdB(14);
    } else
    {
        setPowermW(myEEPROM.currPowermW);
    }

    myEEPROM.pitmodeInRange = pitMode;  // Pitmode set via CMS is not remembered with Tramp, but I have forced it here to be useful like SA pitmode.
    myEEPROM.pitmodeOutRange = 0;       // Set to 0 so only one of PIR or POR is set for smartaudio

    updateEEPROM();
}

void trampProcessSerial(void)
{
    if (serial_available()) {
        uint8_t data = serial_read();

        rxPacket[in_idx++] = data;

        switch (state) {
            case TRAMP_SYNC:
                if (data == TRAMP_HEADER) {
                    state = TRAMP_DATA;
                } else {
                    in_idx = 0;
                }
                break;
            case TRAMP_DATA:
                if ((TRAMP_MSG_SIZE - 1) <= in_idx)
                    state = TRAMP_CRC;
                break;
            case TRAMP_CRC:
                if (data == trampCalcCrc(rxPacket)) {
                    status_led3(1);
                    
                    vtxModeLocked = 1; // Successfully got a packet so lock VTx mode.

                    switch (rxPacket[1]) // command
                    {
                    case 'F': // 0x46 - Freq - do not respond to this packet
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
                    case 'R':// Reboot to bootloader
                        if (rxPacket[2] == 'S' && rxPacket[3] == 'T')
                            reboot_into_bootloader(9600);
                        break;
                    }
                    
                    status_led3(0);
                }
                in_idx = 0;
                state = TRAMP_SYNC;
                break;
        };
    }
}
