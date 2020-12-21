#include "targets.h"
#include "common.h"
#include "helpers.h"
#include <Arduino.h>

struct PowerMapping power_mapping[] = {
  {0, 0}, {25, 14}, {200, 23},
};
uint8_t power_mapping_size = ARRAY_SIZE(power_mapping);


void target_rfPowerAmpPinSetup(void)
{
  pinMode(POWER_AMP_2, OUTPUT);
  pinMode(POWER_AMP_3, OUTPUT);
  pinMode(POWER_AMP_5, OUTPUT);
}

void target_set_power_mW(uint16_t power)
{
  switch (power)
  {
  case 0:
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
    break;
  }
}


void taget_setup(void)
{
}


void taget_loop(void)
{
}
