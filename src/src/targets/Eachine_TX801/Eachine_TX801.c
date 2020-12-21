#include "targets.h"
#include "common.h"
#include "helpers.h"
#include <Arduino.h>

struct PowerMapping power_mapping[4] = {
  {0, 0}, {25, 14}, {100, 20}, {200, 23},
};
uint8_t power_mapping_size = ARRAY_SIZE(power_mapping);


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

uint8_t target_set_power_mW(uint16_t power)
{
  uint8_t pinOutput = 0;
  uint8_t index = 0xff;

  switch (power)
  {
  case 0:
    pinOutput = 1; // Setting to 0 does not reduce power for some reason :|
    index = 0;
    break;
  case 25:
    pinOutput = 18;
    index = 1;
    break;
  case 100:
    pinOutput = 50;
    index = 2;
    break;
  case 200:
    pinOutput = 63;
    index = 3;
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
  return index;
}


void taget_setup(void)
{
}


void taget_loop(void)
{
}
