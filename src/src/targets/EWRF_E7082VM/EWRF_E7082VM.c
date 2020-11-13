#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include <Arduino.h>


void rfPowerAmpPinSetup(void)
{
  // TODO: PWM setup...
}

void setPowermW(uint16_t power)
{
  if (pitMode)
  {
    power = 1;
  }

  switch (power)
  {
  case 1:
    break;
  case 25:
    break;
  case 100:
    break;
  case 200:
    break;
  default:
    return; // power value not recognised and no change
    break;
  }
}

void setPowerdB(uint16_t currPowerdB)
{
  switch (currPowerdB)
  {
  case 0:
    setPowermW(0);
    break;
  case 14:
    setPowermW(25);
    break;
  case 20:
    setPowermW(100);
    break;
  case 23:
    setPowermW(200);
    break;
  default:
    return; // power value not recognised and no change
    break;
  }
}
