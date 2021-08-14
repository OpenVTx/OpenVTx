#pragma once

#include <stdint.h>

#define TRAMP_BAUD 9600
#define SMARTAUDIO_BAUD 4800

#define PROTOCOL_CHECK_TIMEOUT 200

#define FLIGHT_CONTROLLER_CHECK_TIMEOUT 3000
#define FLIGHT_CONTROLLER_CHECK_INTERVAL 300

#define RACE_MODE 2

extern uint8_t rxPacket[16];
extern uint8_t txPacket[18];

extern uint8_t vtxModeLocked;
extern uint8_t pitMode;

extern uint8_t initFreqPacketRecived;

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
