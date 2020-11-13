// https://www.team-blacksheep.com/tbs_smartaudio_rev08.pdf

#pragma once

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


void smartaudioProcessSerial(void);
