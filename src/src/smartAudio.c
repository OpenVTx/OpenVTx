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
    5333, 5373, 5413, 5453, 5493, 5533, 5573, 5613  // L
};


/**** SmartAudio definitions ****/
#define CRC_LEN         1
#define LEGHT_CALC(len) (sizeof(sa_header_t) + CRC_LEN + (len))

// SmartAudio command and response codes
enum {
    SA_CMD_NONE = 0x00,
    SA_CMD_GET_SETTINGS = 0x01,
    SA_CMD_SET_POWER,
    SA_CMD_SET_CHAN,
    SA_CMD_SET_FREQ,
    SA_CMD_SET_MODE,
    SA_CMD_GET_SETTINGS_V2 = 0x09,        // Response only
    SA_CMD_GET_SETTINGS_V21 = 0x11,

    /* Internal custom commands */
    SA_CMD_BOOTLOADER = 0x78,
};

#define PIT_MODE_FREQ_GET   (0x1 << 14)
#define PIT_MODE_FREQ_SET   (0x1 << 15)

#define SA_SYNC_BYTE    0xAA
#define SA_HEADER_BYTE  0x55
#define RESERVE_BYTE    0x01


typedef struct {
    uint8_t sync;
    uint8_t header;
    uint8_t command;
    uint8_t length;
} sa_header_t;

typedef struct {
    uint8_t  channel;
    uint8_t  power;
    uint8_t  operationMode;
    uint8_t  frequency[2];
    uint8_t  rawPowerValue;
    uint8_t  num_pwr_levels;
    uint8_t  levels[SA_NUM_POWER_LEVELS];
} sa_settings_resp_t;

typedef struct {
    uint8_t data_u8;
    uint8_t reserved;
} sa_u8_resp_t;

typedef struct {
    uint8_t data_u16[2];
    uint8_t reserved;
} sa_u16_resp_t;


uint8_t* fill_resp_header(uint8_t cmd, uint8_t len)
{
    sa_header_t * hdr = (sa_header_t*)txPacket;
    hdr->sync = SA_SYNC_BYTE;
    hdr->header = SA_HEADER_BYTE;
    hdr->command = cmd;
    hdr->length = len;
    return &txPacket[sizeof(sa_header_t)];
}


// https://github.com/betaflight/betaflight/blob/287741b816fb5bdac1f72a825846303454765fac/src/main/io/vtx_smartaudio.c#L152
uint8_t smartadioCalcCrc(const uint8_t *data, uint8_t len)
{
#define POLYGEN 0xd5
    uint8_t crc = 0;
    uint_fast8_t ii;

    while (len--) {
        crc ^= *data++;
        ii = 8;
        while (ii--) {
            if ((crc & 0x80) != 0)
                crc = (crc << 1) ^ POLYGEN;
            else
                crc <<= 1;
        }
    }
    return crc;
}


void smartaudioSendPacket(void)
{
    sa_header_t * hdr = (sa_header_t*)txPacket;
    uint8_t len = hdr->length + sizeof(sa_header_t);
    txPacket[len] = smartadioCalcCrc(
        (uint8_t*)&hdr->command,
        (len - offsetof(sa_header_t, command)));
    len += CRC_LEN;
    // Flight Controller needs a bit time to swap TX to RX state
    delay(10);
    Serial_write_len(txPacket, len);
    serial_flush();
}

void smartaudioBuildSettingsPacket(void)
{
    sa_settings_resp_t * payload =
        (sa_settings_resp_t*)fill_resp_header(
            SA_CMD_GET_SETTINGS_V21, sizeof(sa_settings_resp_t));
    uint8_t operationMode = 0;
    bitWrite(operationMode, 0, myEEPROM.freqMode);
    bitWrite(operationMode, 1, pitMode);
    bitWrite(operationMode, 2, myEEPROM.pitmodeInRange);
    bitWrite(operationMode, 3, myEEPROM.pitmodeOutRange);
    bitWrite(operationMode, 4, myEEPROM.unlocked);

    payload->channel = myEEPROM.channel;
    payload->power = 0; // Fake index to allow setting any power level
    payload->operationMode = operationMode;
    payload->frequency[0] = (uint8_t)(myEEPROM.currFreq >> 8);
    payload->frequency[1] = (uint8_t)(myEEPROM.currFreq);
    payload->rawPowerValue = myEEPROM.currPowerdB;
    payload->num_pwr_levels = SA_NUM_POWER_LEVELS;
    for (uint8_t i = 0; i < SA_NUM_POWER_LEVELS; i++)
        payload->levels[i] = powerLevels[i];

    smartaudioSendPacket();
}

void smartaudioProcessFrequencyPacket(void)
{
    sa_u16_resp_t * payload =
        (sa_u16_resp_t*)fill_resp_header(
            SA_CMD_SET_FREQ, sizeof(sa_u16_resp_t));
    uint16_t freq = rxPacket[4];
    freq <<= 8;
    freq |= rxPacket[5];

    if (freq & PIT_MODE_FREQ_GET)
    {
        // POR is not supported in SA2.1 so return currFreq
        freq = myEEPROM.currFreq;
    }
    else if (freq & PIT_MODE_FREQ_SET)
    {
        // POR is not supported in SA2.1 so do not set a POR freq
        freq = myEEPROM.currFreq;
    }
    else
    {
        rtc6705WriteFrequency(freq);
        myEEPROM.freqMode = 1;
    }

    payload->data_u16[0] = (uint8_t)(freq >> 8);
    payload->data_u16[1] = (uint8_t)(freq);
    payload->reserved = RESERVE_BYTE;

    smartaudioSendPacket();
}

