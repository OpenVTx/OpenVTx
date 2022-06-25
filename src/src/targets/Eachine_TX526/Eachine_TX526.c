#include "targets.h"
#include "common.h"
#include "helpers.h"
#include <Arduino.h>


uint8_t saPowerLevelsLut[SA_NUM_POWER_LEVELS] = {1, RACE_MODE, 14, 20, 26};


void target_rfPowerAmpPinSetup(void)
{
  pinMode(POWER_AMP_2, OUTPUT);
  pinMode(POWER_AMP_3, OUTPUT);
  pinMode(POWER_AMP_5, OUTPUT);
}

void target_setup(void)
{
}

void target_loop(void)
{
}

void target_set_power_dB(float dB)
{
  int8_t dBint = (int)(dB + 0.5);

  switch (dBint)
  {
  case 0:
  case 1:
    digitalWrite(POWER_AMP_2, 0);
    digitalWrite(POWER_AMP_3, 0);
    digitalWrite(POWER_AMP_5, 1);
    break;
  case 14:
    digitalWrite(POWER_AMP_2, 0);
    digitalWrite(POWER_AMP_3, 1);
    digitalWrite(POWER_AMP_5, 1);
    break;
  case 23:
    digitalWrite(POWER_AMP_2, 1);
    digitalWrite(POWER_AMP_5, 1);
    break;
  default:
    break;
  }
}

void checkPowerOutput(void)
{
}
