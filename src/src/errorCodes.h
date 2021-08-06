#pragma once

#include <stdint.h>

typedef enum
{
  RTC6705_NOT_DETECTED,
  POWER_5V_NOT_DETECTED,
  POWER_3V3_NOT_DETECTED,
  NO_EEROR
} errorMode_e;

errorMode_e currentErrorMode;

uint8_t rtc6705NotFoundTime[8];
uint8_t power5vNotDetectedime[6];
uint8_t power3v3NotDetectedime[4];

uint8_t errorIndex;
uint8_t errorLedStatus;
uint32_t errorTime;

void errorCheck(void);