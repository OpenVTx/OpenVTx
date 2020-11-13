#include "common.h"
#include <Arduino.h>
#include <string.h>

uint8_t rxPacket[16];
uint8_t txPacket[18];
uint8_t vtxModeLocked;
uint8_t pitMode = 1;
uint16_t temperature; // Dummy value.

void clearSerialBuffer(void)
{
    while (Serial_available()) {
        Serial_read();
    }
}

void zeroRxPacket(void)
{
    memset(rxPacket, 0, sizeof(rxPacket));
}

void zeroTxPacket(void)
{
    memset(txPacket, 0, sizeof(txPacket));
}
