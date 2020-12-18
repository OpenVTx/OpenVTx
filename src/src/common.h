#pragma once

#include <stdint.h>

#define TRAMP_BAUD 9600
#define SMARTAUDIO_BAUD 4800

#define PROTOCOL_CHECK_TIMEOUT 200


extern uint8_t rxPacket[16];
extern uint8_t txPacket[18];

extern uint8_t vtxModeLocked;
extern uint8_t pitMode;


void clearSerialBuffer(void);
void zeroRxPacket(void);
void zeroTxPacket(void);

void setPowerdB(uint16_t currPowerdB);
