#pragma once

#include <stdint.h>

enum
{
  RTC6705_NOT_DETECTED,
  POWER_5V_NOT_DETECTED,
  POWER_3V3_NOT_DETECTED,
  NO_ERROR
};

extern uint8_t currentErrorMode;

void errorCheck(void);
