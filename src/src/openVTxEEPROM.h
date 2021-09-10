#pragma once

#include <stdint.h>

#define versionEEPROM 0x0001

#define BOOT_FREQ 5000

typedef enum
{
  TRAMP,
  SMARTAUDIO,
  SMARTAUDIO_INAV,
  //BOOTLOADER,
  VTX_MODE_MAX
} vtxMode_e;

typedef struct
{
    uint16_t version;
    vtxMode_e vtxMode;
    uint16_t currFreq;
    uint8_t channel;
    uint8_t freqMode;
    uint8_t pitmodeInRange;
    uint8_t pitmodeOutRange;
    float currPowerdB;
    uint16_t currPowermW; // Required due to rounding errors when converting between dBm and mW
    uint8_t unlocked;
} openVTxEEPROM;

extern openVTxEEPROM myEEPROM;

void updateEEPROM(void);
void defaultEEPROM(void);
void readEEPROM(void);
void writeEEPROM(void);
