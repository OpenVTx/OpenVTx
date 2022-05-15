#include "targets.h"
#include "common.h"
#include "helpers.h"
#include <Arduino.h>


uint8_t saPowerLevelsLut[SA_NUM_POWER_LEVELS] = {0, 1, 14, 20, 23};


void target_rfPowerAmpPinSetup(void)
{
  // pinMode(POWER_AMP_1, INPUT);
  // pinMode(POWER_AMP_2, INPUT);
  // pinMode(POWER_AMP_3, INPUT);
  // pinMode(POWER_AMP_4, INPUT);
  // pinMode(POWER_AMP_5, INPUT);

  // pinMode(POWER_AMP_1, OUTPUT);
  // pinMode(POWER_AMP_2, OUTPUT);
  // pinMode(POWER_AMP_3, OUTPUT);
  // pinMode(POWER_AMP_4, OUTPUT);
  // pinMode(POWER_AMP_5, OUTPUT);
  // pinMode(POWER_AMP_6, OUTPUT);
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
  uint8_t pinOutput = 0;

  switch (dBint)
  {
  case 0:
  case 1:
    pinOutput = 1; // Setting to 0 does not reduce power for some reason :|
    break;
  case 14:
    pinOutput = 18;
    break;
  case 20:
    pinOutput = 50;
    break;
  case 23:
    pinOutput = 63;
    break;
  default:
    break;
  }

  if (pinOutput) {
    // digitalWrite(POWER_AMP_1, pinOutput & 0b000001);
    // digitalWrite(POWER_AMP_2, pinOutput & 0b000010);
    // digitalWrite(POWER_AMP_3, pinOutput & 0b000100);
    // digitalWrite(POWER_AMP_4, pinOutput & 0b001000);
    // digitalWrite(POWER_AMP_5, pinOutput & 0b010000);
    // digitalWrite(POWER_AMP_6, pinOutput & 0b100000);
  }
}

void checkPowerOutput(void)
{
}
