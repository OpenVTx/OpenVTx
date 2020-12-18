#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
//#include "rtc6705.h"
#include "gpio.h"
#include "pwm.h"
#include "printf.h"

struct timeout outputPowerTimer;
struct gpio_out vref_pin;
struct adc vpd_pin;

void rfPowerAmpPinSetup(void)
{
  vpd_pin = adc_config(VPD);
  vref_pin = gpio_out_setup(VREF, 0); // Power amp OFF

  outputPowerTimer = pwm_init(RTC_BIAS);
  pwm_out_write(outputPowerTimer, 3000);


}

uint32_t vpd_value_get(void)
{
  return adc_read(vpd_pin);
}

void setPowermW(uint16_t power)
{
  uint16_t pwm_val = 3000;
  uint8_t amp_state = 1;

  if (pitMode)
  {
    power = 0;
  }

  switch (power)
  {
  case 25:
    pwm_val = 2453;
    break;
  case 100:
    pwm_val = 2414;
    break;
  case 400:
    pwm_val = 0;
    break;
  case 0:
  default:
    amp_state = 0;
    break;
  }

  pwm_out_write(outputPowerTimer, pwm_val);
  gpio_out_write(vref_pin, amp_state);
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


void taget_setup(void)
{
  /* TODO: Configure WDG, fwdgt_config() */
}


void taget_loop(void)
{
#if DEBUG
  static uint32_t temp;
  static char buff[32];
  uint32_t now = millis();
  if (1000 <= (now - temp)) {
    int len = snprintf(buff, sizeof(buff), "a:%lu\n", vpd_value_get());
    Serial_write_len((uint8_t*)buff, len);
    temp = now;
  }
#endif /* DEBUG */

  /* Reset WD */
  fwdgt_counter_reload();
}