void smartaudioProcessChannelPacket(void)
{
    sa_u8_resp_t * payload =
        (sa_u8_resp_t*)fill_resp_header(
            SA_CMD_SET_CHAN, sizeof(sa_u8_resp_t));
    uint8_t channel = rxPacket[4];

    if (channel < ARRAY_SIZE(channelFreqTable)) {
        myEEPROM.channel = channel;
        rtc6705WriteFrequency(channelFreqTable[channel]);
        myEEPROM.freqMode = 0;
    } else {
        channel = myEEPROM.channel;
    }

    payload->data_u8 = channel;
    payload->reserved = RESERVE_BYTE;

    smartaudioSendPacket();
}

void smartaudioProcessPowerPacket(void)
{
    sa_u8_resp_t * payload =
        (sa_u8_resp_t*)fill_resp_header(
            SA_CMD_SET_POWER, sizeof(sa_u8_resp_t));
    uint8_t data = rxPacket[4];
    /* SA2.1 sets the MSB to indicate power is in dB.
     * Set MSB to zero and currPower will now be in dB. */
    uint8_t const value_in_db = data & 0x80;
    data &= 0x7F;

    if (!data)
    {
        /* 0dB is pit mode enable */
        pitMode = value_in_db ? 1 : 0; // Do not set pitmode for a 0mW.  INav sends this command on boot for some reason. https://github.com/iNavFlight/inav/issues/6976
        uint8_t tempCurrPowerdB = myEEPROM.currPowerdB;
        setPowerdB(0);
        myEEPROM.currPowerdB = tempCurrPowerdB;
    }
    else if (data == RACE_MODE)
    {
        pitMode = 1;
        setPowerdB(data);
    }
    else if (value_in_db)
    {
        setPowerdB(data);
    }
    else
    {
        setPowermW(data);
    }

    payload->data_u8 = myEEPROM.currPowerdB;
    payload->reserved = RESERVE_BYTE;

    smartaudioSendPacket();
}

void smartaudioProcessModePacket(void)
{
    sa_u8_resp_t * payload =
        (sa_u8_resp_t*)fill_resp_header(
            SA_CMD_SET_MODE, sizeof(sa_u8_resp_t));
    uint8_t data = rxPacket[4], operationMode = 0;

    uint8_t previousPitmodeInRange = myEEPROM.pitmodeInRange;

    // Set PIR and POR. POR is no longer used in SA2.1 and is treated like PIR
    myEEPROM.pitmodeInRange = bitRead(data, 0);
    myEEPROM.pitmodeOutRange = bitRead(data, 1);
    uint8_t clearPitMode = bitRead(data, 2);
    myEEPROM.unlocked = bitRead(data, 3);

    // This bit is only for CLEARING pitmode.
    if (clearPitMode) {
        pitMode = 0;

        if (myEEPROM.currPowerdB == RACE_MODE)
        {
            setPowerdB(14);
            myEEPROM.currPowerdB = RACE_MODE;
        } else
        {
            setPowerdB(myEEPROM.currPowerdB);
        }
    }

    // When turning on pitmodeInRange go into pitmode
    if (previousPitmodeInRange < myEEPROM.pitmodeInRange)
    {
        /* Enable pitmode if PIR is set */
        pitMode = 1;
        setPowerdB(myEEPROM.currPowerdB);
    }

    updateEEPROM();

    bitWrite(operationMode, 0, myEEPROM.pitmodeInRange);
    bitWrite(operationMode, 1, myEEPROM.pitmodeOutRange);
    bitWrite(operationMode, 2, pitMode);
    bitWrite(operationMode, 3, myEEPROM.unlocked);

    payload->data_u8 = operationMode;
    payload->reserved = RESERVE_BYTE;

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
    uint8_t state_next = SA_SYNC;
    if (serial_available()) {
        uint8_t const data = serial_read();

        rxPacket[in_idx++] = data;

        switch (state) {
            case SA_SYNC:
                if (data == SA_SYNC_BYTE) {
                    state_next = SA_HEADER;
                }
                break;
            case SA_HEADER:
                if (data == SA_HEADER_BYTE) {
                    state_next = SA_COMMAND;
                }
                break;
            case SA_COMMAND:
                state_next = SA_LENGTH;
                break;
            case SA_LENGTH:
                state_next = data ? SA_DATA : SA_CRC;
                in_len = in_idx + data;
                break;
            case SA_DATA:
                state_next = (in_idx < in_len) ? SA_DATA : SA_CRC;
                break;
            case SA_CRC:
                // CRC check and packet processing
                if (smartadioCalcCrc(rxPacket, in_len) == data) {
                    status_led3(1);
                    vtxModeLocked = 1; // Successfully got a packet so lock VTx mode.

                    switch (rxPacket[2] >> 1) // Commands
                    {
                    case SA_CMD_GET_SETTINGS: // 0x03
                        smartaudioBuildSettingsPacket();
                        break;
                    case SA_CMD_SET_POWER: // 0x05
                        smartaudioProcessPowerPacket();
                        break;
                    case SA_CMD_SET_CHAN: // 0x07
                        smartaudioProcessChannelPacket();
                        break;
                    case SA_CMD_SET_FREQ: // 0x09
                        smartaudioProcessFrequencyPacket();
                        break;
                    case SA_CMD_SET_MODE: // 0x0B
                        smartaudioProcessModePacket();
                        break;
                    case SA_CMD_BOOTLOADER:
                        if (rxPacket[4] == 'R' && rxPacket[5] == 'S' && rxPacket[6] == 'T')
                            reboot_into_bootloader(SMARTAUDIO_BAUD);
                        break;
                    }
                    status_led3(0);
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
}
