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

void status_leds_init(void);
void status_led1(uint8_t state);
void status_led2(uint8_t state);
void status_led3(uint8_t state);

struct PowerMapping {
    uint16_t mW;
    uint8_t dB;
    uint16_t pwm_val;
    uint16_t VpdSetPoint;
    uint8_t amp_state;
    double a;
    double b;
    double c;
    double d;
};
uint8_t get_power_dB_by_index(uint8_t idx);
uint8_t get_power_dB_by_mW(uint16_t mW);
uint16_t get_power_mW_by_index(uint8_t idx);
uint16_t get_power_mW_by_dB(uint8_t dB);
uint8_t get_power_index_by_mW(uint16_t mW);
uint8_t get_power_index_by_dB(uint8_t dB);


uint8_t get_power_db_values(uint8_t * const list);

void setPowerdB(uint8_t dB);
void setPowermW(uint16_t mW);
