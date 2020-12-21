#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include <Arduino.h>

void rfPowerAmpPinSetup(void)
{
  pinMode(POWER_AMP_2, OUTPUT);
  pinMode(POWER_AMP_3, OUTPUT);
  pinMode(POWER_AMP_5, OUTPUT);
}

uint8_t powerValuesGet(uint8_t * const list)
{
  uint8_t cnt = 0;
  if (list) {
    list[cnt++] = 0;
    list[cnt++] = 14; // 25mW
    list[cnt++] = 23; // 200mW
  }
  return cnt;
}

void setPowermW(uint16_t power)
{
  if (pitMode || power < 1)
  {
    power = 1;
  }

  switch (power)
  {
  case 1:
    digitalWrite(POWER_AMP_2, 0);
    digitalWrite(POWER_AMP_3, 0);
    digitalWrite(POWER_AMP_5, 1);
    break;
  case 25:
    digitalWrite(POWER_AMP_2, 0);
    digitalWrite(POWER_AMP_3, 1);
    digitalWrite(POWER_AMP_5, 1);
    break;
  case 200:
    digitalWrite(POWER_AMP_2, 1);
    digitalWrite(POWER_AMP_5, 1);
    break;
  default:
    return; // power value not recognised and no change
    break;
  }
}


void taget_setup(void)
{
}


void taget_loop(void)
{
}
