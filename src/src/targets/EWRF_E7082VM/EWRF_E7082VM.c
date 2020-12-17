#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include <Arduino.h>
#include "pwm.h"

struct timeout outputPowerTimer;

void rfPowerAmpPinSetup(void)
{
  pinMode(VREF, OUTPUT);
  digitalWrite(VREF, LOW); // Power amp OFF

  outputPowerTimer = pwm_init(RTC_BIAS);
  setPowermW(0);
}

void setPowermW(uint16_t power)
{
  if (pitMode)
  {
    power = 0;
  }

  switch (power)
  {
  case 0:
    pwm_out_write(outputPowerTimer, 3000);
    digitalWrite(VREF, LOW); // Power amp OFF
    break;
  case 25:
    pwm_out_write(outputPowerTimer, 2453);
    digitalWrite(VREF, HIGH); // Power amp ON
    break;
  case 100:
    pwm_out_write(outputPowerTimer, 2414);
    digitalWrite(VREF, HIGH); // Power amp ON
    break;
  case 400:
    pwm_out_write(outputPowerTimer, 0);
    digitalWrite(VREF, HIGH); // Power amp ON
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
  case 26:
    setPowermW(400);
    break;
  default:
    return; // power value not recognised and no change
    break;
  }
}
