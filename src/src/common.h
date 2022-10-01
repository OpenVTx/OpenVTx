#pragma once

#include "platform.h"
#include <stdint.h>

#define TRAMP_BAUD                      9600
#define SMARTAUDIO_BAUD                 4800
#define MSP_BAUD                        9600 // ~10% decrease in setup time increasing to 19200. But not worth it for the inceased chance in packets errors due to heat.

#define PROTOCOL_CHECK_TIMEOUT          500

#define FLIGHT_CONTROLLER_CHECK_TIMEOUT 3000

#define RACE_MODE                       2
#define RACE_MODE_POWER                 14 // dBm

#define BAND_NAME_LENGTH                8
#define IS_FACTORY_BAND                 0
#define CHANNEL_COUNT                   8

extern uint8_t rxPacket[64];
extern uint8_t txPacket[64];
#define FREQ_TABLE_SIZE 48

extern uint16_t channelFreqTable[FREQ_TABLE_SIZE];

extern uint8_t vtxModeLocked;
extern uint8_t pitMode;

extern uint8_t initFreqPacketRecived;

uint8_t getFreqTableSize(void);
uint8_t getFreqTableBands(void);
uint8_t getFreqTableChannels(void);
uint16_t getFreqByIdx(uint8_t idx);

uint8_t getBandLetterByIdx(uint8_t idx);
uint8_t channelFreqLabelByIdx(uint8_t idx);

void clearSerialBuffer(void);
void zeroRxPacket(void);
void zeroTxPacket(void);

void status_leds_init(void);
void status_led1(uint8_t state);
void status_led2(uint8_t state);
void status_led3(uint8_t state);

void setPowerdB(float dB);
void setPowermW(uint16_t mW);

void reboot_into_bootloader(uint32_t baud);
